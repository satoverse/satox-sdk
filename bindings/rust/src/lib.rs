//! Rust bindings for the Satox SDK
//! 
//! This crate provides Rust bindings to the C++ Satox SDK, allowing Rust applications
//! to interact with the Satoxcoin blockchain, manage assets, NFTs, and IPFS content.

pub mod blockchain;
pub mod network;
pub mod security;
pub mod asset;
pub mod nft;
pub mod ipfs;
pub mod error;
pub mod thread_safe_manager;

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void, c_int, c_ulonglong, c_ushort};

/// Initialize the Satox SDK
/// 
/// This function must be called before using any other SDK functionality.
/// It initializes the logging system and other internal components.
pub fn initialize() -> Result<(), error::SatoxError> {
    unsafe {
        let result = satox_sdk_initialize();
        if result == 0 {
            Ok(())
        } else {
            Err(error::SatoxError::InitializationFailed)
        }
    }
}

/// Shutdown the Satox SDK
/// 
/// This function should be called when the application is shutting down
/// to properly clean up resources.
pub fn shutdown() {
    unsafe {
        satox_sdk_shutdown();
    }
}

/// Get the SDK version string
pub fn get_version() -> String {
    unsafe {
        let version_ptr = satox_sdk_get_version();
        if version_ptr.is_null() {
            return "Unknown".to_string();
        }
        
        let version_cstr = CStr::from_ptr(version_ptr);
        version_cstr.to_string_lossy().into_owned()
    }
}

/// Connect to the blockchain network
/// 
/// This function establishes a connection to the Satoxcoin blockchain network.
/// It must be called after initialization and before performing any network operations.
pub fn connect() -> Result<(), error::SatoxError> {
    unsafe {
        let result = satox_sdk_connect();
        if result == 0 {
            Ok(())
        } else {
            Err(error::SatoxError::ConnectionFailed)
        }
    }
}

/// Disconnect from the blockchain network
/// 
/// This function gracefully disconnects from the Satoxcoin blockchain network.
/// It should be called before shutting down the SDK.
pub fn disconnect() -> Result<(), error::SatoxError> {
    unsafe {
        let result = satox_sdk_disconnect();
        if result == 0 {
            Ok(())
        } else {
            Err(error::SatoxError::DisconnectionFailed)
        }
    }
}

// FFI declarations for core SDK functions
#[link(name = "satox-sdk-main")]
extern "C" {
    fn satox_sdk_initialize() -> i32;
    fn satox_sdk_shutdown();
    fn satox_sdk_get_version() -> *const c_char;
    fn satox_sdk_connect() -> i32;
    fn satox_sdk_disconnect() -> i32;
}

// FFI declarations for database manager
#[link(name = "satox-sdk-main")]
extern "C" {
    fn satox_database_manager_create(config_json: *const c_char) -> *mut c_void;
    fn satox_database_manager_destroy(handle: *mut c_void);
    fn satox_database_manager_create_database(handle: *mut c_void, name: *const c_char) -> c_int;
    fn satox_database_manager_list_databases(handle: *mut c_void) -> *mut c_char;
}

// FFI declarations for security manager
#[link(name = "satox-sdk-main")]
extern "C" {
    fn satox_security_manager_create() -> *mut c_void;
    fn satox_security_manager_destroy(handle: *mut c_void);
    fn satox_security_manager_generate_keypair(handle: *mut c_void, public_key: *mut *mut c_char, private_key: *mut *mut c_char) -> c_int;
    fn satox_security_manager_sign_data(handle: *mut c_void, data: *const c_char, private_key: *const c_char) -> *mut c_char;
    fn satox_security_manager_verify_signature(handle: *mut c_void, data: *const c_char, signature: *const c_char, public_key: *const c_char) -> c_int;
    fn satox_security_manager_sha256(handle: *mut c_void, data: *const c_char) -> *mut c_char;
    fn satox_security_manager_encrypt_data(handle: *mut c_void, data: *const c_char, public_key: *const c_char) -> *mut c_char;
    fn satox_security_manager_decrypt_data(handle: *mut c_void, encrypted_data: *const c_char, private_key: *const c_char) -> *mut c_char;
}

// FFI declarations for asset manager
#[link(name = "satox-sdk-main")]
extern "C" {
    fn satox_asset_manager_create() -> *mut c_void;
    fn satox_asset_manager_destroy(handle: *mut c_void);
    fn satox_asset_manager_create_asset(handle: *mut c_void, config_json: *const c_char, owner_address: *const c_char) -> *mut c_char;
    fn satox_asset_manager_get_asset_info(handle: *mut c_void, asset_id: *const c_char) -> *mut c_char;
    fn satox_asset_manager_transfer_asset(handle: *mut c_void, asset_id: *const c_char, from_address: *const c_char, to_address: *const c_char, amount: c_ulonglong) -> *mut c_char;
    fn satox_asset_manager_get_asset_balance(handle: *mut c_void, asset_id: *const c_char, address: *const c_char, balance: *mut c_ulonglong) -> c_int;
    fn satox_asset_manager_reissue_asset(handle: *mut c_void, asset_id: *const c_char, owner_address: *const c_char, amount: c_ulonglong) -> *mut c_char;
    fn satox_asset_manager_burn_asset(handle: *mut c_void, asset_id: *const c_char, owner_address: *const c_char, amount: c_ulonglong) -> *mut c_char;
}

// FFI declarations for NFT manager
#[link(name = "satox-sdk-main")]
extern "C" {
    fn satox_nft_manager_create() -> *mut c_void;
    fn satox_nft_manager_destroy(handle: *mut c_void);
    fn satox_nft_manager_create_nft(handle: *mut c_void, asset_id: *const c_char, metadata_json: *const c_char, owner_address: *const c_char) -> *mut c_char;
    fn satox_nft_manager_get_nft_info(handle: *mut c_void, nft_id: *const c_char) -> *mut c_char;
    fn satox_nft_manager_transfer_nft(handle: *mut c_void, nft_id: *const c_char, from_address: *const c_char, to_address: *const c_char) -> *mut c_char;
    fn satox_nft_manager_get_nfts_by_owner(handle: *mut c_void, address: *const c_char) -> *mut c_char;
    fn satox_nft_manager_update_metadata(handle: *mut c_void, nft_id: *const c_char, metadata_json: *const c_char, owner_address: *const c_char) -> *mut c_char;
}

// FFI declarations for blockchain manager
#[link(name = "satox-sdk-main")]
extern "C" {
    fn satox_blockchain_manager_create(config_json: *const c_char) -> *mut c_void;
    fn satox_blockchain_manager_destroy(handle: *mut c_void);
    fn satox_blockchain_manager_get_block_height(handle: *mut c_void, height: *mut c_ulonglong) -> c_int;
    fn satox_blockchain_manager_get_block_info(handle: *mut c_void, hash: *const c_char) -> *mut c_char;
    fn satox_blockchain_manager_get_transaction_info(handle: *mut c_void, txid: *const c_char) -> *mut c_char;
    fn satox_blockchain_manager_get_best_block_hash(handle: *mut c_void) -> *mut c_char;
    fn satox_blockchain_manager_get_block_hash(handle: *mut c_void, height: c_ulonglong) -> *mut c_char;
}

// FFI declarations for network manager
#[link(name = "satox-sdk-main")]
extern "C" {
    fn satox_network_manager_create(config_json: *const c_char) -> *mut c_void;
    fn satox_network_manager_destroy(handle: *mut c_void);
    fn satox_network_manager_start(handle: *mut c_void) -> c_int;
    fn satox_network_manager_stop(handle: *mut c_void) -> c_int;
    fn satox_network_manager_get_peers(handle: *mut c_void) -> *mut c_char;
    fn satox_network_manager_get_stats(handle: *mut c_void) -> *mut c_char;
    fn satox_network_manager_add_peer(handle: *mut c_void, address: *const c_char, port: c_ushort) -> c_int;
    fn satox_network_manager_remove_peer(handle: *mut c_void, address: *const c_char, port: c_ushort) -> c_int;
    fn satox_network_manager_send_message(handle: *mut c_void, address: *const c_char, port: c_ushort, message: *const c_char) -> c_int;
}

// FFI declarations for IPFS manager
#[link(name = "satox-sdk-main")]
extern "C" {
    fn satox_ipfs_manager_create(config_json: *const c_char) -> *mut c_void;
    fn satox_ipfs_manager_destroy(handle: *mut c_void);
    fn satox_ipfs_manager_upload_file(handle: *mut c_void, file_path: *const c_char) -> *mut c_char;
    fn satox_ipfs_manager_upload_data(handle: *mut c_void, data: *const u8, size: c_ulonglong, filename: *const c_char) -> *mut c_char;
    fn satox_ipfs_manager_download_file(handle: *mut c_void, hash: *const c_char, output_path: *const c_char) -> c_int;
    fn satox_ipfs_manager_get_file_info(handle: *mut c_void, hash: *const c_char) -> *mut c_char;
    fn satox_ipfs_manager_pin_file(handle: *mut c_void, hash: *const c_char) -> c_int;
    fn satox_ipfs_manager_unpin_file(handle: *mut c_void, hash: *const c_char) -> c_int;
    fn satox_ipfs_manager_get_gateway_url(handle: *mut c_void, hash: *const c_char) -> *mut c_char;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_initialization() {
        let result = initialize();
        assert!(result.is_ok());
        
        let version = get_version();
        assert!(!version.is_empty());
        
        shutdown();
    }
} 