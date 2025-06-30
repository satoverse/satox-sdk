/**
 * @file satox_sdk.h
 * @brief Satox satox-sdk - Satox Sdk
 * @details This header provides Satox Sdk functionality for the Satox satox-sdk.
 *
 * @defgroup satox-sdk satox-sdk Component
 * @ingroup satox-sdk
 * @author Satox Development Team
 * @date 2025-06-23
 * @version 1.0.0
 * @copyright Copyright (c) 2024 Satox
 *
 * @note This file is part of the Satox satox-sdk
 * @warning This is a public API header - changes may break compatibility
 *
 * @see satox_sdk.h
 * @see https://github.com/satoverse/satox-satox-sdk
 */

/**
 * @file satox_sdk.h
 * @brief Satox satox-sdk - Satox Sdk
 * @details This header provides Satox Sdk functionality for the Satox satox-sdk.
 * 
 * @author Satox Development Team
 * @date 2025-06-23
 * @version 1.0.0
 * @copyright Copyright (c) 2024 Satox
 * 
 * @note This file is part of the Satox satox-sdk
 * @warning This is a public API header - changes may break compatibility
 * 
 * @see satox_sdk.h
 * @see https://github.com/satoverse/satox-satox-sdk
 */

#ifndef SATOX_SDK_H
#define SATOX_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the Satox SDK
int satox_sdk_initialize(void);

// Shutdown the Satox SDK
void satox_sdk_shutdown(void);

// Get the SDK version
const char* satox_sdk_get_version(void);

// Database manager functions
void* satox_database_manager_create(const char* config_json);
void satox_database_manager_destroy(void* handle);
int satox_database_manager_create_database(void* handle, const char* name);
char* satox_database_manager_list_databases(void* handle);

// Security manager functions
void* satox_security_manager_create(void);
void satox_security_manager_destroy(void* handle);
int satox_security_manager_generate_keypair(void* handle, char** public_key, char** private_key);
char* satox_security_manager_sign_data(void* handle, const char* data, const char* private_key);
int satox_security_manager_verify_signature(void* handle, const char* data, const char* signature, const char* public_key);
char* satox_security_manager_sha256(void* handle, const char* data);
char* satox_security_manager_encrypt_data(void* handle, const char* data, const char* public_key);
char* satox_security_manager_decrypt_data(void* handle, const char* encrypted_data, const char* private_key);

// Asset manager functions
void* satox_asset_manager_create(void);
void satox_asset_manager_destroy(void* handle);
char* satox_asset_manager_create_asset(void* handle, const char* config_json, const char* owner_address);
char* satox_asset_manager_get_asset_info(void* handle, const char* asset_id);
char* satox_asset_manager_transfer_asset(void* handle, const char* asset_id, const char* from_address, const char* to_address, unsigned long long amount);
int satox_asset_manager_get_asset_balance(void* handle, const char* asset_id, const char* address, unsigned long long* balance);
char* satox_asset_manager_reissue_asset(void* handle, const char* asset_id, const char* owner_address, unsigned long long amount);
char* satox_asset_manager_burn_asset(void* handle, const char* asset_id, const char* owner_address, unsigned long long amount);

// NFT manager functions
void* satox_nft_manager_create(void);
void satox_nft_manager_destroy(void* handle);
char* satox_nft_manager_create_nft(void* handle, const char* asset_id, const char* metadata_json, const char* owner_address);
char* satox_nft_manager_get_nft_info(void* handle, const char* nft_id);
char* satox_nft_manager_transfer_nft(void* handle, const char* nft_id, const char* from_address, const char* to_address);
char* satox_nft_manager_get_nfts_by_owner(void* handle, const char* address);
char* satox_nft_manager_update_metadata(void* handle, const char* nft_id, const char* metadata_json, const char* owner_address);

// Blockchain manager functions
void* satox_blockchain_manager_create(const char* config_json);
void satox_blockchain_manager_destroy(void* handle);
int satox_blockchain_manager_get_block_height(void* handle, unsigned long long* height);
char* satox_blockchain_manager_get_block_info(void* handle, const char* hash);
char* satox_blockchain_manager_get_transaction_info(void* handle, const char* txid);
char* satox_blockchain_manager_get_best_block_hash(void* handle);
char* satox_blockchain_manager_get_block_hash(void* handle, unsigned long long height);

// Network manager functions
void* satox_network_manager_create(const char* config_json);
void satox_network_manager_destroy(void* handle);
int satox_network_manager_start(void* handle);
int satox_network_manager_stop(void* handle);
char* satox_network_manager_get_peers(void* handle);
char* satox_network_manager_get_stats(void* handle);
int satox_network_manager_add_peer(void* handle, const char* address, unsigned short port);
int satox_network_manager_remove_peer(void* handle, const char* address, unsigned short port);
int satox_network_manager_send_message(void* handle, const char* address, unsigned short port, const char* message);

// IPFS manager functions
void* satox_ipfs_manager_create(const char* config_json);
void satox_ipfs_manager_destroy(void* handle);
char* satox_ipfs_manager_upload_file(void* handle, const char* file_path);
char* satox_ipfs_manager_upload_data(void* handle, const unsigned char* data, unsigned long long size, const char* filename);
int satox_ipfs_manager_download_file(void* handle, const char* hash, const char* output_path);
char* satox_ipfs_manager_get_file_info(void* handle, const char* hash);
int satox_ipfs_manager_pin_file(void* handle, const char* hash);
int satox_ipfs_manager_unpin_file(void* handle, const char* hash);
char* satox_ipfs_manager_get_gateway_url(void* handle, const char* hash);

#ifdef __cplusplus
}
#endif

#endif // SATOX_SDK_H 
