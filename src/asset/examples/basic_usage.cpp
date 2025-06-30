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

#include "satox/asset/asset_manager.hpp"
#include <iostream>

int main() {
    try {
        // Get the manager instance
        auto& manager = satox::asset::AssetManager::getInstance();
        
        // Initialize the manager
        satox::asset::AssetConfig config;
        config.name = "example_asset_manager";
        config.type = satox::asset::AssetType::TOKEN;
        config.maxAssets = 1000;
        config.timeout = 30;
        config.enableLogging = true;
        config.logPath = "logs/components/asset/";
        
        if (!manager.initialize(config)) {
            std::cerr << "Failed to initialize asset manager: " 
                      << manager.getLastError() << std::endl;
            return 1;
        }
        
        std::cout << "Asset manager initialized successfully!" << std::endl;
        
        // Create an asset
        satox::asset::AssetCreationRequest create_request;
        create_request.name = "MyToken";
        create_request.symbol = "MTK";
        create_request.amount = 1000000;
        create_request.units = 8;
        create_request.reissuable = true;
        create_request.owner_address = "alice@example.com";
        create_request.metadata["description"] = "My custom token";
        create_request.metadata["website"] = "https://example.com";
        
        if (manager.createAsset(create_request)) {
            std::cout << "Asset 'MyToken' created successfully!" << std::endl;
        } else {
            std::cerr << "Failed to create asset: " << manager.getLastError() << std::endl;
        }
        
        // Get asset information
        satox::asset::AssetMetadata metadata;
        if (manager.getAssetMetadata("MyToken", metadata)) {
            std::cout << "Asset details:" << std::endl;
            std::cout << "  Name: " << metadata.name << std::endl;
            std::cout << "  Symbol: " << metadata.symbol << std::endl;
            std::cout << "  Total Supply: " << metadata.total_supply << std::endl;
            std::cout << "  Issuer: " << metadata.issuer << std::endl;
            std::cout << "  Reissuable: " << (metadata.reissuable ? "Yes" : "No") << std::endl;
        }
        
        // Check balance
        uint64_t balance;
        if (manager.getAssetBalance("alice@example.com", "MyToken", balance)) {
            std::cout << "Alice's balance: " << balance << " MTK" << std::endl;
        }
        
        // Transfer some tokens
        satox::asset::AssetTransferRequest transfer_request;
        transfer_request.asset_name = "MyToken";
        transfer_request.from_address = "alice@example.com";
        transfer_request.to_address = "bob@example.com";
        transfer_request.amount = 100000;
        transfer_request.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        if (manager.transferAsset(transfer_request)) {
            std::cout << "Transferred 100,000 MTK from Alice to Bob" << std::endl;
        } else {
            std::cerr << "Transfer failed: " << manager.getLastError() << std::endl;
        }
        
        // Check balances after transfer
        uint64_t alice_balance, bob_balance;
        if (manager.getAssetBalance("alice@example.com", "MyToken", alice_balance) &&
            manager.getAssetBalance("bob@example.com", "MyToken", bob_balance)) {
            std::cout << "After transfer:" << std::endl;
            std::cout << "  Alice's balance: " << alice_balance << " MTK" << std::endl;
            std::cout << "  Bob's balance: " << bob_balance << " MTK" << std::endl;
        }
        
        // List all assets
        auto assets = manager.listAssets();
        std::cout << "Total assets: " << assets.size() << std::endl;
        for (const auto& asset : assets) {
            std::cout << "  - " << asset["name"].get<std::string>() 
                      << " (" << asset["symbol"].get<std::string>() << ")" << std::endl;
        }
        
        // Get asset statistics
        auto stats = manager.getStats();
        std::cout << "Asset manager statistics:" << std::endl;
        std::cout << "  Total operations: " << stats.totalOperations << std::endl;
        std::cout << "  Successful operations: " << stats.successfulOperations << std::endl;
        std::cout << "  Failed operations: " << stats.failedOperations << std::endl;
        
        // Health check
        if (manager.healthCheck()) {
            auto health = manager.getHealthStatus();
            std::cout << "Health status: OK" << std::endl;
            std::cout << "  Total assets: " << health["total_assets"].get<int>() << std::endl;
        }
        
        // Shutdown
        manager.shutdown();
        std::cout << "Asset manager shut down successfully!" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
