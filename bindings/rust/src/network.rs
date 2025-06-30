use crate::error::{SatoxError, SatoxResult};
use serde::{Deserialize, Serialize};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};

/// Network configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkConfig {
    pub p2p_port: u16,
    pub rpc_port: u16,
    pub max_connections: u32,
    pub timeout_seconds: u32,
    pub enable_ssl: bool,
    pub ssl_cert_path: Option<String>,
    pub ssl_key_path: Option<String>,
}

impl Default for NetworkConfig {
    fn default() -> Self {
        Self {
            p2p_port: 60777,
            rpc_port: 7777,
            max_connections: 100,
            timeout_seconds: 30,
            enable_ssl: false,
            ssl_cert_path: None,
            ssl_key_path: None,
        }
    }
}

/// Network peer information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PeerInfo {
    pub address: String,
    pub port: u16,
    pub version: String,
    pub subversion: String,
    pub connection_time: u64,
    pub last_seen: u64,
    pub bytes_sent: u64,
    pub bytes_received: u64,
}

/// Network statistics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NetworkStats {
    pub total_connections: u32,
    pub active_connections: u32,
    pub bytes_sent: u64,
    pub bytes_received: u64,
    pub uptime_seconds: u64,
}

/// Network manager for P2P and RPC communication
pub struct NetworkManager {
    handle: *mut c_void,
}

impl NetworkManager {
    /// Create a new network manager
    pub fn new(config: NetworkConfig) -> SatoxResult<Self> {
        let config_json = serde_json::to_string(&config)?;
        let config_cstr = CString::new(config_json)?;
        
        unsafe {
            let handle = satox_network_manager_create(config_cstr.as_ptr());
            if handle.is_null() {
                return Err(SatoxError::NetworkError("Failed to create network manager".to_string()));
            }
            
            Ok(Self { handle })
        }
    }
    
    /// Start the network manager
    pub fn start(&self) -> SatoxResult<()> {
        unsafe {
            let result = satox_network_manager_start(self.handle);
            if result == 0 {
                Ok(())
            } else {
                Err(SatoxError::NetworkError("Failed to start network manager".to_string()))
            }
        }
    }
    
    /// Stop the network manager
    pub fn stop(&self) -> SatoxResult<()> {
        unsafe {
            let result = satox_network_manager_stop(self.handle);
            if result == 0 {
                Ok(())
            } else {
                Err(SatoxError::NetworkError("Failed to stop network manager".to_string()))
            }
        }
    }
    
    /// Get connected peers
    pub fn get_peers(&self) -> SatoxResult<Vec<PeerInfo>> {
        unsafe {
            let peers_json_ptr = satox_network_manager_get_peers(self.handle);
            if peers_json_ptr.is_null() {
                return Err(SatoxError::NetworkError("Failed to get peers".to_string()));
            }
            
            let peers_json = CStr::from_ptr(peers_json_ptr).to_string_lossy().into_owned();
            let peers: Vec<PeerInfo> = serde_json::from_str(&peers_json)?;
            Ok(peers)
        }
    }
    
    /// Get network statistics
    pub fn get_stats(&self) -> SatoxResult<NetworkStats> {
        unsafe {
            let stats_json_ptr = satox_network_manager_get_stats(self.handle);
            if stats_json_ptr.is_null() {
                return Err(SatoxError::NetworkError("Failed to get network stats".to_string()));
            }
            
            let stats_json = CStr::from_ptr(stats_json_ptr).to_string_lossy().into_owned();
            let stats: NetworkStats = serde_json::from_str(&stats_json)?;
            Ok(stats)
        }
    }
    
    /// Add a peer to connect to
    pub fn add_peer(&self, address: &str, port: u16) -> SatoxResult<()> {
        let address_cstr = CString::new(address)?;
        
        unsafe {
            let result = satox_network_manager_add_peer(self.handle, address_cstr.as_ptr(), port);
            if result == 0 {
                Ok(())
            } else {
                Err(SatoxError::NetworkError("Failed to add peer".to_string()))
            }
        }
    }
    
    /// Remove a peer
    pub fn remove_peer(&self, address: &str, port: u16) -> SatoxResult<()> {
        let address_cstr = CString::new(address)?;
        
        unsafe {
            let result = satox_network_manager_remove_peer(self.handle, address_cstr.as_ptr(), port);
            if result == 0 {
                Ok(())
            } else {
                Err(SatoxError::NetworkError("Failed to remove peer".to_string()))
            }
        }
    }
    
    /// Send a message to a peer
    pub fn send_message(&self, address: &str, port: u16, message: &str) -> SatoxResult<()> {
        let address_cstr = CString::new(address)?;
        let message_cstr = CString::new(message)?;
        
        unsafe {
            let result = satox_network_manager_send_message(
                self.handle,
                address_cstr.as_ptr(),
                port,
                message_cstr.as_ptr()
            );
            if result == 0 {
                Ok(())
            } else {
                Err(SatoxError::NetworkError("Failed to send message".to_string()))
            }
        }
    }
}

impl Drop for NetworkManager {
    fn drop(&mut self) {
        unsafe {
            satox_network_manager_destroy(self.handle);
        }
    }
}

// FFI declarations
#[link(name = "satox-network")]
extern "C" {
    fn satox_network_manager_create(config_json: *const c_char) -> *mut c_void;
    fn satox_network_manager_destroy(handle: *mut c_void);
    fn satox_network_manager_start(handle: *mut c_void) -> i32;
    fn satox_network_manager_stop(handle: *mut c_void) -> i32;
    fn satox_network_manager_get_peers(handle: *mut c_void) -> *const c_char;
    fn satox_network_manager_get_stats(handle: *mut c_void) -> *const c_char;
    fn satox_network_manager_add_peer(handle: *mut c_void, address: *const c_char, port: u16) -> i32;
    fn satox_network_manager_remove_peer(handle: *mut c_void, address: *const c_char, port: u16) -> i32;
    fn satox_network_manager_send_message(handle: *mut c_void, address: *const c_char, port: u16, message: *const c_char) -> i32;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_network_manager_creation() {
        let config = NetworkConfig::default();
        let manager = NetworkManager::new(config);
        assert!(manager.is_ok());
    }
} 