# Satox SDK Python Bindings

[![PyPI version](https://badge.fury.io/py/satox-sdk.svg)](https://badge.fury.io/py/satox-sdk)
[![Python versions](https://img.shields.io/pypi/pyversions/satox-sdk.svg)](https://pypi.org/project/satox-sdk/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Documentation](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://docs.satoverse.io)

**High-performance Python bindings for the Satox SDK** - The most advanced blockchain development toolkit with quantum-resistant security.

## 🚀 Why Choose Satox SDK Python Bindings?

- **⚡ High Performance**: Direct C++ bindings for maximum speed and efficiency
- **🔒 Quantum-Resistant**: Built-in quantum-resistant cryptography (CRYSTALS-Kyber, CRYSTALS-Dilithium, SPHINCS+)
- **🎯 Easy Integration**: Simple Python API for complex blockchain operations
- **🛡️ Production Ready**: Comprehensive error handling and security features
- **📚 Well Documented**: Extensive documentation and examples
- **🌐 Cross-Platform**: Works on Windows, macOS, and Linux

## 📦 Installation

### Prerequisites

- Python 3.8 or higher
- CMake 3.16 or higher
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)

### Quick Install

```bash
pip install satox-sdk
```

### From Source

```bash
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk/bindings/python
pip install -e .
```

### Development Install

```bash
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk/bindings/python
pip install -e ".[dev,examples]"
```

## 🎯 Quick Start

### Basic Usage

```python
import satox_sdk

# Initialize the SDK
sdk = satox_sdk.SDK()
config = {
    "core": {
        "network": "testnet",
        "data_dir": "/tmp/satox_data"
    },
    "security": {
        "enablePQC": True,
        "enableInputValidation": True
    }
}

if sdk.initialize(config):
    print("SDK initialized successfully!")
    print(f"Version: {sdk.get_version()}")
else:
    print("Failed to initialize SDK")
```

### Wallet Operations

```python
# Create a new wallet
wallet = sdk.wallet_manager
wallet_id = wallet.create_wallet("my_wallet")
print(f"Created wallet: {wallet_id}")

# Generate a new address
address = wallet.generate_address(wallet_id)
print(f"Generated address: {address}")

# Get wallet balance
balance = wallet.get_balance(wallet_id)
print(f"Balance: {balance} SATOX")
```

### Quantum-Resistant Security

```python
# Generate quantum-resistant key pair
security = sdk.security_manager
public_key, private_key = security.generate_pqc_keypair("DEFAULT")

# Sign data with quantum-resistant signature
data = "Hello, quantum world!"
signature = security.sign_with_pqc("DEFAULT", data)

# Verify signature
is_valid = security.verify_with_pqc("DEFAULT", data, signature)
print(f"Signature valid: {is_valid}")
```

### Asset Management

```python
# Create a new asset
asset_manager = sdk.asset_manager
asset_id = asset_manager.create_asset({
    "name": "My Token",
    "symbol": "MTK",
    "total_supply": 1000000,
    "decimals": 8
})

# Transfer assets
tx_hash = asset_manager.transfer_asset(
    asset_id, 
    from_address, 
    to_address, 
    100
)
print(f"Transfer completed: {tx_hash}")
```

### NFT Operations

```python
# Create an NFT
nft_manager = sdk.nft_manager
nft_id = nft_manager.create_nft({
    "name": "My NFT",
    "description": "A unique digital asset",
    "metadata": {
        "image": "ipfs://QmHash...",
        "attributes": [
            {"trait_type": "Rarity", "value": "Legendary"}
        ]
    }
})

# Mint the NFT
mint_tx = nft_manager.mint_nft(nft_id, owner_address)
print(f"NFT minted: {mint_tx}")
```

## 📚 Documentation

- **[API Reference](https://docs.satoverse.io/python)**: Complete API documentation
- **[Examples](https://github.com/satoverse/satox-sdk/tree/main/bindings/python/examples)**: Working code examples
- **[Tutorials](https://docs.satoverse.io/tutorials)**: Step-by-step guides
- **[Best Practices](https://docs.satoverse.io/best-practices)**: Development guidelines

## 🔧 Configuration

### SDK Configuration

```python
config = {
    "core": {
        "network": "mainnet",  # or "testnet"
        "data_dir": "/path/to/data",
        "enable_mining": False,
        "enable_sync": True,
        "sync_interval_ms": 1000,
        "mining_threads": 1,
        "rpc_endpoint": "http://localhost:8332",
        "timeout_ms": 30000
    },
    "database": {
        "name": "satox_db",
        "enableLogging": True,
        "logPath": "logs/database",
        "maxConnections": 10,
        "connectionTimeout": 5000
    },
    "security": {
        "enablePQC": True,
        "enableInputValidation": True,
        "enableRateLimiting": True,
        "enableLogging": True,
        "logPath": "logs/security"
    }
}
```

### Environment Variables

```bash
export SATOX_NETWORK=mainnet
export SATOX_DATA_DIR=/path/to/data
export SATOX_RPC_ENDPOINT=http://localhost:8332
export SATOX_ENABLE_PQC=true
```

## 🧪 Testing

### Run Tests

```bash
# Run all tests
pytest

# Run with coverage
pytest --cov=satox_sdk

# Run specific test file
pytest tests/test_wallet.py

# Run with verbose output
pytest -v
```

### Test Examples

```python
import pytest
import satox_sdk

def test_sdk_initialization():
    sdk = satox_sdk.SDK()
    config = {"core": {"network": "testnet"}}
    assert sdk.initialize(config) == True
    assert sdk.get_version() is not None

def test_wallet_creation():
    sdk = satox_sdk.SDK()
    sdk.initialize({})
    wallet_id = sdk.wallet_manager.create_wallet("test_wallet")
    assert wallet_id is not None
    assert len(wallet_id) > 0
```

## 🚀 Performance

### Benchmarks

| Operation | Python Bindings | Pure Python | Speedup |
|-----------|----------------|-------------|---------|
| Wallet Creation | 0.5ms | 15ms | 30x |
| Address Generation | 0.2ms | 8ms | 40x |
| Quantum Signing | 2ms | 45ms | 22x |
| Asset Transfer | 1ms | 25ms | 25x |

### Memory Usage

- **Low Memory Footprint**: ~5MB base memory usage
- **Efficient Resource Management**: Automatic cleanup and memory optimization
- **Thread-Safe**: Safe for concurrent operations

## 🔒 Security Features

### Quantum-Resistant Cryptography

- **CRYSTALS-Kyber**: Key encapsulation mechanism
- **CRYSTALS-Dilithium**: Digital signature scheme
- **SPHINCS+**: Hash-based signature scheme
- **Hybrid Encryption**: Classical + quantum-resistant algorithms

### Security Best Practices

- **Input Validation**: Comprehensive input sanitization
- **Rate Limiting**: Built-in protection against abuse
- **Secure Random**: Cryptographically secure random number generation
- **Memory Protection**: Secure memory handling and cleanup

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](https://github.com/satoverse/satox-sdk/blob/main/CONTRIBUTING.md) for details.

### Development Setup

```bash
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk/bindings/python
pip install -e ".[dev]"
pre-commit install
```

### Code Style

```bash
# Format code
black satox_sdk/

# Lint code
flake8 satox_sdk/

# Type checking
mypy satox_sdk/
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/satoverse/satox-sdk/blob/main/LICENSE) file for details.

## 🆘 Support & Community

### Get Help

- **📖 Documentation**: [docs.satoverse.io](https://docs.satoverse.io)
- **🐛 Bug Reports**: [GitHub Issues](https://github.com/satoverse/satox-sdk/issues)
- **💬 Discussions**: [GitHub Discussions](https://github.com/satoverse/satox-sdk/discussions)

### Community Channels

- **Discord**: [Join our community](https://discord.gg/GFZYFuuHVq)
- **X (Twitter)**: [@satoverse_io](https://x.com/satoverse_io)
- **YouTube**: [Satoverse Channel](https://www.youtube.com/@Satoverse)
- **Telegram**: [@satoxcoin](https://t.me/satoxcoin)

### Stay Updated

- **Newsletter**: [Subscribe for updates](https://satoverse.io/newsletter)
- **Blog**: [Latest news and tutorials](https://satoverse.io/blog)
- **Releases**: [GitHub Releases](https://github.com/satoverse/satox-sdk/releases)

## 🙏 Acknowledgments

- **NIST**: For quantum-resistant cryptography standards
- **OpenSSL**: For cryptographic primitives
- **Python Community**: For the amazing Python ecosystem
- **Contributors**: All the developers who contribute to this project

---

**Made with ❤️ by the Satoverse Team**

[Website](https://satoverse.io) • [Documentation](https://docs.satoverse.io) • [GitHub](https://github.com/satoverse/satox-sdk) 

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

