# Satox Asset Component

## Overview
This component provides comprehensive asset management functionality for the Satox SDK, following the unified architecture pattern and template compliance requirements.

## Features
- Asset creation and management
- Asset transfers and balance tracking
- Asset reissuance and burning
- Asset freezing and unfreezing
- Comprehensive validation and verification
- Statistics and monitoring
- Health checks and error handling
- Callback system for event notifications

## Architecture
The Asset component follows the established template patterns:
- **Namespace**: `satox::asset`
- **Singleton Pattern**: `AssetManager::getInstance()`
- **Thread Safety**: Proper mutex usage for all operations
- **Error Handling**: Comprehensive error management with context
- **Logging**: Structured logging to `logs/components/asset/`
- **Statistics**: Performance monitoring and metrics collection

## Usage

### Basic Initialization
```cpp
#include "satox/asset/asset_manager.hpp"

// Get the manager instance
auto& manager = satox::asset::AssetManager::getInstance();

// Initialize with configuration
satox::asset::AssetConfig config;
config.name = "my_asset_manager";
config.type = satox::asset::AssetType::TOKEN;
config.maxAssets = 1000;
config.timeout = 30;
config.enableLogging = true;
config.logPath = "logs/components/asset/";

if (!manager.initialize(config)) {
    std::cerr << "Initialization failed: " << manager.getLastError() << std::endl;
    return 1;
}
```

### Asset Creation
```cpp
// Create a new asset
satox::asset::AssetCreationRequest request;
request.name = "MyToken";
request.symbol = "MTK";
request.amount = 1000000;
request.units = 8;
request.reissuable = true;
request.owner_address = "alice@example.com";
request.metadata["description"] = "My custom token";

if (manager.createAsset(request)) {
    std::cout << "Asset created successfully!" << std::endl;
}
```

### Asset Transfers
```cpp
// Transfer assets between addresses
satox::asset::AssetTransferRequest transfer;
transfer.asset_name = "MyToken";
transfer.from_address = "alice@example.com";
transfer.to_address = "bob@example.com";
transfer.amount = 100000;
transfer.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();

if (manager.transferAsset(transfer)) {
    std::cout << "Transfer successful!" << std::endl;
}
```

### Asset Operations
```cpp
// Reissue assets (if reissuable)
satox::asset::AssetReissueRequest reissue;
reissue.asset_name = "MyToken";
reissue.owner_address = "alice@example.com";
reissue.amount = 500000;
reissue.reissuable = true;
reissue.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();

manager.reissueAsset(reissue);

// Burn assets
satox::asset::AssetBurnRequest burn;
burn.asset_name = "MyToken";
burn.owner_address = "alice@example.com";
burn.amount = 100000;
burn.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();

manager.burnAsset(burn);

// Freeze/unfreeze assets
manager.freezeAsset("MyToken", "alice@example.com");
manager.unfreezeAsset("MyToken", "alice@example.com");
```

### Asset Queries
```cpp
// Get asset metadata
satox::asset::AssetMetadata metadata;
if (manager.getAssetMetadata("MyToken", metadata)) {
    std::cout << "Asset: " << metadata.name << " (" << metadata.symbol << ")" << std::endl;
    std::cout << "Total supply: " << metadata.total_supply << std::endl;
}

// Check balances
uint64_t balance;
if (manager.getAssetBalance("alice@example.com", "MyToken", balance)) {
    std::cout << "Balance: " << balance << std::endl;
}

// List all assets
auto assets = manager.listAssets();
for (const auto& asset : assets) {
    std::cout << asset["name"].get<std::string>() << std::endl;
}
```

### Validation and Verification
```cpp
// Validate asset metadata
satox::asset::AssetMetadata metadata;
metadata.name = "TestAsset";
metadata.symbol = "TEST";
metadata.total_supply = 1000000;

satox::asset::AssetVerificationResult result;
if (manager.validateAssetMetadata(metadata, result)) {
    if (result.valid) {
        std::cout << "Metadata is valid" << std::endl;
    } else {
        std::cout << "Validation failed: " << result.error_message << std::endl;
    }
}

// Verify asset
if (manager.verifyAsset("MyToken", result)) {
    if (result.valid) {
        std::cout << "Asset is valid" << std::endl;
    }
}
```

### Statistics and Monitoring
```cpp
// Enable statistics
manager.enableStats(true);

// Get statistics
auto stats = manager.getStats();
std::cout << "Total operations: " << stats.totalOperations << std::endl;
std::cout << "Successful operations: " << stats.successfulOperations << std::endl;
std::cout << "Failed operations: " << stats.failedOperations << std::endl;

// Health check
if (manager.healthCheck()) {
    auto health = manager.getHealthStatus();
    std::cout << "Health: OK" << std::endl;
    std::cout << "Total assets: " << health["total_assets"].get<int>() << std::endl;
}
```

### Callbacks and Event Handling
```cpp
// Register callbacks
manager.registerAssetCallback([](const std::string& event, bool success) {
    std::cout << "Asset event: " << event << " - " << (success ? "SUCCESS" : "FAILED") << std::endl;
});

manager.registerErrorCallback([](const std::string& operation, const std::string& error) {
    std::cerr << "Error in " << operation << ": " << error << std::endl;
});
```

## Building
```bash
mkdir build && cd build
cmake ..
make
```

## Testing
```bash
# Run all tests
make test

# Run specific test
./satox-asset-tests
```

## Examples
```bash
# Run the basic usage example
./asset-example
```

## Configuration Options

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| name | string | Manager name | - |
| type | AssetType | Asset type (TOKEN, NFT, etc.) | TOKEN |
| maxAssets | int | Maximum number of assets | 1000 |
| timeout | int | Operation timeout in seconds | 30 |
| enableLogging | bool | Enable logging | true |
| logPath | string | Log file path | - |

## Error Handling
The component provides comprehensive error handling:
- All operations return boolean success/failure
- Detailed error messages via `getLastError()`
- Error callbacks for event-driven error handling
- Validation results with detailed feedback

## Thread Safety
All operations are thread-safe using proper mutex synchronization:
- Read operations use shared locks where appropriate
- Write operations use exclusive locks
- Callbacks are executed safely

## Logging
Logs are written to the configured log path with structured format:
- Timestamps and log levels
- Operation context and results
- Performance metrics
- Error details and stack traces

## Compliance
This component follows all SDK template requirements:
- ✅ Template-compliant structure
- ✅ Proper namespace usage (`satox::asset`)
- ✅ Singleton pattern implementation
- ✅ Thread-safe operations
- ✅ Comprehensive error handling
- ✅ Statistics and monitoring
- ✅ Health check functionality
- ✅ Callback registration system
- ✅ Configuration validation
- ✅ Centralized logging structure

## ⚠️ Limitations

### **Blockchain Compatibility**

- **No Smart Contract Support:** Satox is based on Ravencoin and does **not** support smart contracts or EVM compatibility
- **Asset-Based:** Focuses on asset creation, management, and transfer rather than programmable contracts
- **UTXO Model:** Uses Unspent Transaction Output (UTXO) model like Bitcoin/Ravencoin, not account-based like Ethereum

### **Platform Support**

- **Desktop Focus:** Primarily designed for desktop/server environments
- **Mobile Limitations:** Mobile SDK provides limited functionality compared to core SDK
- **Browser Support:** WebAssembly bindings have performance limitations for complex operations

### **Network Requirements**

- **Internet Connection:** Requires active internet connection for blockchain operations
- **Node Synchronization:** Full node operations require significant storage and bandwidth
- **API Dependencies:** Some features depend on external API services

### **Development Considerations**

- **Learning Curve:** Ravencoin/UTXO model differs from Ethereum/smart contract development
- **Testing Environment:** Testnet required for development and testing
- **Asset Management:** Asset creation and management have specific requirements and limitations

