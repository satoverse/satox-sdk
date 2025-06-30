# Satoxcoin Wallet Manager Tests

This directory contains the test suite for the Satoxcoin Wallet Manager. The tests are organized into several categories:

1. Unit Tests
2. Integration Tests
3. Performance Benchmarks
4. Mock Objects

## Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- Google Test and Google Mock
- Google Benchmark
- nlohmann/json
- OpenSSL
- libcurl
- Boost
- Docker (for running local Satoxcoin node)

## Building Tests

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build tests
make

# Run tests
ctest
```

## Running Specific Tests

```bash
# Run all tests
./wallet_manager_tests

# Run specific test
./wallet_manager_tests --gtest_filter=WalletManagerTests.CreateWallet

# Run benchmarks
./wallet_manager_benchmarks
```

## Local Satoxcoin Node Setup

For integration tests, you need a local Satoxcoin node running. You can use Docker:

```bash
# Pull the official Satoxcoin node image
docker pull satoverse/satoxcoin-mainnet

# Run the node
docker run -d \
    --name satoxcoin-node \
    -p 7777:7777 \
    -p 60777:60777 \
    satoverse/satoxcoin-mainnet
```

## Test Categories

### Unit Tests
- Basic wallet operations
- Key management
- Address generation
- Transaction creation and signing
- Asset management
- IPFS operations
- Error handling

### Integration Tests
- Network connection
- Blockchain interaction
- Transaction broadcasting
- Asset creation and transfer
- IPFS data management
- Edge cases
- Error scenarios

### Performance Benchmarks
- Key generation
- Address derivation
- Transaction creation and signing
- Asset operations
- IPFS operations
- Concurrent operations

### Mock Objects
- Network client
- Blockchain client
- IPFS client
- Hardware wallet

## Test Configuration

The tests use the following configuration:

```cpp
NetworkConfig config;
config.network_id = "satoxcoin";
config.chain_id = 9007; // SLIP-044 for Satoxcoin
config.rpc_url = "http://localhost:7777";
config.p2p_port = 60777;
config.timeout = 30;
config.max_retries = 3;
```

## Adding New Tests

1. Create a new test file in the appropriate category
2. Include necessary headers
3. Create test class inheriting from `::testing::Test`
4. Implement test cases using Google Test macros
5. Add test file to `CMakeLists.txt`

Example:

```cpp
#include <gtest/gtest.h>
#include "satox/wallet/wallet_manager.hpp"

namespace satox::wallet {
namespace test {

class NewFeatureTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }

    void TearDown() override {
        // Cleanup code
    }

    WalletManager manager_;
};

TEST_F(NewFeatureTests, TestNewFeature) {
    // Test implementation
}

} // namespace test
} // namespace satox::wallet
```

## Best Practices

1. Use descriptive test names
2. Test both success and failure cases
3. Clean up resources in `TearDown`
4. Use appropriate assertions
5. Mock external dependencies
6. Test edge cases
7. Include performance benchmarks
8. Document test requirements

## Troubleshooting

### Common Issues

1. **Node Connection Failed**
   - Check if Satoxcoin node is running
   - Verify port configuration
   - Check network connectivity

2. **Test Timeout**
   - Increase timeout in configuration
   - Check system resources
   - Verify network latency

3. **Build Errors**
   - Check dependency versions
   - Verify compiler compatibility
   - Check CMake configuration

### Debugging

1. Enable verbose logging:
```cpp
spdlog::set_level(spdlog::level::debug);
```

2. Use debugger:
```bash
gdb ./wallet_manager_tests
```

3. Check test output:
```bash
./wallet_manager_tests --gtest_also_run_disabled_tests --gtest_color=yes
```

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


## Contributing

1. Follow the coding style
2. Add tests for new features
3. Update documentation
4. Run all tests before submitting
5. Include performance benchmarks
6. Add appropriate comments
7. Update this guide if needed 
