use crate::error::{SatoxError, SatoxResult};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};

/// Security manager for cryptographic operations
pub struct SecurityManager {
    handle: *mut c_void,
}

impl SecurityManager {
    /// Create a new security manager
    pub fn new() -> SatoxResult<Self> {
        unsafe {
            let handle = satox_security_manager_create();
            if handle.is_null() {
                return Err(SatoxError::SecurityError("Failed to create security manager".to_string()));
            }
            
            Ok(Self { handle })
        }
    }
    
    /// Generate a new key pair
    pub fn generate_keypair(&self) -> SatoxResult<(String, String)> {
        unsafe {
            let mut public_key_ptr: *mut c_char = std::ptr::null_mut();
            let mut private_key_ptr: *mut c_char = std::ptr::null_mut();
            
            let result = satox_security_manager_generate_keypair(
                self.handle,
                &mut public_key_ptr,
                &mut private_key_ptr
            );
            
            if result == 0 && !public_key_ptr.is_null() && !private_key_ptr.is_null() {
                let public_key = CStr::from_ptr(public_key_ptr).to_string_lossy().into_owned();
                let private_key = CStr::from_ptr(private_key_ptr).to_string_lossy().into_owned();
                Ok((public_key, private_key))
            } else {
                Err(SatoxError::SecurityError("Failed to generate keypair".to_string()))
            }
        }
    }
    
    /// Sign data with a private key
    pub fn sign_data(&self, data: &str, private_key: &str) -> SatoxResult<String> {
        let data_cstr = CString::new(data)?;
        let private_key_cstr = CString::new(private_key)?;
        
        unsafe {
            let signature_ptr = satox_security_manager_sign_data(
                self.handle,
                data_cstr.as_ptr(),
                private_key_cstr.as_ptr()
            );
            
            if signature_ptr.is_null() {
                return Err(SatoxError::SecurityError("Failed to sign data".to_string()));
            }
            
            let signature = CStr::from_ptr(signature_ptr).to_string_lossy().into_owned();
            Ok(signature)
        }
    }
    
    /// Verify signature with a public key
    pub fn verify_signature(&self, data: &str, signature: &str, public_key: &str) -> SatoxResult<bool> {
        let data_cstr = CString::new(data)?;
        let signature_cstr = CString::new(signature)?;
        let public_key_cstr = CString::new(public_key)?;
        
        unsafe {
            let result = satox_security_manager_verify_signature(
                self.handle,
                data_cstr.as_ptr(),
                signature_cstr.as_ptr(),
                public_key_cstr.as_ptr()
            );
            
            Ok(result == 1)
        }
    }
    
    /// Hash data using SHA256
    pub fn sha256(&self, data: &str) -> SatoxResult<String> {
        let data_cstr = CString::new(data)?;
        
        unsafe {
            let hash_ptr = satox_security_manager_sha256(self.handle, data_cstr.as_ptr());
            if hash_ptr.is_null() {
                return Err(SatoxError::SecurityError("Failed to hash data".to_string()));
            }
            
            let hash = CStr::from_ptr(hash_ptr).to_string_lossy().into_owned();
            Ok(hash)
        }
    }
    
    /// Encrypt data with a public key
    pub fn encrypt_data(&self, data: &str, public_key: &str) -> SatoxResult<String> {
        let data_cstr = CString::new(data)?;
        let public_key_cstr = CString::new(public_key)?;
        
        unsafe {
            let encrypted_ptr = satox_security_manager_encrypt_data(
                self.handle,
                data_cstr.as_ptr(),
                public_key_cstr.as_ptr()
            );
            
            if encrypted_ptr.is_null() {
                return Err(SatoxError::SecurityError("Failed to encrypt data".to_string()));
            }
            
            let encrypted = CStr::from_ptr(encrypted_ptr).to_string_lossy().into_owned();
            Ok(encrypted)
        }
    }
    
    /// Decrypt data with a private key
    pub fn decrypt_data(&self, encrypted_data: &str, private_key: &str) -> SatoxResult<String> {
        let encrypted_cstr = CString::new(encrypted_data)?;
        let private_key_cstr = CString::new(private_key)?;
        
        unsafe {
            let decrypted_ptr = satox_security_manager_decrypt_data(
                self.handle,
                encrypted_cstr.as_ptr(),
                private_key_cstr.as_ptr()
            );
            
            if decrypted_ptr.is_null() {
                return Err(SatoxError::SecurityError("Failed to decrypt data".to_string()));
            }
            
            let decrypted = CStr::from_ptr(decrypted_ptr).to_string_lossy().into_owned();
            Ok(decrypted)
        }
    }
}

impl Drop for SecurityManager {
    fn drop(&mut self) {
        unsafe {
            satox_security_manager_destroy(self.handle);
        }
    }
}

// FFI declarations
#[link(name = "satox-security")]
extern "C" {
    fn satox_security_manager_create() -> *mut c_void;
    fn satox_security_manager_destroy(handle: *mut c_void);
    fn satox_security_manager_generate_keypair(handle: *mut c_void, public_key: *mut *mut c_char, private_key: *mut *mut c_char) -> i32;
    fn satox_security_manager_sign_data(handle: *mut c_void, data: *const c_char, private_key: *const c_char) -> *const c_char;
    fn satox_security_manager_verify_signature(handle: *mut c_void, data: *const c_char, signature: *const c_char, public_key: *const c_char) -> i32;
    fn satox_security_manager_sha256(handle: *mut c_void, data: *const c_char) -> *const c_char;
    fn satox_security_manager_encrypt_data(handle: *mut c_void, data: *const c_char, public_key: *const c_char) -> *const c_char;
    fn satox_security_manager_decrypt_data(handle: *mut c_void, encrypted_data: *const c_char, private_key: *const c_char) -> *const c_char;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_security_manager_creation() {
        let manager = SecurityManager::new();
        assert!(manager.is_ok());
    }
} 