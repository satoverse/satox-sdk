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

#include <satox/sdk.hpp>
#include <satox/blockchain.hpp>
#include <satox/asset.hpp>
#include <satox/nft.hpp>
#include <satox/ipfs.hpp>
#include <satox/security.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

using namespace satox;

class AssetMarketplace {
private:
    std::shared_ptr<SDK> sdk;
    std::shared_ptr<BlockchainManager> blockchain_manager;
    std::shared_ptr<AssetManager> asset_manager;
    std::shared_ptr<NFTManager> nft_manager;
    std::shared_ptr<IPFSManager> ipfs_manager;
    std::shared_ptr<SecurityManager> security_manager;

    // Configuration
    const std::string RPC_ENDPOINT = "http://localhost:7777";
    const std::string RPC_USERNAME = "satox";
    const std::string RPC_PASSWORD = "satox";
    const std::string DATA_DIR = "./data";

public:
    AssetMarketplace() {
        // Initialize SDK
        SDKConfig config;
        config.network = "mainnet";
        config.data_dir = DATA_DIR;
        config.rpc_endpoint = RPC_ENDPOINT;
        config.rpc_username = RPC_USERNAME;
        config.rpc_password = RPC_PASSWORD;
        config.enable_sync = true;
        config.sync_interval = 1000;

        sdk = std::make_shared<SDK>();
        sdk->initialize(config);

        // Get managers
        blockchain_manager = sdk->get_blockchain_manager();
        asset_manager = sdk->get_asset_manager();
        nft_manager = sdk->get_nft_manager();
        ipfs_manager = sdk->get_ipfs_manager();
        security_manager = sdk->get_security_manager();

        // Initialize security
        security_manager->initialize();
        security_manager->set_policy(SecurityPolicy::HIGH);
    }

    ~AssetMarketplace() {
        // Cleanup
        security_manager->shutdown();
        sdk->shutdown();
    }

    // Blockchain operations
    void showBlockchainInfo() {
        try {
            auto info = blockchain_manager->get_blockchain_info();
            std::cout << "Blockchain Info:" << std::endl;
            std::cout << "  Height: " << info.height << std::endl;
            std::cout << "  Hash: " << info.hash << std::endl;
            std::cout << "  Difficulty: " << info.difficulty << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error getting blockchain info: " << e.what() << std::endl;
        }
    }

    // Asset operations
    std::string createAsset(const std::string& name, const std::string& symbol, 
                          uint64_t total_supply, uint8_t decimals, bool reissuable) {
        try {
            AssetMetadata metadata;
            metadata.name = name;
            metadata.symbol = symbol;
            metadata.total_supply = total_supply;
            metadata.decimals = decimals;
            metadata.reissuable = reissuable;

            auto asset_id = asset_manager->create_asset(metadata);
            std::cout << "Created asset: " << asset_id << std::endl;
            return asset_id;
        } catch (const std::exception& e) {
            std::cerr << "Error creating asset: " << e.what() << std::endl;
            return "";
        }
    }

    // NFT operations
    std::string createNFT(const std::string& name, const std::string& description, 
                         const std::string& image_path) {
        try {
            // Store image in IPFS
            std::vector<uint8_t> image_data;
            // Read image file...
            auto ipfs_hash = ipfs_manager->add_data(image_data);

            NFTMetadata metadata;
            metadata.name = name;
            metadata.description = description;
            metadata.image = "ipfs://" + ipfs_hash;
            metadata.attributes = {};

            auto nft_id = nft_manager->create_nft(metadata);
            std::cout << "Created NFT: " << nft_id << std::endl;
            return nft_id;
        } catch (const std::exception& e) {
            std::cerr << "Error creating NFT: " << e.what() << std::endl;
            return "";
        }
    }

    // Transfer operations
    bool transferAsset(const std::string& asset_id, const std::string& from, 
                      const std::string& to, uint64_t amount) {
        try {
            auto success = asset_manager->transfer_asset(asset_id, from, to, amount);
            if (success) {
                std::cout << "Transferred " << amount << " of asset " << asset_id 
                         << " from " << from << " to " << to << std::endl;
            }
            return success;
        } catch (const std::exception& e) {
            std::cerr << "Error transferring asset: " << e.what() << std::endl;
            return false;
        }
    }

    bool transferNFT(const std::string& nft_id, const std::string& from, 
                    const std::string& to) {
        try {
            auto success = nft_manager->transfer_nft(nft_id, from, to);
            if (success) {
                std::cout << "Transferred NFT " << nft_id << " from " << from 
                         << " to " << to << std::endl;
            }
            return success;
        } catch (const std::exception& e) {
            std::cerr << "Error transferring NFT: " << e.what() << std::endl;
            return false;
        }
    }

    // Query operations
    void showAssetInfo(const std::string& asset_id) {
        try {
            auto asset = asset_manager->get_asset(asset_id);
            std::cout << "Asset Info:" << std::endl;
            std::cout << "  Name: " << asset.name << std::endl;
            std::cout << "  Symbol: " << asset.symbol << std::endl;
            std::cout << "  Total Supply: " << asset.total_supply << std::endl;
            std::cout << "  Decimals: " << static_cast<int>(asset.decimals) << std::endl;
            std::cout << "  Reissuable: " << (asset.reissuable ? "Yes" : "No") << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error getting asset info: " << e.what() << std::endl;
        }
    }

    void showNFTInfo(const std::string& nft_id) {
        try {
            auto nft = nft_manager->get_nft(nft_id);
            std::cout << "NFT Info:" << std::endl;
            std::cout << "  Name: " << nft.name << std::endl;
            std::cout << "  Description: " << nft.description << std::endl;
            std::cout << "  Image: " << nft.image << std::endl;
            std::cout << "  Owner: " << nft.owner << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error getting NFT info: " << e.what() << std::endl;
        }
    }
};

int main() {
    try {
        // Create marketplace instance
        AssetMarketplace marketplace;

        // Show blockchain info
        marketplace.showBlockchainInfo();

        // Create an asset
        auto asset_id = marketplace.createAsset(
            "Example Asset",
            "EXA",
            1000000,
            8,
            true
        );

        if (!asset_id.empty()) {
            // Show asset info
            marketplace.showAssetInfo(asset_id);

            // Create an NFT
            auto nft_id = marketplace.createNFT(
                "Example NFT",
                "This is an example NFT",
                "./example_image.png"
            );

            if (!nft_id.empty()) {
                // Show NFT info
                marketplace.showNFTInfo(nft_id);

                // Transfer operations
                marketplace.transferAsset(
                    asset_id,
                    "sender_address",
                    "receiver_address",
                    1000
                );

                marketplace.transferNFT(
                    nft_id,
                    "sender_address",
                    "receiver_address"
                );
            }
        }

        // Wait for operations to complete
        std::this_thread::sleep_for(std::chrono::seconds(5));

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 