# Hello World - Your First SATOX Application

## Overview

This example demonstrates the most basic usage of the SATOX SDK - initializing the SDK and performing simple blockchain operations. It's the perfect starting point for anyone new to blockchain development with SATOX.

## What You'll Learn

- How to initialize the SATOX SDK
- Basic configuration and setup
- Connecting to the blockchain network
- Retrieving blockchain information
- Proper error handling
- Basic logging and debugging

## Prerequisites

Before running this example, ensure you have:

- **SATOX SDK installed** - Follow the [Getting Started Guide](../../../docs/getting-started.md)
- **API Key** - Get your API key from the [Satoverse Developer Portal](https://your-satox-project.com)
- **C++ Compiler** - GCC 9+ or Clang 10+
- **CMake** - Version 3.14+
- **Basic C++ Knowledge** - Understanding of basic C++ syntax

## Files

- `main.cpp` - Main example code
- `CMakeLists.txt` - Build configuration
- `README.md` - This documentation
- `expected-output.txt` - Expected program output

## How to Run

### Step 1: Set Up Environment

```bash
# Set your API key (replace with your actual key)
export SATOX_API_KEY="your-api-key-here"

# Set network to testnet for safe development
export SATOX_NETWORK="testnet"
```

### Step 2: Build the Example

```bash
# Navigate to the example directory
cd examples/basic-examples/hello-world

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the example
make
```

### Step 3: Run the Example

```bash
# Run the hello world application
./hello-world
```

## Expected Output

You should see output similar to this:

```
=== SATOX SDK Hello World Example ===

Initializing SATOX SDK...
✓ SDK initialized successfully!

Connecting to blockchain network...
✓ Connected to: testnet
✓ Current block: 1234567
✓ Network difficulty: 1.234567

Retrieving wallet information...
✓ Wallet created: SXxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
✓ Balance: 0.00000000 SATOX

Performing basic operations...
✓ Asset creation successful
✓ Asset ID: ASSETxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

Example completed successfully!
```

## Code Explanation

### SDK Initialization

```cpp
// Initialize the SDK with basic configuration
SatoxConfig config;
config.network = "testnet";  // Use testnet for development
config.apiKey = apiKey;      // Your API key
config.logLevel = "INFO";    // Logging level

SatoxSDK sdk;
sdk.initialize(config);
```

This section:
- Creates a configuration object with basic settings
- Sets the network to testnet for safe development
- Configures logging for debugging
- Initializes the SDK with the configuration

### Blockchain Connection

```cpp
// Get blockchain information
BlockchainInfo info = sdk.getBlockchainInfo();
std::cout << "Connected to: " << info.network << std::endl;
std::cout << "Current block: " << info.currentBlock << std::endl;
std::cout << "Difficulty: " << info.difficulty << std::endl;
```

This section:
- Retrieves current blockchain information
- Displays network status and connection details
- Shows current block height and network difficulty

### Wallet Operations

```cpp
// Create a new wallet
Wallet wallet = sdk.createWallet();
std::cout << "Wallet created: " << wallet.getAddress() << std::endl;

// Get wallet balance
Balance balance = sdk.getBalance(wallet.getAddress());
std::cout << "Balance: " << balance.amount << " SATOX" << std::endl;
```

This section:
- Creates a new wallet for the application
- Retrieves the wallet's public address
- Checks the wallet's current balance

### Asset Operations

```cpp
// Create a simple asset
AssetData assetData;
assetData.name = "Hello World Asset";
assetData.description = "My first SATOX asset";
assetData.type = AssetType::COLLECTIBLE;
assetData.quantity = 1;

Asset asset = sdk.createAsset(assetData, wallet.getAddress());
std::cout << "Asset created: " << asset.getId() << std::endl;
```

This section:
- Creates a simple digital asset
- Sets basic asset properties
- Demonstrates asset creation workflow

## Key Concepts

### Testnet vs Mainnet

- **Testnet**: Safe environment for development and testing
- **Mainnet**: Live blockchain network with real value
- **Always use testnet** for development and examples

### API Keys

- **Required**: All SDK operations require a valid API key
- **Security**: Keep your API key secure and never commit it to version control
- **Environment Variables**: Use environment variables for API key management

### Error Handling

```cpp
try {
    // SDK operations
    sdk.initialize(config);
} catch (const SatoxException& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
```

- **Always wrap SDK operations** in try-catch blocks
- **Handle specific exceptions** for better error management
- **Provide meaningful error messages** to users

## Troubleshooting

### Common Issues

**"SDK initialization failed"**
- Check your API key is correct and valid
- Ensure network connectivity
- Verify SATOX SDK is properly installed

**"Network connection failed"**
- Check internet connectivity
- Verify testnet is accessible
- Ensure firewall allows blockchain connections

**"Wallet creation failed"**
- Check system resources
- Verify cryptographic libraries
- Ensure proper permissions

### Debug Mode

Enable debug logging for more detailed information:

```bash
export SATOX_LOG_LEVEL=DEBUG
./hello-world
```

### Getting Help

If you encounter issues:

1. Check the error messages carefully
2. Verify your API key and network settings
3. Review the main documentation
4. Check the troubleshooting section
5. Create an issue on GitHub with detailed information

## Next Steps

After completing this example:

1. **Try Simple Setup**: Learn about more advanced configuration options
2. **Explore Wallet Creation**: Dive deeper into wallet management
3. **Study Asset Basics**: Learn more about digital asset creation
4. **Read the API Reference**: Explore the complete SDK documentation

## Related Examples

- [Simple Setup](../simple-setup/) - More advanced configuration options
- [Wallet Creation](../wallet-creation/) - Detailed wallet management
- [Asset Basics](../asset-basics/) - Comprehensive asset operations

## Support

- **Documentation**: Check the [main docs](../../../docs/) directory
- **API Reference**: Review the [API documentation](../../../docs/api-reference.md)
- **Community**: Join the Satoverse developer community
- **Issues**: Report bugs and request features on GitHub

## License

This example is licensed under the MIT License. See the LICENSE file for details. 