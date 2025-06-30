use crate::error::{SatoxError, SatoxResult};
use crate::thread_safe_manager::{ThreadSafeManager, ThreadState};
use serde::{Deserialize, Serialize};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};
use std::sync::Arc;
use std::time::Duration;

/// Blockchain configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BlockchainConfig {
    pub rpc_url: String,
    pub rpc_port: u16,
    pub rpc_username: Option<String>,
    pub rpc_password: Option<String>,
    pub timeout_seconds: u32,
    pub enable_ssl: bool,
}

impl Default for BlockchainConfig {
    fn default() -> Self {
        Self {
            rpc_url: "127.0.0.1".to_string(),
            rpc_port: 7777,
            rpc_username: None,
            rpc_password: None,
            timeout_seconds: 30,
            enable_ssl: false,
        }
    }
}

/// Block information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BlockInfo {
    pub hash: String,
    pub height: u64,
    pub timestamp: u64,
    pub size: u64,
    pub weight: u64,
    pub version: u32,
    pub merkleroot: String,
    pub tx_count: u32,
    pub previous_hash: Option<String>,
    pub next_hash: Option<String>,
}

/// Transaction information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TransactionInfo {
    pub txid: String,
    pub block_hash: Option<String>,
    pub block_height: Option<u64>,
    pub timestamp: u64,
    pub size: u64,
    pub weight: u64,
    pub version: u32,
    pub locktime: u32,
    pub confirmations: u32,
    pub fee: Option<f64>,
    pub amount: Option<f64>,
}

/// Blockchain manager for Satoxcoin operations with full thread safety
pub struct BlockchainManager {
    manager: ThreadSafeManager,
    handle: *mut c_void,
}

impl BlockchainManager {
    /// Create a new blockchain manager
    pub fn new(config: BlockchainConfig) -> SatoxResult<Self> {
        let config_json = serde_json::to_string(&config)?;
        let config_cstr = CString::new(config_json)?;
        
        unsafe {
            let handle = satox_blockchain_manager_create(config_cstr.as_ptr());
            if handle.is_null() {
                return Err(SatoxError::BlockchainError("Failed to create blockchain manager".to_string()));
            }
            
            Ok(Self {
                manager: ThreadSafeManager::new("BlockchainManager"),
                handle,
            })
        }
    }
    
    /// Initialize the blockchain manager
    pub fn initialize(&self) -> SatoxResult<()> {
        let operation_id = self.manager.begin_operation();
        let current_state = self.manager.get_state().map_err(SatoxError::BlockchainError)?;
        if current_state == ThreadState::Initialized {
            self.manager.end_operation(operation_id);
            return Ok(());
        }
        self.manager.set_state(ThreadState::Initializing).map_err(SatoxError::BlockchainError)?;
        unsafe {
            let result = satox_blockchain_manager_initialize(self.handle);
            if result == 0 {
                self.manager.set_state(ThreadState::Initialized).map_err(SatoxError::BlockchainError)?;
                self.manager.clear_error();
                self.manager.emit_event("initialized", serde_json::Value::Null).map_err(SatoxError::BlockchainError)?;
                self.manager.end_operation(operation_id);
                Ok(())
            } else {
                self.manager.set_error("Failed to initialize blockchain manager".to_string());
                self.manager.end_operation(operation_id);
                Err(SatoxError::BlockchainError(self.manager.get_last_error()))
            }
        }
    }
    
    /// Shutdown the blockchain manager
    pub fn shutdown(&self) -> SatoxResult<()> {
        let operation_id = self.manager.begin_operation();
        let current_state = self.manager.get_state().map_err(SatoxError::BlockchainError)?;
        if current_state == ThreadState::Shutdown {
            self.manager.end_operation(operation_id);
            return Ok(());
        }
        self.manager.set_state(ThreadState::ShuttingDown).map_err(SatoxError::BlockchainError)?;
        unsafe {
            let result = satox_blockchain_manager_shutdown(self.handle);
            if result == 0 {
                self.manager.set_state(ThreadState::Shutdown).map_err(SatoxError::BlockchainError)?;
                self.manager.clear_error();
                self.manager.emit_event("shutdown", serde_json::Value::Null).map_err(SatoxError::BlockchainError)?;
                self.manager.end_operation(operation_id);
                Ok(())
            } else {
                self.manager.set_error("Failed to shutdown blockchain manager".to_string());
                self.manager.end_operation(operation_id);
                Err(SatoxError::BlockchainError(self.manager.get_last_error()))
            }
        }
    }
    
    /// Get the current block height
    pub fn get_block_height(&self) -> SatoxResult<u64> {
        let operation_id = self.manager.begin_operation();
        self.manager.ensure_initialized().map_err(SatoxError::BlockchainError)?;
        unsafe {
            let mut height: u64 = 0;
            let result = satox_blockchain_manager_get_block_height(self.handle, &mut height);
            
            if result == 0 {
                let height_value = serde_json::json!({ "height": height });
                self.manager.emit_event("height_queried", height_value).map_err(SatoxError::BlockchainError)?;
                self.manager.end_operation(operation_id);
                Ok(height)
            } else {
                self.manager.set_error("Failed to get block height".to_string());
                self.manager.end_operation(operation_id);
                Err(SatoxError::BlockchainError(self.manager.get_last_error()))
            }
        }
    }
    
    /// Get block information by hash
    pub fn get_block_info(&self, hash: &str) -> SatoxResult<BlockInfo> {
        let operation_id = self.manager.begin_operation();
        self.manager.ensure_initialized().map_err(SatoxError::BlockchainError)?;
        if hash.is_empty() {
            self.manager.set_error("Invalid block hash".to_string());
            self.manager.end_operation(operation_id);
            return Err(SatoxError::BlockchainError(self.manager.get_last_error()));
        }
        let cache_key = format!("block_{}", hash);
        if let Some(cached_info) = self.manager.get_cache(&cache_key) {
            if let Ok(block_info) = serde_json::from_value::<BlockInfo>(cached_info) {
                self.manager.end_operation(operation_id);
                return Ok(block_info);
            }
        }
        let hash_cstr = CString::new(hash)?;
        unsafe {
            let block_json_ptr = satox_blockchain_manager_get_block_info(self.handle, hash_cstr.as_ptr());
            if block_json_ptr.is_null() {
                self.manager.set_error("Failed to get block info".to_string());
                self.manager.end_operation(operation_id);
                return Err(SatoxError::BlockchainError(self.manager.get_last_error()));
            }
            
            let block_json = CStr::from_ptr(block_json_ptr).to_string_lossy().into_owned();
            let block_info: BlockInfo = serde_json::from_str(&block_json)?;
            
            let cache_value = serde_json::to_value(&block_info)?;
            self.manager.set_cache(&cache_key, cache_value, Duration::from_secs(300)).map_err(SatoxError::BlockchainError)?;
            
            let event_data = serde_json::json!({
                "hash": hash,
                "height": block_info.height,
                "timestamp": block_info.timestamp
            });
            self.manager.emit_event("block_info_queried", event_data).map_err(SatoxError::BlockchainError)?;
            
            self.manager.end_operation(operation_id);
            Ok(block_info)
        }
    }
    
    /// Get transaction information by transaction ID
    pub fn get_transaction_info(&self, txid: &str) -> SatoxResult<TransactionInfo> {
        let operation_id = self.manager.begin_operation();
        self.manager.ensure_initialized().map_err(SatoxError::BlockchainError)?;
        if txid.is_empty() {
            self.manager.set_error("Invalid transaction ID".to_string());
            self.manager.end_operation(operation_id);
            return Err(SatoxError::BlockchainError(self.manager.get_last_error()));
        }
        let cache_key = format!("tx_{}", txid);
        if let Some(cached_info) = self.manager.get_cache(&cache_key) {
            if let Ok(tx_info) = serde_json::from_value::<TransactionInfo>(cached_info) {
                self.manager.end_operation(operation_id);
                return Ok(tx_info);
            }
        }
        let txid_cstr = CString::new(txid)?;
        unsafe {
            let tx_json_ptr = satox_blockchain_manager_get_transaction_info(self.handle, txid_cstr.as_ptr());
            if tx_json_ptr.is_null() {
                self.manager.set_error("Failed to get transaction info".to_string());
                self.manager.end_operation(operation_id);
                return Err(SatoxError::BlockchainError(self.manager.get_last_error()));
            }
            
            let tx_json = CStr::from_ptr(tx_json_ptr).to_string_lossy().into_owned();
            let tx_info: TransactionInfo = serde_json::from_str(&tx_json)?;
            
            let cache_value = serde_json::to_value(&tx_info)?;
            self.manager.set_cache(&cache_key, cache_value, Duration::from_secs(120)).map_err(SatoxError::BlockchainError)?;
            
            let event_data = serde_json::json!({
                "txid": txid,
                "block_height": tx_info.block_height,
                "confirmations": tx_info.confirmations
            });
            self.manager.emit_event("transaction_info_queried", event_data).map_err(SatoxError::BlockchainError)?;
            
            self.manager.end_operation(operation_id);
            Ok(tx_info)
        }
    }
    
    /// Get the best block hash
    pub fn get_best_block_hash(&self) -> SatoxResult<String> {
        let operation_id = self.manager.begin_operation();
        self.manager.ensure_initialized().map_err(SatoxError::BlockchainError)?;
        let cache_key = "best_block_hash";
        if let Some(cached_hash) = self.manager.get_cache(cache_key) {
            if let Some(hash) = cached_hash.as_str() {
                self.manager.end_operation(operation_id);
                return Ok(hash.to_string());
            }
        }
        unsafe {
            let hash_ptr = satox_blockchain_manager_get_best_block_hash(self.handle);
            if hash_ptr.is_null() {
                self.manager.set_error("Failed to get best block hash".to_string());
                self.manager.end_operation(operation_id);
                return Err(SatoxError::BlockchainError(self.manager.get_last_error()));
            }
            
            let hash = CStr::from_ptr(hash_ptr).to_string_lossy().into_owned();
            
            let cache_value = serde_json::Value::String(hash.clone());
            self.manager.set_cache(cache_key, cache_value, Duration::from_secs(30)).map_err(SatoxError::BlockchainError)?;
            
            let event_data = serde_json::json!({ "hash": hash });
            self.manager.emit_event("best_block_hash_queried", event_data).map_err(SatoxError::BlockchainError)?;
            
            self.manager.end_operation(operation_id);
            Ok(hash)
        }
    }
    
    /// Get block hash by height
    pub fn get_block_hash(&self, height: u64) -> SatoxResult<String> {
        let operation_id = self.manager.begin_operation();
        self.manager.ensure_initialized().map_err(SatoxError::BlockchainError)?;
        let cache_key = format!("block_hash_{}", height);
        if let Some(cached_hash) = self.manager.get_cache(&cache_key) {
            if let Some(hash) = cached_hash.as_str() {
                self.manager.end_operation(operation_id);
                return Ok(hash.to_string());
            }
        }
        unsafe {
            let hash_ptr = satox_blockchain_manager_get_block_hash(self.handle, height);
            if hash_ptr.is_null() {
                self.manager.set_error("Failed to get block hash".to_string());
                self.manager.end_operation(operation_id);
                return Err(SatoxError::BlockchainError(self.manager.get_last_error()));
            }
            
            let hash = CStr::from_ptr(hash_ptr).to_string_lossy().into_owned();
            
            let cache_value = serde_json::Value::String(hash.clone());
            self.manager.set_cache(&cache_key, cache_value, Duration::from_secs(300)).map_err(SatoxError::BlockchainError)?;
            
            let event_data = serde_json::json!({
                "height": height,
                "hash": hash
            });
            self.manager.emit_event("block_hash_queried", event_data).map_err(SatoxError::BlockchainError)?;
            
            self.manager.end_operation(operation_id);
            Ok(hash)
        }
    }
    
    /// Get blockchain statistics
    pub fn get_statistics(&self) -> SatoxResult<serde_json::Value> {
        let operation_id = self.manager.begin_operation();
        self.manager.ensure_initialized().map_err(SatoxError::BlockchainError)?;
        let cache_key = "blockchain_statistics";
        if let Some(cached_stats) = self.manager.get_cache(cache_key) {
            self.manager.end_operation(operation_id);
            return Ok(cached_stats);
        }
        unsafe {
            let stats_ptr = satox_blockchain_manager_get_statistics(self.handle);
            if stats_ptr.is_null() {
                self.manager.set_error("Failed to get statistics".to_string());
                self.manager.end_operation(operation_id);
                return Err(SatoxError::BlockchainError(self.manager.get_last_error()));
            }
            
            let stats_json = CStr::from_ptr(stats_ptr).to_string_lossy().into_owned();
            let stats: serde_json::Value = serde_json::from_str(&stats_json)?;
            
            self.manager.set_cache(cache_key, stats.clone(), Duration::from_secs(60)).map_err(SatoxError::BlockchainError)?;
            
            self.manager.emit_event("statistics_queried", stats.clone()).map_err(SatoxError::BlockchainError)?;
            
            self.manager.end_operation(operation_id);
            Ok(stats)
        }
    }
    
    /// Validate a Satoxcoin address
    pub fn validate_address(&self, address: &str) -> SatoxResult<bool> {
        let operation_id = self.manager.begin_operation();
        self.manager.ensure_initialized().map_err(SatoxError::BlockchainError)?;
        if address.is_empty() {
            self.manager.end_operation(operation_id);
            return Ok(false);
        }
        let address_cstr = CString::new(address)?;
        unsafe {
            let result = satox_blockchain_manager_validate_address(self.handle, address_cstr.as_ptr());
            
            let is_valid = result != 0;
            
            let event_data = serde_json::json!({
                "address": address,
                "valid": is_valid
            });
            self.manager.emit_event("address_validated", event_data).map_err(SatoxError::BlockchainError)?;
            
            self.manager.end_operation(operation_id);
            Ok(is_valid)
        }
    }
    
    /// Clear all caches
    pub fn clear_cache(&self) -> SatoxResult<()> {
        let operation_id = self.manager.begin_operation();
        self.manager.clear_cache().map_err(SatoxError::BlockchainError)?;
        self.manager.emit_event("cache_cleared", serde_json::Value::Null).map_err(SatoxError::BlockchainError)?;
        self.manager.end_operation(operation_id);
        Ok(())
    }
    
    /// Get thread safety statistics
    pub fn get_thread_safety_stats(&self) -> SatoxResult<crate::thread_safe_manager::ThreadSafeStats> {
        Ok(self.manager.get_stats().map_err(SatoxError::BlockchainError)?)
    }
    
    /// Get the underlying thread-safe manager
    pub fn get_manager(&self) -> &ThreadSafeManager {
        &self.manager
    }
}

impl Drop for BlockchainManager {
    fn drop(&mut self) {
        let _ = self.shutdown();
        unsafe {
            satox_blockchain_manager_destroy(self.handle);
        }
    }
}

// FFI declarations
#[link(name = "satox-blockchain")]
extern "C" {
    fn satox_blockchain_manager_create(config_json: *const c_char) -> *mut c_void;
    fn satox_blockchain_manager_destroy(handle: *mut c_void);
    fn satox_blockchain_manager_initialize(handle: *mut c_void) -> i32;
    fn satox_blockchain_manager_shutdown(handle: *mut c_void) -> i32;
    fn satox_blockchain_manager_get_block_height(handle: *mut c_void, height: *mut u64) -> i32;
    fn satox_blockchain_manager_get_block_info(handle: *mut c_void, hash: *const c_char) -> *const c_char;
    fn satox_blockchain_manager_get_transaction_info(handle: *mut c_void, txid: *const c_char) -> *const c_char;
    fn satox_blockchain_manager_get_best_block_hash(handle: *mut c_void) -> *const c_char;
    fn satox_blockchain_manager_get_block_hash(handle: *mut c_void, height: u64) -> *const c_char;
    fn satox_blockchain_manager_get_statistics(handle: *mut c_void) -> *const c_char;
    fn satox_blockchain_manager_validate_address(handle: *mut c_void, address: *const c_char) -> i32;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_blockchain_manager_creation() {
        let config = BlockchainConfig::default();
        let manager = BlockchainManager::new(config);
        assert!(manager.is_ok());
    }
    
    // #[test]
    // fn test_thread_safety() {
    //     let config = BlockchainConfig::default();
    //     let manager = BlockchainManager::new(config).unwrap();
    //     
    //     // Test concurrent operations
    //     let manager_arc = Arc::new(manager);
    //     let mut handles = vec![];
    //     
    //     for i in 0..10 {
    //         let manager_clone = Arc::clone(&manager_arc);
    //         let handle = std::thread::spawn(move || {
    //             // These operations should be thread-safe
    //             let _ = manager_clone.get_thread_safety_stats();
    //             let _ = manager_clone.get_manager().get_uptime();
    //         });
    //         handles.push(handle);
    //     }
    //     
    //     for handle in handles {
    //         handle.join().unwrap();
    //     }
    // }
} 