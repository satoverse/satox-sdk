use crate::error::{SatoxError, SatoxResult};
use serde::{Deserialize, Serialize};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};

/// Asset configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AssetConfig {
    pub name: String,
    pub symbol: String,
    pub description: Option<String>,
    pub precision: u8,
    pub reissuable: bool,
    pub ipfs_hash: Option<String>,
}

/// Asset information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AssetInfo {
    pub asset_id: String,
    pub name: String,
    pub symbol: String,
    pub description: Option<String>,
    pub precision: u8,
    pub reissuable: bool,
    pub total_supply: u64,
    pub circulating_supply: u64,
    pub ipfs_hash: Option<String>,
    pub owner: String,
    pub creation_txid: String,
    pub creation_block: u64,
}

/// Asset transfer information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AssetTransfer {
    pub asset_id: String,
    pub from_address: String,
    pub to_address: String,
    pub amount: u64,
    pub txid: String,
    pub block_height: u64,
    pub confirmations: u32,
}

/// Asset manager for creating and managing Satoxcoin assets
pub struct AssetManager {
    handle: *mut c_void,
}

impl AssetManager {
    /// Create a new asset manager
    pub fn new() -> SatoxResult<Self> {
        unsafe {
            let handle = satox_asset_manager_create();
            if handle.is_null() {
                return Err(SatoxError::AssetError("Failed to create asset manager".to_string()));
            }
            
            Ok(Self { handle })
        }
    }
    
    /// Create a new asset
    pub fn create_asset(&self, config: AssetConfig, owner_address: &str) -> SatoxResult<String> {
        let config_json = serde_json::to_string(&config)?;
        let config_cstr = CString::new(config_json)?;
        let owner_cstr = CString::new(owner_address)?;
        
        unsafe {
            let txid_ptr = satox_asset_manager_create_asset(
                self.handle,
                config_cstr.as_ptr(),
                owner_cstr.as_ptr()
            );
            
            if txid_ptr.is_null() {
                return Err(SatoxError::AssetError("Failed to create asset".to_string()));
            }
            
            let txid = CStr::from_ptr(txid_ptr).to_string_lossy().into_owned();
            Ok(txid)
        }
    }
    
    /// Get asset information by asset ID
    pub fn get_asset_info(&self, asset_id: &str) -> SatoxResult<AssetInfo> {
        let asset_id_cstr = CString::new(asset_id)?;
        
        unsafe {
            let info_json_ptr = satox_asset_manager_get_asset_info(self.handle, asset_id_cstr.as_ptr());
            if info_json_ptr.is_null() {
                return Err(SatoxError::AssetError("Failed to get asset info".to_string()));
            }
            
            let info_json = CStr::from_ptr(info_json_ptr).to_string_lossy().into_owned();
            let asset_info: AssetInfo = serde_json::from_str(&info_json)?;
            Ok(asset_info)
        }
    }
    
    /// Transfer assets
    pub fn transfer_asset(
        &self,
        asset_id: &str,
        from_address: &str,
        to_address: &str,
        amount: u64
    ) -> SatoxResult<String> {
        let asset_id_cstr = CString::new(asset_id)?;
        let from_cstr = CString::new(from_address)?;
        let to_cstr = CString::new(to_address)?;
        
        unsafe {
            let txid_ptr = satox_asset_manager_transfer_asset(
                self.handle,
                asset_id_cstr.as_ptr(),
                from_cstr.as_ptr(),
                to_cstr.as_ptr(),
                amount
            );
            
            if txid_ptr.is_null() {
                return Err(SatoxError::AssetError("Failed to transfer asset".to_string()));
            }
            
            let txid = CStr::from_ptr(txid_ptr).to_string_lossy().into_owned();
            Ok(txid)
        }
    }
    
    /// Get asset balance for an address
    pub fn get_asset_balance(&self, asset_id: &str, address: &str) -> SatoxResult<u64> {
        let asset_id_cstr = CString::new(asset_id)?;
        let address_cstr = CString::new(address)?;
        
        unsafe {
            let mut balance: u64 = 0;
            let result = satox_asset_manager_get_asset_balance(
                self.handle,
                asset_id_cstr.as_ptr(),
                address_cstr.as_ptr(),
                &mut balance
            );
            
            if result == 0 {
                Ok(balance)
            } else {
                Err(SatoxError::AssetError("Failed to get asset balance".to_string()))
            }
        }
    }
    
    /// Reissue assets (if reissuable)
    pub fn reissue_asset(
        &self,
        asset_id: &str,
        owner_address: &str,
        amount: u64
    ) -> SatoxResult<String> {
        let asset_id_cstr = CString::new(asset_id)?;
        let owner_cstr = CString::new(owner_address)?;
        
        unsafe {
            let txid_ptr = satox_asset_manager_reissue_asset(
                self.handle,
                asset_id_cstr.as_ptr(),
                owner_cstr.as_ptr(),
                amount
            );
            
            if txid_ptr.is_null() {
                return Err(SatoxError::AssetError("Failed to reissue asset".to_string()));
            }
            
            let txid = CStr::from_ptr(txid_ptr).to_string_lossy().into_owned();
            Ok(txid)
        }
    }
    
    /// Burn assets
    pub fn burn_asset(
        &self,
        asset_id: &str,
        owner_address: &str,
        amount: u64
    ) -> SatoxResult<String> {
        let asset_id_cstr = CString::new(asset_id)?;
        let owner_cstr = CString::new(owner_address)?;
        
        unsafe {
            let txid_ptr = satox_asset_manager_burn_asset(
                self.handle,
                asset_id_cstr.as_ptr(),
                owner_cstr.as_ptr(),
                amount
            );
            
            if txid_ptr.is_null() {
                return Err(SatoxError::AssetError("Failed to burn asset".to_string()));
            }
            
            let txid = CStr::from_ptr(txid_ptr).to_string_lossy().into_owned();
            Ok(txid)
        }
    }
}

impl Drop for AssetManager {
    fn drop(&mut self) {
        unsafe {
            satox_asset_manager_destroy(self.handle);
        }
    }
}

// FFI declarations
#[link(name = "satox-asset")]
extern "C" {
    fn satox_asset_manager_create() -> *mut c_void;
    fn satox_asset_manager_destroy(handle: *mut c_void);
    fn satox_asset_manager_create_asset(handle: *mut c_void, config_json: *const c_char, owner_address: *const c_char) -> *const c_char;
    fn satox_asset_manager_get_asset_info(handle: *mut c_void, asset_id: *const c_char) -> *const c_char;
    fn satox_asset_manager_transfer_asset(handle: *mut c_void, asset_id: *const c_char, from_address: *const c_char, to_address: *const c_char, amount: u64) -> *const c_char;
    fn satox_asset_manager_get_asset_balance(handle: *mut c_void, asset_id: *const c_char, address: *const c_char, balance: *mut u64) -> i32;
    fn satox_asset_manager_reissue_asset(handle: *mut c_void, asset_id: *const c_char, owner_address: *const c_char, amount: u64) -> *const c_char;
    fn satox_asset_manager_burn_asset(handle: *mut c_void, asset_id: *const c_char, owner_address: *const c_char, amount: u64) -> *const c_char;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_asset_manager_creation() {
        let manager = AssetManager::new();
        assert!(manager.is_ok());
    }
} 