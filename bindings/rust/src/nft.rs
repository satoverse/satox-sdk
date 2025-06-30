use crate::error::{SatoxError, SatoxResult};
use serde::{Deserialize, Serialize};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};

/// NFT metadata
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NftMetadata {
    pub name: String,
    pub description: Option<String>,
    pub image_url: Option<String>,
    pub attributes: Vec<NftAttribute>,
    pub external_url: Option<String>,
}

/// NFT attribute
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NftAttribute {
    pub trait_type: String,
    pub value: String,
}

/// NFT information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NftInfo {
    pub nft_id: String,
    pub asset_id: String,
    pub owner: String,
    pub metadata: NftMetadata,
    pub creation_txid: String,
    pub creation_block: u64,
    pub ipfs_hash: Option<String>,
}

/// NFT manager for creating and managing Satoxcoin NFTs
pub struct NftManager {
    handle: *mut c_void,
}

impl NftManager {
    /// Create a new NFT manager
    pub fn new() -> SatoxResult<Self> {
        unsafe {
            let handle = satox_nft_manager_create();
            if handle.is_null() {
                return Err(SatoxError::NftError("Failed to create NFT manager".to_string()));
            }
            
            Ok(Self { handle })
        }
    }
    
    /// Create a new NFT
    pub fn create_nft(
        &self,
        asset_id: &str,
        metadata: NftMetadata,
        owner_address: &str
    ) -> SatoxResult<String> {
        let asset_id_cstr = CString::new(asset_id)?;
        let metadata_json = serde_json::to_string(&metadata)?;
        let metadata_cstr = CString::new(metadata_json)?;
        let owner_cstr = CString::new(owner_address)?;
        
        unsafe {
            let txid_ptr = satox_nft_manager_create_nft(
                self.handle,
                asset_id_cstr.as_ptr(),
                metadata_cstr.as_ptr(),
                owner_cstr.as_ptr()
            );
            
            if txid_ptr.is_null() {
                return Err(SatoxError::NftError("Failed to create NFT".to_string()));
            }
            
            let txid = CStr::from_ptr(txid_ptr).to_string_lossy().into_owned();
            Ok(txid)
        }
    }
    
    /// Get NFT information by NFT ID
    pub fn get_nft_info(&self, nft_id: &str) -> SatoxResult<NftInfo> {
        let nft_id_cstr = CString::new(nft_id)?;
        
        unsafe {
            let info_json_ptr = satox_nft_manager_get_nft_info(self.handle, nft_id_cstr.as_ptr());
            if info_json_ptr.is_null() {
                return Err(SatoxError::NftError("Failed to get NFT info".to_string()));
            }
            
            let info_json = CStr::from_ptr(info_json_ptr).to_string_lossy().into_owned();
            let nft_info: NftInfo = serde_json::from_str(&info_json)?;
            Ok(nft_info)
        }
    }
    
    /// Transfer NFT ownership
    pub fn transfer_nft(
        &self,
        nft_id: &str,
        from_address: &str,
        to_address: &str
    ) -> SatoxResult<String> {
        let nft_id_cstr = CString::new(nft_id)?;
        let from_cstr = CString::new(from_address)?;
        let to_cstr = CString::new(to_address)?;
        
        unsafe {
            let txid_ptr = satox_nft_manager_transfer_nft(
                self.handle,
                nft_id_cstr.as_ptr(),
                from_cstr.as_ptr(),
                to_cstr.as_ptr()
            );
            
            if txid_ptr.is_null() {
                return Err(SatoxError::NftError("Failed to transfer NFT".to_string()));
            }
            
            let txid = CStr::from_ptr(txid_ptr).to_string_lossy().into_owned();
            Ok(txid)
        }
    }
    
    /// Get NFTs owned by an address
    pub fn get_nfts_by_owner(&self, address: &str) -> SatoxResult<Vec<NftInfo>> {
        let address_cstr = CString::new(address)?;
        
        unsafe {
            let nfts_json_ptr = satox_nft_manager_get_nfts_by_owner(self.handle, address_cstr.as_ptr());
            if nfts_json_ptr.is_null() {
                return Err(SatoxError::NftError("Failed to get NFTs by owner".to_string()));
            }
            
            let nfts_json = CStr::from_ptr(nfts_json_ptr).to_string_lossy().into_owned();
            let nfts: Vec<NftInfo> = serde_json::from_str(&nfts_json)?;
            Ok(nfts)
        }
    }
    
    /// Update NFT metadata
    pub fn update_nft_metadata(
        &self,
        nft_id: &str,
        metadata: NftMetadata,
        owner_address: &str
    ) -> SatoxResult<String> {
        let nft_id_cstr = CString::new(nft_id)?;
        let metadata_json = serde_json::to_string(&metadata)?;
        let metadata_cstr = CString::new(metadata_json)?;
        let owner_cstr = CString::new(owner_address)?;
        
        unsafe {
            let txid_ptr = satox_nft_manager_update_metadata(
                self.handle,
                nft_id_cstr.as_ptr(),
                metadata_cstr.as_ptr(),
                owner_cstr.as_ptr()
            );
            
            if txid_ptr.is_null() {
                return Err(SatoxError::NftError("Failed to update NFT metadata".to_string()));
            }
            
            let txid = CStr::from_ptr(txid_ptr).to_string_lossy().into_owned();
            Ok(txid)
        }
    }
}

impl Drop for NftManager {
    fn drop(&mut self) {
        unsafe {
            satox_nft_manager_destroy(self.handle);
        }
    }
}

// FFI declarations
#[link(name = "satox-nft")]
extern "C" {
    fn satox_nft_manager_create() -> *mut c_void;
    fn satox_nft_manager_destroy(handle: *mut c_void);
    fn satox_nft_manager_create_nft(handle: *mut c_void, asset_id: *const c_char, metadata_json: *const c_char, owner_address: *const c_char) -> *const c_char;
    fn satox_nft_manager_get_nft_info(handle: *mut c_void, nft_id: *const c_char) -> *const c_char;
    fn satox_nft_manager_transfer_nft(handle: *mut c_void, nft_id: *const c_char, from_address: *const c_char, to_address: *const c_char) -> *const c_char;
    fn satox_nft_manager_get_nfts_by_owner(handle: *mut c_void, address: *const c_char) -> *const c_char;
    fn satox_nft_manager_update_metadata(handle: *mut c_void, nft_id: *const c_char, metadata_json: *const c_char, owner_address: *const c_char) -> *const c_char;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nft_manager_creation() {
        let manager = NftManager::new();
        assert!(manager.is_ok());
    }
} 