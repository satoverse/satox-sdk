/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../include/satox_sdk.h"
#include "../include/satox/satox.hpp"
#include "satox/core/core_manager.hpp"
#include "satox/database/database_manager.hpp"
#include "satox/security/security_manager.hpp"
#include "satox/asset/asset_manager.hpp"
#include "satox/nft/nft_manager.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <cstring>

using json = nlohmann::json;

#ifdef __cplusplus
extern "C" {
#endif

// Global SDK state
static bool sdk_initialized = false;

// Helper function to convert std::string to C string
char* string_to_c_string(const std::string& str) {
    char* c_str = new char[str.length() + 1];
    std::strcpy(c_str, str.c_str());
    return c_str;
}

// Helper function to convert JSON to C string
char* json_to_c_string(const json& j) {
    std::string str = j.dump();
    return string_to_c_string(str);
}

// Initialize the Satox SDK
int satox_sdk_initialize() {
    if (sdk_initialized) {
        return 0; // Already initialized
    }
    
    try {
        // Initialize the SDK with default config
        json config = json::object();
        if (satox::SDK::getInstance().initialize(config)) {
            sdk_initialized = true;
            return 0;
        }
        return -1;
    } catch (...) {
        return -1;
    }
}

// Shutdown the Satox SDK
void satox_sdk_shutdown() {
    if (sdk_initialized) {
        try {
            satox::SDK::getInstance().shutdown();
            sdk_initialized = false;
        } catch (...) {
            // Ignore errors during shutdown
        }
    }
}

// Get the SDK version
const char* satox_sdk_get_version() {
    try {
        return satox::SDK::getInstance().getVersion().c_str();
    } catch (...) {
        return "unknown";
    }
}

// Security manager functions
void* satox_security_manager_create() {
    try {
        auto& manager = satox::security::SecurityManager::getInstance();
        return &manager;
    } catch (...) {
        return nullptr;
    }
}

void satox_security_manager_destroy(void* handle) {
    // No-op for singleton pattern
}

int satox_security_manager_generate_keypair(void* handle, char** public_key, char** private_key) {
    try {
        auto& manager = *static_cast<satox::security::SecurityManager*>(handle);
        if (manager.generatePQCKeyPair("DEFAULT")) {
            // For now, return placeholder keys
            *public_key = string_to_c_string("placeholder_public_key");
            *private_key = string_to_c_string("placeholder_private_key");
            return 0;
        }
        return -1;
    } catch (...) {
        return -1;
    }
}

char* satox_security_manager_sign_data(void* handle, const char* data, const char* private_key) {
    try {
        auto& manager = *static_cast<satox::security::SecurityManager*>(handle);
        if (manager.signWithPQC("DEFAULT", data)) {
            return string_to_c_string("placeholder_signature");
        }
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

int satox_security_manager_verify_signature(void* handle, const char* data, const char* signature, const char* public_key) {
    try {
        auto& manager = *static_cast<satox::security::SecurityManager*>(handle);
        if (manager.verifyWithPQC("DEFAULT", data, signature)) {
            return 1; // Success
        }
        return 0; // Failure
    } catch (...) {
        return 0;
    }
}

char* satox_security_manager_sha256(void* handle, const char* data) {
    try {
        // Simple SHA256 implementation
        std::string input(data);
        std::string hash = "placeholder_sha256_hash"; // In real implementation, compute actual hash
        return string_to_c_string(hash);
    } catch (...) {
        return nullptr;
    }
}

char* satox_security_manager_encrypt_data(void* handle, const char* data, const char* public_key) {
    try {
        auto& manager = *static_cast<satox::security::SecurityManager*>(handle);
        if (manager.encryptWithPQC("DEFAULT", data)) {
            return string_to_c_string("placeholder_encrypted_data");
        }
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

char* satox_security_manager_decrypt_data(void* handle, const char* encrypted_data, const char* private_key) {
    try {
        auto& manager = *static_cast<satox::security::SecurityManager*>(handle);
        if (manager.decryptWithPQC("DEFAULT", encrypted_data)) {
            return string_to_c_string("placeholder_decrypted_data");
        }
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

// Database manager functions
void* satox_database_manager_create(const char* config_json) {
    try {
        auto& manager = satox::database::DatabaseManager::getInstance();
        satox::database::DatabaseConfig config;
        if (config_json) {
            json j = json::parse(config_json);
            if (j.contains("name")) config.name = j["name"];
            if (j.contains("enableLogging")) config.enableLogging = j["enableLogging"];
            if (j.contains("logPath")) config.logPath = j["logPath"];
            if (j.contains("maxConnections")) config.maxConnections = j["maxConnections"];
            if (j.contains("connectionTimeout")) config.connectionTimeout = j["connectionTimeout"];
        }
        if (manager.initialize(config)) {
            return &manager;
        }
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

void satox_database_manager_destroy(void* handle) {
    // No-op for singleton pattern
}

int satox_database_manager_create_database(void* handle, const char* name) {
    try {
        auto& manager = *static_cast<satox::database::DatabaseManager*>(handle);
        return manager.createDatabase(name) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}

char* satox_database_manager_list_databases(void* handle) {
    try {
        auto& manager = *static_cast<satox::database::DatabaseManager*>(handle);
        auto databases = manager.listDatabases();
        json result = json::array();
        for (const auto& db : databases) {
            result.push_back(db);
        }
        return json_to_c_string(result);
    } catch (...) {
        return nullptr;
    }
}

// Asset manager functions
void* satox_asset_manager_create() {
    try {
        auto& manager = satox::asset::AssetManager::getInstance();
        return &manager;
    } catch (...) {
        return nullptr;
    }
}

void satox_asset_manager_destroy(void* handle) {
    // No-op for singleton pattern
}

char* satox_asset_manager_create_asset(void* handle, const char* name, const char* symbol) {
    // TODO: Fix this function to use correct AssetManager API
    /*
    try {
        auto& manager = satox::asset::AssetManager::getInstance();
        
        satox::asset::Asset asset;
        asset.id = "asset_" + std::to_string(time(nullptr));
        asset.name = name ? name : "";
        asset.symbol = symbol ? symbol : "";
        asset.owner = "default_owner";
        asset.total_supply = 1000000;
        asset.decimals = 8;
        asset.status = "created";
        asset.metadata = {
            {"asset_id", asset.id},
            {"status", "created"}
        };
        
        auto result = manager.createAsset(asset);
        if (result.success) {
            return strdup(asset.id.c_str());
        } else {
            return strdup(result.error_message.c_str());
        }
    } catch (const std::exception& e) {
        return strdup(e.what());
    }
    */
    return strdup("C wrapper not implemented yet");
}

char* satox_asset_manager_get_asset_info(void* handle, const char* asset_id) {
    // TODO: Fix this function to use correct AssetManager API
    /*
    try {
        auto& manager = satox::asset::AssetManager::getInstance();
        
        auto asset_opt = manager.getAsset(asset_id);
        if (asset_opt) {
            const auto& asset = *asset_opt;
            nlohmann::json result = {
                {"id", asset.id},
                {"name", asset.name},
                {"symbol", asset.symbol},
                {"owner", asset.owner},
                {"total_supply", asset.total_supply},
                {"decimals", asset.decimals},
                {"status", asset.status},
                {"metadata", asset.metadata}
            };
            return strdup(result.dump().c_str());
        } else {
            return strdup("Asset not found");
        }
    } catch (const std::exception& e) {
        return strdup(e.what());
    }
    */
    return strdup("C wrapper not implemented yet");
}

// NFT manager functions
void* satox_nft_manager_create() {
    try {
        auto& manager = satox::nft::NFTManager::getInstance();
        return &manager;
    } catch (...) {
        return nullptr;
    }
}

void satox_nft_manager_destroy(void* handle) {
    // No-op for singleton pattern
}

char* satox_nft_manager_create_nft(void* handle, const char* asset_id, const char* metadata_json, const char* owner_address) {
    try {
        auto& manager = *static_cast<satox::nft::NFTManager*>(handle);
        json metadata = json::parse(metadata_json);
        
        satox::nft::NFTManager::NFTMetadata nft_metadata;
        nft_metadata.name = metadata.value("name", "Default NFT");
        nft_metadata.description = metadata.value("description", "");
        nft_metadata.image = metadata.value("image", "");
        nft_metadata.externalUrl = metadata.value("external_url", "");
        nft_metadata.additionalData = metadata;
        
        if (metadata.contains("attributes")) {
            for (const auto& [key, value] : metadata["attributes"].items()) {
                nft_metadata.attributes[key] = value.get<std::string>();
            }
        }
        
        std::string contract_address = metadata.value("contract_address", "default_contract");
        std::string nft_id = manager.createNFT(contract_address, nft_metadata, owner_address);
        
        if (!nft_id.empty()) {
            json result = {
                {"nft_id", nft_id},
                {"status", "created"}
            };
            return json_to_c_string(result);
        }
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

char* satox_nft_manager_get_nft_info(void* handle, const char* nft_id) {
    try {
        auto& manager = *static_cast<satox::nft::NFTManager*>(handle);
        auto nft = manager.getNFT(nft_id);
        
        if (!nft.id.empty()) {
            json result = {
                {"id", nft.id},
                {"contract_address", nft.contractAddress},
                {"owner", nft.owner},
                {"creator", nft.creator},
                {"token_id", nft.tokenId},
                {"token_uri", nft.tokenURI},
                {"metadata", {
                    {"name", nft.metadata.name},
                    {"description", nft.metadata.description},
                    {"image", nft.metadata.image},
                    {"external_url", nft.metadata.externalUrl},
                    {"attributes", nft.metadata.attributes},
                    {"additional_data", nft.metadata.additionalData}
                }}
            };
            return json_to_c_string(result);
        }
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

// Placeholder implementations for other managers (to be implemented as needed)
void* satox_blockchain_manager_create(const char* config_json) { return nullptr; }
void satox_blockchain_manager_destroy(void* handle) {}
int satox_blockchain_manager_get_block_height(void* handle, unsigned long long* height) { return -1; }
char* satox_blockchain_manager_get_block_info(void* handle, const char* hash) { return nullptr; }
char* satox_blockchain_manager_get_transaction_info(void* handle, const char* txid) { return nullptr; }
char* satox_blockchain_manager_get_best_block_hash(void* handle) { return nullptr; }
char* satox_blockchain_manager_get_block_hash(void* handle, unsigned long long height) { return nullptr; }

char* satox_asset_manager_transfer_asset(void* handle, const char* asset_id, const char* from_address, const char* to_address, unsigned long long amount) { return nullptr; }
int satox_asset_manager_get_asset_balance(void* handle, const char* asset_id, const char* address, unsigned long long* balance) { return -1; }
char* satox_asset_manager_reissue_asset(void* handle, const char* asset_id, const char* owner_address, unsigned long long amount) { return nullptr; }
char* satox_asset_manager_burn_asset(void* handle, const char* asset_id, const char* owner_address, unsigned long long amount) { return nullptr; }

void* satox_network_manager_create(const char* config_json) { return nullptr; }
void satox_network_manager_destroy(void* handle) {}
int satox_network_manager_start(void* handle) { return -1; }
int satox_network_manager_stop(void* handle) { return -1; }
char* satox_network_manager_get_peers(void* handle) { return nullptr; }
char* satox_network_manager_get_stats(void* handle) { return nullptr; }
int satox_network_manager_add_peer(void* handle, const char* address, unsigned short port) { return -1; }
int satox_network_manager_remove_peer(void* handle, const char* address, unsigned short port) { return -1; }
int satox_network_manager_send_message(void* handle, const char* address, unsigned short port, const char* message) { return -1; }

char* satox_nft_manager_transfer_nft(void* handle, const char* nft_id, const char* from_address, const char* to_address) { return nullptr; }
char* satox_nft_manager_get_nfts_by_owner(void* handle, const char* address) { return nullptr; }
char* satox_nft_manager_update_metadata(void* handle, const char* nft_id, const char* metadata_json, const char* owner_address) { return nullptr; }

void* satox_ipfs_manager_create(const char* config_json) { return nullptr; }
void satox_ipfs_manager_destroy(void* handle) {}
char* satox_ipfs_manager_upload_file(void* handle, const char* file_path) { return nullptr; }
char* satox_ipfs_manager_upload_data(void* handle, const unsigned char* data, unsigned long long size, const char* filename) { return nullptr; }
int satox_ipfs_manager_download_file(void* handle, const char* hash, const char* output_path) { return -1; }
char* satox_ipfs_manager_get_file_info(void* handle, const char* hash) { return nullptr; }
int satox_ipfs_manager_pin_file(void* handle, const char* hash) { return -1; }
int satox_ipfs_manager_unpin_file(void* handle, const char* hash) { return -1; }
char* satox_ipfs_manager_get_gateway_url(void* handle, const char* hash) { return nullptr; }

#ifdef __cplusplus
} // extern "C"
#endif 