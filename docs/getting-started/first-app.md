# Building Your First Application - Satox Core SDK

This tutorial will guide you through building a complete blockchain application using the Satox Core SDK. You'll create a wallet management application that can create wallets, send transactions, and query blockchain data.

## What You'll Build

By the end of this tutorial, you'll have a complete application that can:

- ✅ Create and manage multiple wallets
- ✅ Send and receive transactions
- ✅ Query blockchain information
- ✅ Handle errors gracefully
- ✅ Save and load wallet data
- ✅ Display transaction history

## Prerequisites

Before starting this tutorial, make sure you have:

- ✅ Satox Core SDK installed (see [Installation Guide](installation.md))
- ✅ Basic C++ knowledge
- ✅ A text editor or IDE
- ✅ Internet connection for blockchain access

## Project Structure

We'll create a simple wallet management application with this structure:

```
my-first-app/
├── src/
│   ├── main.cpp
│   ├── wallet_manager.cpp
│   ├── transaction_handler.cpp
│   ├── blockchain_client.cpp
│   └── utils.cpp
├── include/
│   ├── wallet_manager.h
│   ├── transaction_handler.h
│   ├── blockchain_client.h
│   └── utils.h
├── CMakeLists.txt
├── config.json
└── README.md
```

## Step 1: Create the Project Structure

First, let's create our project directory and files:

```bash
# Create project directory
mkdir my-first-app
cd my-first-app

# Create source and include directories
mkdir src include

# Create source files
touch src/main.cpp
touch src/wallet_manager.cpp
touch src/transaction_handler.cpp
touch src/blockchain_client.cpp
touch src/utils.cpp

# Create header files
touch include/wallet_manager.h
touch include/transaction_handler.h
touch include/blockchain_client.h
touch include/utils.h

# Create build files
touch CMakeLists.txt
touch config.json
touch README.md
```

## Step 2: Create the CMakeLists.txt

Create the build configuration file:

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(MyFirstApp VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# Find Satox SDK (adjust path as needed)
find_package(satox REQUIRED)

# Include directories
include_directories(include)
include_directories(${SATOX_INCLUDE_DIRS})

# Create executable
add_executable(my-first-app
    src/main.cpp
    src/wallet_manager.cpp
    src/transaction_handler.cpp
    src/blockchain_client.cpp
    src/utils.cpp
)

# Link libraries
target_link_libraries(my-first-app
    ${SATOX_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
)

# Set compiler flags
target_compile_options(my-first-app PRIVATE
    -Wall
    -Wextra
    -O2
)

# Copy configuration file
configure_file(config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)
```

## Step 3: Create the Configuration File

Create a configuration file for your application:

```json
{
    "app_name": "My First Satox App",
    "version": "1.0.0",
    "sdk_config": {
        "core": {
            "data_dir": "./app_data",
            "log_level": "info",
            "environment": "development"
        },
        "network": {
            "name": "testnet",
            "rpc_url": "https://testnet-rpc.satox.com",
            "timeout": 30,
            "retry_attempts": 3
        },
        "security": {
            "quantum_security": {
                "enabled": true,
                "algorithm": "kyber-1024"
            }
        }
    },
    "app_settings": {
        "max_wallets": 10,
        "default_fee": 0.0001,
        "auto_backup": true,
        "backup_interval": 3600
    }
}
```

## Step 4: Create the Header Files

### Wallet Manager Header

```cpp
// include/wallet_manager.h
#pragma once

#include <satox/wallet/wallet_manager.hpp>
#include <satox/core/core_manager.hpp>
#include <string>
#include <vector>
#include <memory>

class WalletManager {
public:
    WalletManager(std::shared_ptr<satox::CoreManager> core_manager);
    ~WalletManager() = default;

    // Wallet operations
    bool createWallet(const std::string& name, const std::string& password);
    bool loadWallet(const std::string& name, const std::string& password);
    bool deleteWallet(const std::string& name);
    std::vector<std::string> listWallets();
    
    // Wallet information
    std::string getWalletAddress(const std::string& name);
    double getWalletBalance(const std::string& name);
    std::vector<satox::Transaction> getTransactionHistory(const std::string& name);
    
    // Backup and restore
    bool backupWallet(const std::string& name, const std::string& backup_path);
    bool restoreWallet(const std::string& backup_path, const std::string& password);

private:
    std::shared_ptr<satox::CoreManager> core_manager_;
    std::shared_ptr<satox::WalletManager> wallet_manager_;
    std::map<std::string, std::shared_ptr<satox::Wallet>> loaded_wallets_;
    
    bool validateWalletName(const std::string& name);
    bool validatePassword(const std::string& password);
};
```

### Transaction Handler Header

```cpp
// include/transaction_handler.h
#pragma once

#include <satox/blockchain/transaction_manager.hpp>
#include <satox/wallet/wallet_manager.hpp>
#include <string>
#include <memory>

struct TransactionRequest {
    std::string from_wallet;
    std::string to_address;
    double amount;
    double fee;
    std::string message;
};

struct TransactionResult {
    bool success;
    std::string transaction_hash;
    std::string error_message;
    double fee_paid;
};

class TransactionHandler {
public:
    TransactionHandler(std::shared_ptr<satox::CoreManager> core_manager);
    ~TransactionHandler() = default;

    // Transaction operations
    TransactionResult sendTransaction(const TransactionRequest& request);
    TransactionResult sendToMultiple(const std::string& from_wallet, 
                                   const std::vector<std::pair<std::string, double>>& recipients);
    
    // Transaction information
    satox::Transaction getTransaction(const std::string& transaction_hash);
    std::vector<satox::Transaction> getPendingTransactions(const std::string& wallet_name);
    bool isTransactionConfirmed(const std::string& transaction_hash);
    
    // Fee estimation
    double estimateFee(double amount, const std::string& priority = "normal");
    double getNetworkFee();

private:
    std::shared_ptr<satox::CoreManager> core_manager_;
    std::shared_ptr<satox::TransactionManager> transaction_manager_;
    std::shared_ptr<satox::WalletManager> wallet_manager_;
    
    bool validateTransactionRequest(const TransactionRequest& request);
    std::string createTransactionMessage(const std::string& message);
};
```

### Blockchain Client Header

```cpp
// include/blockchain_client.h
#pragma once

#include <satox/blockchain/blockchain_manager.hpp>
#include <string>
#include <memory>

struct NetworkInfo {
    std::string name;
    std::string version;
    uint64_t block_height;
    uint64_t total_transactions;
    double total_supply;
    double difficulty;
    double average_block_time;
};

struct BlockInfo {
    std::string hash;
    uint64_t height;
    uint64_t timestamp;
    size_t transaction_count;
    double total_amount;
};

class BlockchainClient {
public:
    BlockchainClient(std::shared_ptr<satox::CoreManager> core_manager);
    ~BlockchainClient() = default;

    // Network information
    NetworkInfo getNetworkInfo();
    bool isConnected();
    std::string getNetworkName();
    
    // Block information
    BlockInfo getLatestBlock();
    BlockInfo getBlock(uint64_t height);
    BlockInfo getBlock(const std::string& hash);
    std::vector<BlockInfo> getRecentBlocks(size_t count = 10);
    
    // Address information
    double getAddressBalance(const std::string& address);
    std::vector<satox::Transaction> getAddressTransactions(const std::string& address);
    bool isAddressValid(const std::string& address);
    
    // Asset information
    std::vector<satox::Asset> getAssets();
    satox::Asset getAsset(const std::string& asset_id);
    std::vector<satox::Transaction> getAssetTransactions(const std::string& asset_id);

private:
    std::shared_ptr<satox::CoreManager> core_manager_;
    std::shared_ptr<satox::BlockchainManager> blockchain_manager_;
    
    bool validateAddress(const std::string& address);
    std::string formatAmount(double amount);
};
```

### Utils Header

```cpp
// include/utils.h
#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace Utils {
    // String utilities
    std::string toLower(const std::string& str);
    std::string toUpper(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);
    bool startsWith(const std::string& str, const std::string& prefix);
    bool endsWith(const std::string& str, const std::string& suffix);
    
    // Formatting utilities
    std::string formatAmount(double amount, int decimals = 8);
    std::string formatAddress(const std::string& address);
    std::string formatTransactionHash(const std::string& hash);
    std::string formatTimestamp(uint64_t timestamp);
    std::string formatDuration(std::chrono::seconds duration);
    
    // Validation utilities
    bool isValidAddress(const std::string& address);
    bool isValidAmount(double amount);
    bool isValidWalletName(const std::string& name);
    bool isValidPassword(const std::string& password);
    
    // File utilities
    bool fileExists(const std::string& path);
    bool createDirectory(const std::string& path);
    std::string getCurrentDirectory();
    std::string getHomeDirectory();
    
    // Security utilities
    std::string generateRandomString(size_t length);
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
}
```

## Step 5: Implement the Source Files

### Main Application

```cpp
// src/main.cpp
#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>

#include <satox/core/core_manager.hpp>
#include <satox/core/config.hpp>
#include <nlohmann/json.hpp>

#include "wallet_manager.h"
#include "transaction_handler.h"
#include "blockchain_client.h"
#include "utils.h"

using json = nlohmann::json;

class MyFirstApp {
public:
    MyFirstApp() {
        std::cout << "🚀 Starting My First Satox App..." << std::endl;
    }
    
    ~MyFirstApp() {
        if (core_manager_) {
            core_manager_->shutdown();
        }
        std::cout << "👋 Application shutdown complete!" << std::endl;
    }
    
    bool initialize() {
        try {
            // Load configuration
            if (!loadConfiguration()) {
                std::cerr << "❌ Failed to load configuration" << std::endl;
                return false;
            }
            
            // Initialize SDK
            std::cout << "🔧 Initializing Satox SDK..." << std::endl;
            core_manager_ = std::make_shared<satox::CoreManager>();
            core_manager_->initialize(sdk_config_);
            
            // Initialize components
            wallet_manager_ = std::make_unique<WalletManager>(core_manager_);
            transaction_handler_ = std::make_unique<TransactionHandler>(core_manager_);
            blockchain_client_ = std::make_unique<BlockchainClient>(core_manager_);
            
            std::cout << "✅ SDK initialized successfully!" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Initialization failed: " << e.what() << std::endl;
            return false;
        }
    }
    
    void run() {
        std::cout << "\n🎯 Welcome to My First Satox App!" << std::endl;
        std::cout << "=================================" << std::endl;
        
        // Display network information
        displayNetworkInfo();
        
        // Main application loop
        while (true) {
            displayMenu();
            std::string choice = getUserChoice();
            
            if (choice == "1") {
                createWallet();
            } else if (choice == "2") {
                listWallets();
            } else if (choice == "3") {
                sendTransaction();
            } else if (choice == "4") {
                viewTransactionHistory();
            } else if (choice == "5") {
                viewBlockchainInfo();
            } else if (choice == "6") {
                backupWallet();
            } else if (choice == "0") {
                std::cout << "👋 Goodbye!" << std::endl;
                break;
            } else {
                std::cout << "❌ Invalid choice. Please try again." << std::endl;
            }
            
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

private:
    std::shared_ptr<satox::CoreManager> core_manager_;
    std::unique_ptr<WalletManager> wallet_manager_;
    std::unique_ptr<TransactionHandler> transaction_handler_;
    std::unique_ptr<BlockchainClient> blockchain_client_;
    satox::CoreConfig sdk_config_;
    json app_config_;
    
    bool loadConfiguration() {
        try {
            // Load app configuration
            std::ifstream config_file("config.json");
            if (!config_file.is_open()) {
                std::cerr << "❌ Could not open config.json" << std::endl;
                return false;
            }
            
            config_file >> app_config_;
            
            // Load SDK configuration
            sdk_config_.data_dir = app_config_["sdk_config"]["core"]["data_dir"];
            sdk_config_.log_level = app_config_["sdk_config"]["core"]["log_level"];
            sdk_config_.environment = app_config_["sdk_config"]["core"]["environment"];
            
            sdk_config_.network.name = app_config_["sdk_config"]["network"]["name"];
            sdk_config_.network.rpc_url = app_config_["sdk_config"]["network"]["rpc_url"];
            sdk_config_.network.timeout = app_config_["sdk_config"]["network"]["timeout"];
            sdk_config_.network.retry_attempts = app_config_["sdk_config"]["network"]["retry_attempts"];
            
            sdk_config_.quantum_security.enabled = app_config_["sdk_config"]["security"]["quantum_security"]["enabled"];
            sdk_config_.quantum_security.algorithm = app_config_["sdk_config"]["security"]["quantum_security"]["algorithm"];
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Configuration error: " << e.what() << std::endl;
            return false;
        }
    }
    
    void displayNetworkInfo() {
        try {
            auto network_info = blockchain_client_->getNetworkInfo();
            std::cout << "\n🌐 Network Information:" << std::endl;
            std::cout << "   Network: " << network_info.name << std::endl;
            std::cout << "   Version: " << network_info.version << std::endl;
            std::cout << "   Block Height: " << network_info.block_height << std::endl;
            std::cout << "   Total Supply: " << Utils::formatAmount(network_info.total_supply) << " SATOX" << std::endl;
            std::cout << "   Average Block Time: " << network_info.average_block_time << " seconds" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "⚠️  Could not fetch network information: " << e.what() << std::endl;
        }
    }
    
    void displayMenu() {
        std::cout << "\n📋 Main Menu:" << std::endl;
        std::cout << "   1. Create New Wallet" << std::endl;
        std::cout << "   2. List Wallets" << std::endl;
        std::cout << "   3. Send Transaction" << std::endl;
        std::cout << "   4. View Transaction History" << std::endl;
        std::cout << "   5. View Blockchain Info" << std::endl;
        std::cout << "   6. Backup Wallet" << std::endl;
        std::cout << "   0. Exit" << std::endl;
        std::cout << "\nEnter your choice: ";
    }
    
    std::string getUserChoice() {
        std::string choice;
        std::getline(std::cin, choice);
        return Utils::trim(choice);
    }
    
    void createWallet() {
        std::cout << "\n💰 Create New Wallet" << std::endl;
        std::cout << "===================" << std::endl;
        
        std::string name, password, confirm_password;
        
        std::cout << "Enter wallet name: ";
        std::getline(std::cin, name);
        
        if (!Utils::isValidWalletName(name)) {
            std::cout << "❌ Invalid wallet name. Use only letters, numbers, and underscores." << std::endl;
            return;
        }
        
        std::cout << "Enter password: ";
        std::getline(std::cin, password);
        
        std::cout << "Confirm password: ";
        std::getline(std::cin, confirm_password);
        
        if (password != confirm_password) {
            std::cout << "❌ Passwords do not match." << std::endl;
            return;
        }
        
        if (!Utils::isValidPassword(password)) {
            std::cout << "❌ Password must be at least 8 characters long." << std::endl;
            return;
        }
        
        try {
            if (wallet_manager_->createWallet(name, password)) {
                std::cout << "✅ Wallet created successfully!" << std::endl;
                std::cout << "   Name: " << name << std::endl;
                std::cout << "   Address: " << wallet_manager_->getWalletAddress(name) << std::endl;
                std::cout << "   Balance: " << Utils::formatAmount(wallet_manager_->getWalletBalance(name)) << " SATOX" << std::endl;
            } else {
                std::cout << "❌ Failed to create wallet." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error creating wallet: " << e.what() << std::endl;
        }
    }
    
    void listWallets() {
        std::cout << "\n📋 Your Wallets" << std::endl;
        std::cout << "===============" << std::endl;
        
        try {
            auto wallets = wallet_manager_->listWallets();
            
            if (wallets.empty()) {
                std::cout << "No wallets found. Create one first!" << std::endl;
                return;
            }
            
            for (const auto& wallet_name : wallets) {
                std::cout << "\n   Wallet: " << wallet_name << std::endl;
                std::cout << "   Address: " << wallet_manager_->getWalletAddress(wallet_name) << std::endl;
                std::cout << "   Balance: " << Utils::formatAmount(wallet_manager_->getWalletBalance(wallet_name)) << " SATOX" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error listing wallets: " << e.what() << std::endl;
        }
    }
    
    void sendTransaction() {
        std::cout << "\n📤 Send Transaction" << std::endl;
        std::cout << "==================" << std::endl;
        
        // List available wallets
        auto wallets = wallet_manager_->listWallets();
        if (wallets.empty()) {
            std::cout << "❌ No wallets available. Create one first!" << std::endl;
            return;
        }
        
        std::cout << "Available wallets:" << std::endl;
        for (size_t i = 0; i < wallets.size(); ++i) {
            std::cout << "   " << (i + 1) << ". " << wallets[i] << std::endl;
        }
        
        // Get sender wallet
        std::string wallet_choice;
        std::cout << "Select sender wallet (1-" << wallets.size() << "): ";
        std::getline(std::cin, wallet_choice);
        
        size_t wallet_index = std::stoul(wallet_choice) - 1;
        if (wallet_index >= wallets.size()) {
            std::cout << "❌ Invalid wallet selection." << std::endl;
            return;
        }
        
        std::string from_wallet = wallets[wallet_index];
        
        // Get recipient address
        std::string to_address;
        std::cout << "Enter recipient address: ";
        std::getline(std::cin, to_address);
        
        if (!Utils::isValidAddress(to_address)) {
            std::cout << "❌ Invalid recipient address." << std::endl;
            return;
        }
        
        // Get amount
        std::string amount_str;
        std::cout << "Enter amount (SATOX): ";
        std::getline(std::cin, amount_str);
        
        double amount;
        try {
            amount = std::stod(amount_str);
        } catch (...) {
            std::cout << "❌ Invalid amount." << std::endl;
            return;
        }
        
        if (!Utils::isValidAmount(amount)) {
            std::cout << "❌ Invalid amount." << std::endl;
            return;
        }
        
        // Get message (optional)
        std::string message;
        std::cout << "Enter message (optional): ";
        std::getline(std::cin, message);
        
        try {
            TransactionRequest request;
            request.from_wallet = from_wallet;
            request.to_address = to_address;
            request.amount = amount;
            request.fee = app_config_["app_settings"]["default_fee"];
            request.message = message;
            
            std::cout << "\n📤 Sending transaction..." << std::endl;
            auto result = transaction_handler_->sendTransaction(request);
            
            if (result.success) {
                std::cout << "✅ Transaction sent successfully!" << std::endl;
                std::cout << "   Transaction Hash: " << result.transaction_hash << std::endl;
                std::cout << "   Fee Paid: " << Utils::formatAmount(result.fee_paid) << " SATOX" << std::endl;
                std::cout << "   View on explorer: https://explorer.satox.com/tx/" << result.transaction_hash << std::endl;
            } else {
                std::cout << "❌ Transaction failed: " << result.error_message << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error sending transaction: " << e.what() << std::endl;
        }
    }
    
    void viewTransactionHistory() {
        std::cout << "\n📜 Transaction History" << std::endl;
        std::cout << "======================" << std::endl;
        
        auto wallets = wallet_manager_->listWallets();
        if (wallets.empty()) {
            std::cout << "❌ No wallets available." << std::endl;
            return;
        }
        
        std::cout << "Available wallets:" << std::endl;
        for (size_t i = 0; i < wallets.size(); ++i) {
            std::cout << "   " << (i + 1) << ". " << wallets[i] << std::endl;
        }
        
        std::string wallet_choice;
        std::cout << "Select wallet (1-" << wallets.size() << "): ";
        std::getline(std::cin, wallet_choice);
        
        size_t wallet_index = std::stoul(wallet_choice) - 1;
        if (wallet_index >= wallets.size()) {
            std::cout << "❌ Invalid wallet selection." << std::endl;
            return;
        }
        
        std::string wallet_name = wallets[wallet_index];
        
        try {
            auto transactions = wallet_manager_->getTransactionHistory(wallet_name);
            
            if (transactions.empty()) {
                std::cout << "No transactions found for this wallet." << std::endl;
                return;
            }
            
            std::cout << "\nTransaction History for " << wallet_name << ":" << std::endl;
            std::cout << "=========================================" << std::endl;
            
            for (const auto& tx : transactions) {
                std::cout << "\n   Hash: " << tx.hash << std::endl;
                std::cout << "   Type: " << (tx.is_incoming ? "Received" : "Sent") << std::endl;
                std::cout << "   Amount: " << Utils::formatAmount(tx.amount) << " SATOX" << std::endl;
                std::cout << "   Fee: " << Utils::formatAmount(tx.fee) << " SATOX" << std::endl;
                std::cout << "   Status: " << tx.status << std::endl;
                std::cout << "   Time: " << Utils::formatTimestamp(tx.timestamp) << std::endl;
                
                if (!tx.message.empty()) {
                    std::cout << "   Message: " << tx.message << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error fetching transaction history: " << e.what() << std::endl;
        }
    }
    
    void viewBlockchainInfo() {
        std::cout << "\n🔗 Blockchain Information" << std::endl;
        std::cout << "=========================" << std::endl;
        
        try {
            auto latest_block = blockchain_client_->getLatestBlock();
            
            std::cout << "Latest Block:" << std::endl;
            std::cout << "   Height: " << latest_block.height << std::endl;
            std::cout << "   Hash: " << latest_block.hash << std::endl;
            std::cout << "   Transactions: " << latest_block.transaction_count << std::endl;
            std::cout << "   Total Amount: " << Utils::formatAmount(latest_block.total_amount) << " SATOX" << std::endl;
            std::cout << "   Time: " << Utils::formatTimestamp(latest_block.timestamp) << std::endl;
            
            // Show recent blocks
            auto recent_blocks = blockchain_client_->getRecentBlocks(5);
            std::cout << "\nRecent Blocks:" << std::endl;
            for (const auto& block : recent_blocks) {
                std::cout << "   Block " << block.height << ": " << block.transaction_count << " transactions" << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cout << "❌ Error fetching blockchain information: " << e.what() << std::endl;
        }
    }
    
    void backupWallet() {
        std::cout << "\n💾 Backup Wallet" << std::endl;
        std::cout << "================" << std::endl;
        
        auto wallets = wallet_manager_->listWallets();
        if (wallets.empty()) {
            std::cout << "❌ No wallets available." << std::endl;
            return;
        }
        
        std::cout << "Available wallets:" << std::endl;
        for (size_t i = 0; i < wallets.size(); ++i) {
            std::cout << "   " << (i + 1) << ". " << wallets[i] << std::endl;
        }
        
        std::string wallet_choice;
        std::cout << "Select wallet to backup (1-" << wallets.size() << "): ";
        std::getline(std::cin, wallet_choice);
        
        size_t wallet_index = std::stoul(wallet_choice) - 1;
        if (wallet_index >= wallets.size()) {
            std::cout << "❌ Invalid wallet selection." << std::endl;
            return;
        }
        
        std::string wallet_name = wallets[wallet_index];
        std::string backup_path = "./backups/" + wallet_name + "_backup_" + 
                                 std::to_string(std::time(nullptr)) + ".dat";
        
        try {
            if (wallet_manager_->backupWallet(wallet_name, backup_path)) {
                std::cout << "✅ Wallet backed up successfully!" << std::endl;
                std::cout << "   Backup location: " << backup_path << std::endl;
                std::cout << "   Keep this file safe and secure!" << std::endl;
            } else {
                std::cout << "❌ Failed to backup wallet." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "❌ Error backing up wallet: " << e.what() << std::endl;
        }
    }
};

int main() {
    try {
        MyFirstApp app;
        
        if (!app.initialize()) {
            std::cerr << "❌ Failed to initialize application" << std::endl;
            return 1;
        }
        
        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Step 6: Build and Run

Now let's build and test our application:

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run the application
./my-first-app
```

## Expected Output

When you run the application, you should see:

```
🚀 Starting My First Satox App...
🔧 Initializing Satox SDK...
✅ SDK initialized successfully!

🎯 Welcome to My First Satox App!
=================================

🌐 Network Information:
   Network: testnet
   Version: 1.0.0
   Block Height: 1234567
   Total Supply: 21000000.00000000 SATOX
   Average Block Time: 60.5 seconds

📋 Main Menu:
   1. Create New Wallet
   2. List Wallets
   3. Send Transaction
   4. View Transaction History
   5. View Blockchain Info
   6. Backup Wallet
   0. Exit

Enter your choice:
```

## Testing the Application

### Test Scenario 1: Create a Wallet

1. Choose option `1` from the menu
2. Enter a wallet name (e.g., "my_first_wallet")
3. Enter a secure password
4. Confirm the password
5. Verify the wallet is created successfully

### Test Scenario 2: Send a Transaction

1. Choose option `3` from the menu
2. Select your wallet
3. Enter a recipient address (use a test address)
4. Enter a small amount (e.g., 0.0001 SATOX)
5. Add an optional message
6. Verify the transaction is sent successfully

### Test Scenario 3: View Transaction History

1. Choose option `4` from the menu
2. Select your wallet
3. View the transaction history
4. Verify your recent transaction appears

## Next Steps

Congratulations! You've successfully built your first Satox application. Here are some ideas for extending it:

### Advanced Features to Add

1. **Multi-signature Wallets**: Support for multi-signature transactions
2. **Asset Management**: Create and manage custom assets
3. **Smart Contracts**: Integrate with smart contract functionality
4. **IPFS Integration**: Store data on decentralized storage
5. **GUI Interface**: Add a graphical user interface
6. **Mobile App**: Create a mobile version using the Mobile SDK

### Learning Resources

- **API Reference**: [Complete API documentation](../integration/api-reference.md)
- **Advanced Examples**: [Advanced examples](../examples/)
- **Security Guide**: [Security best practices](../security/)
- **Performance Guide**: [Performance optimization](../advanced/performance.md)

### Community and Support

- **Discord Server**: [Join our community](https://discord.gg/satox)
- **GitHub**: [Report issues and contribute](https://github.com/satoverse/satox-sdk)
- **Documentation**: [Complete documentation](../README.md)

---

*Need help? Check our [troubleshooting guide](../troubleshooting/common-issues.md) or [contact support](https://support.satox.com).*

*Last updated: $(date)* 