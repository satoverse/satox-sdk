use crate::error::{SatoxError, SatoxResult};
use serde::{Deserialize, Serialize};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};

/// IPFS configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IpfsConfig {
    pub gateway_url: String,
    pub api_url: Option<String>,
    pub timeout_seconds: u32,
    pub max_file_size: u64,
}

impl Default for IpfsConfig {
    fn default() -> Self {
        Self {
            gateway_url: "https://ipfs.io".to_string(),
            api_url: None,
            timeout_seconds: 30,
            max_file_size: 100 * 1024 * 1024, // 100MB
        }
    }
}

/// IPFS file information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IpfsFileInfo {
    pub hash: String,
    pub size: u64,
    pub name: Option<String>,
    pub content_type: Option<String>,
    pub upload_time: u64,
}

/// IPFS manager for interacting with IPFS network
pub struct IpfsManager {
    handle: *mut c_void,
}

impl IpfsManager {
    /// Create a new IPFS manager
    pub fn new(config: IpfsConfig) -> SatoxResult<Self> {
        let config_json = serde_json::to_string(&config)?;
        let config_cstr = CString::new(config_json)?;
        
        unsafe {
            let handle = satox_ipfs_manager_create(config_cstr.as_ptr());
            if handle.is_null() {
                return Err(SatoxError::IpfsError("Failed to create IPFS manager".to_string()));
            }
            
            Ok(Self { handle })
        }
    }
    
    /// Upload a file to IPFS
    pub fn upload_file(&self, file_path: &str) -> SatoxResult<IpfsFileInfo> {
        let file_path_cstr = CString::new(file_path)?;
        
        unsafe {
            let info_json_ptr = satox_ipfs_manager_upload_file(self.handle, file_path_cstr.as_ptr());
            if info_json_ptr.is_null() {
                return Err(SatoxError::IpfsError("Failed to upload file".to_string()));
            }
            
            let info_json = CStr::from_ptr(info_json_ptr).to_string_lossy().into_owned();
            let file_info: IpfsFileInfo = serde_json::from_str(&info_json)?;
            Ok(file_info)
        }
    }
    
    /// Upload data to IPFS
    pub fn upload_data(&self, data: &[u8], filename: Option<&str>) -> SatoxResult<IpfsFileInfo> {
        let filename_cstr = filename.map(|s| CString::new(s)).transpose()?;
        let filename_ptr = filename_cstr.as_ref().map(|s| s.as_ptr()).unwrap_or(std::ptr::null());
        
        unsafe {
            let info_json_ptr = satox_ipfs_manager_upload_data(
                self.handle,
                data.as_ptr(),
                data.len() as u64,
                filename_ptr
            );
            
            if info_json_ptr.is_null() {
                return Err(SatoxError::IpfsError("Failed to upload data".to_string()));
            }
            
            let info_json = CStr::from_ptr(info_json_ptr).to_string_lossy().into_owned();
            let file_info: IpfsFileInfo = serde_json::from_str(&info_json)?;
            Ok(file_info)
        }
    }
    
    /// Download a file from IPFS
    pub fn download_file(&self, hash: &str, output_path: &str) -> SatoxResult<()> {
        let hash_cstr = CString::new(hash)?;
        let output_cstr = CString::new(output_path)?;
        
        unsafe {
            let result = satox_ipfs_manager_download_file(
                self.handle,
                hash_cstr.as_ptr(),
                output_cstr.as_ptr()
            );
            
            if result == 0 {
                Ok(())
            } else {
                Err(SatoxError::IpfsError("Failed to download file".to_string()))
            }
        }
    }
    
    /// Get file information from IPFS
    pub fn get_file_info(&self, hash: &str) -> SatoxResult<IpfsFileInfo> {
        let hash_cstr = CString::new(hash)?;
        
        unsafe {
            let info_json_ptr = satox_ipfs_manager_get_file_info(self.handle, hash_cstr.as_ptr());
            if info_json_ptr.is_null() {
                return Err(SatoxError::IpfsError("Failed to get file info".to_string()));
            }
            
            let info_json = CStr::from_ptr(info_json_ptr).to_string_lossy().into_owned();
            let file_info: IpfsFileInfo = serde_json::from_str(&info_json)?;
            Ok(file_info)
        }
    }
    
    /// Pin a file to IPFS
    pub fn pin_file(&self, hash: &str) -> SatoxResult<()> {
        let hash_cstr = CString::new(hash)?;
        
        unsafe {
            let result = satox_ipfs_manager_pin_file(self.handle, hash_cstr.as_ptr());
            if result == 0 {
                Ok(())
            } else {
                Err(SatoxError::IpfsError("Failed to pin file".to_string()))
            }
        }
    }
    
    /// Unpin a file from IPFS
    pub fn unpin_file(&self, hash: &str) -> SatoxResult<()> {
        let hash_cstr = CString::new(hash)?;
        
        unsafe {
            let result = satox_ipfs_manager_unpin_file(self.handle, hash_cstr.as_ptr());
            if result == 0 {
                Ok(())
            } else {
                Err(SatoxError::IpfsError("Failed to unpin file".to_string()))
            }
        }
    }
    
    /// Get the gateway URL for a file
    pub fn get_gateway_url(&self, hash: &str) -> SatoxResult<String> {
        let hash_cstr = CString::new(hash)?;
        
        unsafe {
            let url_ptr = satox_ipfs_manager_get_gateway_url(self.handle, hash_cstr.as_ptr());
            if url_ptr.is_null() {
                return Err(SatoxError::IpfsError("Failed to get gateway URL".to_string()));
            }
            
            let url = CStr::from_ptr(url_ptr).to_string_lossy().into_owned();
            Ok(url)
        }
    }
}

impl Drop for IpfsManager {
    fn drop(&mut self) {
        unsafe {
            satox_ipfs_manager_destroy(self.handle);
        }
    }
}

// FFI declarations
#[link(name = "satox-ipfs")]
extern "C" {
    fn satox_ipfs_manager_create(config_json: *const c_char) -> *mut c_void;
    fn satox_ipfs_manager_destroy(handle: *mut c_void);
    fn satox_ipfs_manager_upload_file(handle: *mut c_void, file_path: *const c_char) -> *const c_char;
    fn satox_ipfs_manager_upload_data(handle: *mut c_void, data: *const u8, size: u64, filename: *const c_char) -> *const c_char;
    fn satox_ipfs_manager_download_file(handle: *mut c_void, hash: *const c_char, output_path: *const c_char) -> i32;
    fn satox_ipfs_manager_get_file_info(handle: *mut c_void, hash: *const c_char) -> *const c_char;
    fn satox_ipfs_manager_pin_file(handle: *mut c_void, hash: *const c_char) -> i32;
    fn satox_ipfs_manager_unpin_file(handle: *mut c_void, hash: *const c_char) -> i32;
    fn satox_ipfs_manager_get_gateway_url(handle: *mut c_void, hash: *const c_char) -> *const c_char;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ipfs_manager_creation() {
        let config = IpfsConfig::default();
        let manager = IpfsManager::new(config);
        assert!(manager.is_ok());
    }
} 