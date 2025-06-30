# Quick Start Guide

Get up and running with the Satox SDK in minutes. This guide will walk you through creating your first blockchain application.

## Prerequisites

- Satox SDK installed (see [Installation Guide](installation.md))
- Basic knowledge of C++ programming
- Development environment set up

## Your First Application

### Step 1: Create a New Project

```bash
# Create project directory
mkdir my-satox-app
cd my-satox-app

# Create source file
touch main.cpp
touch CMakeLists.txt
```

### Step 2: Write Your First Code

Create `main.cpp`:

```cpp
#include <satox/sdk.hpp>
#include <iostream>
#include <memory>

int main() {
    try {
        // Initialize the SDK
        satox::SDKConfig config;
        config.network = "testnet";
        config.log_level = "info";
        
        auto sdk = std::make_unique<satox::SDK>(config);
        
        // Create a wallet
        auto wallet = sdk->createWallet("my-wallet");
        std::cout << "Wallet created: " << wallet->getAddress() << std::endl;
        
        // Get wallet balance
        auto balance = wallet->getBalance();
        std::cout << "Balance: " << balance << " SATOX" << std::endl;
        
        // Create a simple transaction
        if (balance > 0) {
            auto tx = wallet->createTransaction("recipient-address", 0.001);
            std::cout << "Transaction created: " << tx->getTxId() << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

### Step 3: Create CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(my-satox-app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Satox SDK
find_package(SatoxSDK REQUIRED)

# Create executable
add_executable(my-satox-app main.cpp)

# Link against Satox SDK
target_link_libraries(my-satox-app SatoxSDK::satox-sdk)
```

### Step 4: Build and Run

```bash
# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run your application
./my-satox-app
```

## Core Concepts

### SDK Initialization

The SDK must be initialized before use:

```cpp
#include <satox/sdk.hpp>

// Basic initialization
satox::SDKConfig config;
config.network = "mainnet";  // or "testnet"
config.log_level = "info";

auto sdk = std::make_unique<satox::SDK>(config);
```

### Wallet Management

Create and manage wallets:

```cpp
// Create a new wallet
auto wallet = sdk->createWallet("wallet-name");

// Import existing wallet
auto imported_wallet = sdk->importWallet("wallet-name", "private-key");

// List all wallets
auto wallets = sdk->listWallets();
for (const auto& w : wallets) {
    std::cout << "Wallet: " << w->getName() << std::endl;
}
```

### Blockchain Operations

Interact with the blockchain:

```cpp
// Get blockchain info
auto info = sdk->getBlockchainInfo();
std::cout << "Current block: " << info.current_block << std::endl;

// Get block by height
auto block = sdk->getBlock(12345);

// Get transaction by ID
auto tx = sdk->getTransaction("tx-id-here");
```

### Asset Management

Manage digital assets:

```cpp
// Create a new asset
auto asset = sdk->createAsset("MyToken", "MTK", 1000000);

// Transfer assets
auto transfer = wallet->transferAsset("recipient", "MTK", 100);

// Get asset balance
auto balance = wallet->getAssetBalance("MTK");
```

## Common Patterns

### Error Handling

Always handle errors properly:

```cpp
try {
    auto result = sdk->someOperation();
    // Handle success
} catch (const satox::SDKException& e) {
    std::cerr << "SDK Error: " << e.what() << std::endl;
    std::cerr << "Error Code: " << e.getErrorCode() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "General Error: " << e.what() << std::endl;
}
```

### Asynchronous Operations

For better performance, use async operations:

```cpp
#include <future>

// Async wallet creation
auto future_wallet = std::async(std::launch::async, [&]() {
    return sdk->createWallet("async-wallet");
});

// Do other work while wallet is being created
// ...

// Get the result
auto wallet = future_wallet.get();
```

### Configuration Management

Load configuration from file:

```cpp
// Load config from file
satox::SDKConfig config;
config.loadFromFile("config.json");

// Or set programmatically
config.network = "mainnet";
config.rpc_url = "https://rpc.satox.com";
config.api_key = "your-api-key";

auto sdk = std::make_unique<satox::SDK>(config);
```

## Testing Your Application

### Unit Testing

Create tests for your application:

```cpp
#include <gtest/gtest.h>
#include <satox/sdk.hpp>

class MyAppTest : public ::testing::Test {
protected:
    void SetUp() override {
        satox::SDKConfig config;
        config.network = "testnet";
        sdk = std::make_unique<satox::SDK>(config);
    }
    
    std::unique_ptr<satox::SDK> sdk;
};

TEST_F(MyAppTest, CreateWallet) {
    auto wallet = sdk->createWallet("test-wallet");
    EXPECT_FALSE(wallet->getAddress().empty());
}
```

### Integration Testing

Test with the testnet:

```cpp
// Use testnet for testing
satox::SDKConfig config;
config.network = "testnet";
config.log_level = "debug";

auto sdk = std::make_unique<satox::SDK>(config);

// Get testnet faucet
auto faucet = sdk->getTestnetFaucet();
auto test_wallet = sdk->createWallet("test-wallet");
faucet->sendToAddress(test_wallet->getAddress(), 1.0);
```

## Best Practices

### Security

- Never hardcode private keys
- Use environment variables for sensitive data
- Always validate inputs
- Use testnet for development

```cpp
// Good: Use environment variables
const char* api_key = std::getenv("SATOX_API_KEY");
if (!api_key) {
    throw std::runtime_error("SATOX_API_KEY not set");
}

// Bad: Hardcoded keys
config.api_key = "hardcoded-key-here";
```

### Performance

- Use connection pooling
- Implement proper error handling
- Cache frequently accessed data
- Use async operations when possible

### Logging

Configure proper logging:

```cpp
satox::SDKConfig config;
config.log_level = "info";
config.log_file = "app.log";

// Log important events
SATOX_LOG_INFO("Application started");
SATOX_LOG_ERROR("Operation failed: {}", error_message);
```

## Next Steps

Now that you have a basic understanding:

1. **Explore Examples**: Check out [Basic Examples](../examples/basic-examples.md)
2. **Learn API**: Read the [API Reference](../api/core-api-reference.md)
3. **Security**: Review [Security Best Practices](../security/security-best-practices.md)
4. **Deployment**: Learn about [Deployment](../deployment/deployment-guide.md)

## Troubleshooting

### Common Issues

**"SDK not found"**
```bash
# Ensure SDK is installed
pkg-config --exists satox-sdk
# Set library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

**"Network connection failed"**
```bash
# Check network connectivity
ping rpc.satox.com
# Verify API key
echo $SATOX_API_KEY
```

**"Wallet creation failed"**
```bash
# Check disk space
df -h
# Verify permissions
ls -la ~/.satox
```

For more help, see the [Troubleshooting Guide](../troubleshooting/common-issues.md).

---

**Last Updated**: $(date '+%Y-%m-%d %H:%M:%S')
**SDK Version**: 1.0.0 