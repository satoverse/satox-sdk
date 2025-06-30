#include "satox_sdk.h"
#include <iostream>
#include <cstdlib>
#include <string>

int main() {
    std::cout << "=== SATOX SDK Hello World Example ===" << std::endl;
    std::cout << std::endl;

    // Get API key from environment variable
    const char* apiKey = std::getenv("SATOX_API_KEY");
    if (!apiKey) {
        std::cerr << "Error: SATOX_API_KEY environment variable not set" << std::endl;
        std::cerr << "Please set your API key: export SATOX_API_KEY='your-api-key'" << std::endl;
        return 1;
    }

    // Get network from environment variable (default to testnet)
    const char* network = std::getenv("SATOX_NETWORK");
    if (!network) {
        network = "testnet";
    }

    try {
        std::cout << "Initializing SATOX SDK..." << std::endl;
        
        // Initialize the SDK with basic configuration
        SatoxConfig config;
        config.network = network;
        config.apiKey = apiKey;
        config.logLevel = "INFO";
        
        SatoxSDK sdk;
        sdk.initialize(config);
        
        std::cout << "✓ SDK initialized successfully!" << std::endl;
        std::cout << std::endl;

        // Get blockchain information
        std::cout << "Connecting to blockchain network..." << std::endl;
        BlockchainInfo info = sdk.getBlockchainInfo();
        std::cout << "✓ Connected to: " << info.network << std::endl;
        std::cout << "✓ Current block: " << info.currentBlock << std::endl;
        std::cout << "✓ Network difficulty: " << info.difficulty << std::endl;
        std::cout << std::endl;

        // Create a wallet
        std::cout << "Retrieving wallet information..." << std::endl;
        Wallet wallet = sdk.createWallet();
        std::cout << "✓ Wallet created: " << wallet.getAddress() << std::endl;
        
        // Get wallet balance
        Balance balance = sdk.getBalance(wallet.getAddress());
        std::cout << "✓ Balance: " << balance.amount << " SATOX" << std::endl;
        std::cout << std::endl;

        // Create a simple asset
        std::cout << "Performing basic operations..." << std::endl;
        AssetData assetData;
        assetData.name = "Hello World Asset";
        assetData.description = "My first SATOX asset created with the SDK";
        assetData.type = AssetType::COLLECTIBLE;
        assetData.quantity = 1;
        assetData.metadata["creator"] = "SATOX SDK Hello World Example";
        assetData.metadata["version"] = "1.0.0";
        assetData.metadata["created"] = "2024-01-20";

        Asset asset = sdk.createAsset(assetData, wallet.getAddress());
        std::cout << "✓ Asset creation successful" << std::endl;
        std::cout << "✓ Asset ID: " << asset.getId() << std::endl;
        std::cout << "✓ Asset Name: " << asset.getName() << std::endl;
        std::cout << std::endl;

        // Demonstrate basic transaction
        std::cout << "Creating a simple transaction..." << std::endl;
        
        // Create a transaction to transfer a small amount (if balance available)
        if (balance.amount > 0.001) {
            TransactionData txData;
            txData.fromAddress = wallet.getAddress();
            txData.toAddress = wallet.getAddress(); // Send to self for demo
            txData.amount = 0.0001; // Very small amount
            txData.assetId = asset.getId();
            txData.fee = 0.00001;

            Transaction tx = sdk.createTransaction(txData);
            std::cout << "✓ Transaction created: " << tx.getId() << std::endl;
            
            // Note: In a real application, you would sign and broadcast the transaction
            std::cout << "  (Transaction not broadcasted in this example)" << std::endl;
        } else {
            std::cout << "  (Skipping transaction - insufficient balance)" << std::endl;
        }
        std::cout << std::endl;

        // Show asset details
        std::cout << "Asset Details:" << std::endl;
        std::cout << "  ID: " << asset.getId() << std::endl;
        std::cout << "  Name: " << asset.getName() << std::endl;
        std::cout << "  Description: " << asset.getDescription() << std::endl;
        std::cout << "  Type: " << static_cast<int>(asset.getType()) << std::endl;
        std::cout << "  Quantity: " << asset.getQuantity() << std::endl;
        std::cout << "  Owner: " << asset.getOwner() << std::endl;
        std::cout << std::endl;

        // Demonstrate error handling
        std::cout << "Testing error handling..." << std::endl;
        try {
            // Try to get balance of an invalid address
            Balance invalidBalance = sdk.getBalance("INVALID_ADDRESS");
            std::cout << "  (This should not be reached)" << std::endl;
        } catch (const SatoxException& e) {
            std::cout << "✓ Error handling working: " << e.what() << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Example completed successfully!" << std::endl;
        std::cout << std::endl;
        std::cout << "What you've accomplished:" << std::endl;
        std::cout << "  ✓ Initialized the SATOX SDK" << std::endl;
        std::cout << "  ✓ Connected to the blockchain network" << std::endl;
        std::cout << "  ✓ Created a wallet" << std::endl;
        std::cout << "  ✓ Created a digital asset" << std::endl;
        std::cout << "  ✓ Demonstrated error handling" << std::endl;
        std::cout << std::endl;
        std::cout << "Next steps:" << std::endl;
        std::cout << "  - Try the Simple Setup example for more configuration options" << std::endl;
        std::cout << "  - Explore Wallet Creation for advanced wallet management" << std::endl;
        std::cout << "  - Study Asset Basics for comprehensive asset operations" << std::endl;
        std::cout << "  - Read the API Reference for complete documentation" << std::endl;

    } catch (const AuthenticationException& e) {
        std::cerr << "Authentication Error: " << e.what() << std::endl;
        std::cerr << "Please check your API key and ensure it's valid." << std::endl;
        return 1;
    } catch (const NetworkException& e) {
        std::cerr << "Network Error: " << e.what() << std::endl;
        std::cerr << "Please check your internet connection and network settings." << std::endl;
        return 1;
    } catch (const SatoxException& e) {
        std::cerr << "SATOX SDK Error: " << e.what() << std::endl;
        std::cerr << "Please check the documentation or create an issue on GitHub." << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 