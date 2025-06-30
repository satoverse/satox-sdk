# Satox SDK Go Bindings

[![Go Version](https://img.shields.io/github/go-mod/go-version/satoverse/satox-sdk)](https://golang.org/dl/)
[![Go Report Card](https://goreportcard.com/badge/github.com/satoverse/satox-sdk)](https://goreportcard.com/report/github.com/satoverse/satox-sdk)
[![GoDoc](https://godoc.org/github.com/satoverse/satox-sdk?status.svg)](https://godoc.org/github.com/satoverse/satox-sdk)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Documentation](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://docs.satoverse.io)

**High-performance Go bindings for the Satox SDK** - The most advanced blockchain development toolkit with quantum-resistant security.

## 🚀 Why Choose Satox SDK Go Bindings?

- **⚡ High Performance**: Direct C++ bindings for maximum speed and efficiency
- **🔒 Quantum-Resistant**: Built-in quantum-resistant cryptography (CRYSTALS-Kyber, CRYSTALS-Dilithium, SPHINCS+)
- **🛡️ Enterprise Security**: Military-grade encryption and security features
- **🌐 Multi-Network**: Support for mainnet, testnet, and regtest networks
- **📦 Easy Integration**: Simple Go module with comprehensive documentation
- **🔧 Full SDK Access**: Complete access to all SDK components and features
- **📈 Production Ready**: Battle-tested in enterprise environments

## 📦 Installation

```bash
go get github.com/satoverse/satox-sdk
```

## 🚀 Quick Start

```go
package main

import (
    "fmt"
    "log"
    
    "github.com/satoverse/satox-sdk"
)

func main() {
    // Initialize SDK
    sdk := satox.NewSDK()
    
    // Configure for testnet
    config := satox.DefaultConfig()
    config.Core.Network = "testnet"
    
    // Initialize
    if err := sdk.Initialize(config); err != nil {
        log.Fatal("Failed to initialize SDK:", err)
    }
    defer sdk.Shutdown()
    
    // Create a wallet
    walletID, err := sdk.Wallet.CreateWallet("My Wallet")
    if err != nil {
        log.Fatal("Failed to create wallet:", err)
    }
    
    // Generate an address
    address, err := sdk.Wallet.GenerateAddress(walletID)
    if err != nil {
        log.Fatal("Failed to generate address:", err)
    }
    
    fmt.Printf("Wallet created: %s\n", walletID)
    fmt.Printf("Address generated: %s\n", address)
}
```

## 🔧 Configuration

```go
config := satox.Config{
    Core: satox.CoreConfig{
        Network:        "testnet",
        DataDir:        "~/.satox",
        EnableMining:   false,
        EnableSync:     true,
        SyncIntervalMs: 1000,
        MiningThreads:  1,
        TimeoutMs:      30000,
    },
    Security: satox.SecurityConfig{
        EnablePQC:           true,
        EnableInputValidation: true,
        EnableRateLimiting:   true,
    },
    Wallet: satox.WalletConfig{
        EncryptionEnabled: true,
        BackupEnabled:     true,
        AutoBackupInterval: 86400,
    },
}
```

## 💰 Wallet Management

```go
// Create wallet
walletID, err := sdk.Wallet.CreateWallet("My Wallet")

// Generate address
address, err := sdk.Wallet.GenerateAddress(walletID)

// Get balance
balance, err := sdk.Wallet.GetBalance(walletID)

// Send transaction
txID, err := sdk.Wallet.SendTransaction(fromAddress, toAddress, amount, fee)

// Get wallet info
info, err := sdk.Wallet.GetWalletInfo(walletID)
```

## 🔐 Security Features

```go
// Generate quantum-resistant key pair
publicKey, privateKey, err := sdk.Security.GeneratePQCKeyPair("my-key")

// Sign data
signature, err := sdk.Security.SignWithPQC("my-key", data)

// Verify signature
valid, err := sdk.Security.VerifyWithPQC("my-key", data, signature)

// Encrypt data
encrypted, err := sdk.Security.EncryptWithPQC("my-key", data)

// Decrypt data
decrypted, err := sdk.Security.DecryptWithPQC("my-key", encrypted)
```

## 🎨 Asset Management

```go
// Create asset
metadata := satox.AssetMetadata{
    Name:        "My Token",
    Symbol:      "MTK",
    TotalSupply: 1000000,
    Decimals:    8,
    Description: "My custom token",
}

assetID, err := sdk.Asset.CreateAsset(metadata)

// Transfer asset
txID, err := sdk.Asset.TransferAsset(assetID, fromAddress, toAddress, amount)

// Get asset balance
balance, err := sdk.Asset.GetAssetBalance(assetID, address)

// Get asset info
info, err := sdk.Asset.GetAssetInfo(assetID)
```

## 🖼️ NFT Management

```go
// Create NFT
metadata := satox.NFTMetadata{
    Name:        "My NFT",
    Description: "A unique digital asset",
    Image:       "https://example.com/image.png",
    Attributes: []satox.NFTAttribute{
        {TraitType: "Rarity", Value: "Legendary"},
        {TraitType: "Level", Value: 100},
    },
}

nftID, err := sdk.NFT.CreateNFT(metadata)

// Mint NFT
txID, err := sdk.NFT.MintNFT(nftID, ownerAddress)

// Transfer NFT
txID, err := sdk.NFT.TransferNFT(nftID, fromAddress, toAddress)

// Get NFT info
info, err := sdk.NFT.GetNFTInfo(nftID)
```

## 🌐 IPFS Integration

```go
// Upload file to IPFS
hash, err := sdk.IPFS.UploadFile("path/to/file.txt")

// Download file from IPFS
data, err := sdk.IPFS.DownloadFile(hash)

// Pin content
err := sdk.IPFS.PinContent(hash)

// Get file info
info, err := sdk.IPFS.GetFileInfo(hash)
```

## 🔗 Blockchain Integration

```go
// Get blockchain info
info, err := sdk.Blockchain.GetInfo()

// Get block by height
block, err := sdk.Blockchain.GetBlockByHeight(height)

// Get transaction
tx, err := sdk.Blockchain.GetTransaction(txID)

// Get current height
height, err := sdk.Blockchain.GetCurrentHeight()
```

## 🗄️ Database Operations

```go
// Execute query
result, err := sdk.Database.ExecuteQuery("SELECT * FROM wallets WHERE id = ?", walletID)

// Execute transaction
err := sdk.Database.ExecuteTransaction(func(tx *satox.Transaction) error {
    // Multiple operations
    return nil
})

// Get database stats
stats, err := sdk.Database.GetStats()
```

## 🌐 Network Management

```go
// Get connection info
info, err := sdk.Network.GetConnectionInfo()

// Get peer list
peers, err := sdk.Network.GetPeers()

// Ban peer
err := sdk.Network.BanPeer(address, reason)

// Get network stats
stats, err := sdk.Network.GetStats()
```

## 📊 Event Handling

```go
// Subscribe to events
sdk.On("wallet.created", func(event satox.WalletEvent) {
    fmt.Printf("Wallet created: %s\n", event.WalletID)
})

sdk.On("transaction.sent", func(event satox.TransactionEvent) {
    fmt.Printf("Transaction sent: %s\n", event.TransactionID)
})

sdk.On("asset.created", func(event satox.AssetEvent) {
    fmt.Printf("Asset created: %s\n", event.AssetID)
})
```

## 🧪 Testing

```go
package main

import (
    "testing"
    
    "github.com/satoverse/satox-sdk"
    "github.com/stretchr/testify/assert"
)

func TestWalletCreation(t *testing.T) {
    sdk := satox.NewSDK()
    config := satox.DefaultConfig()
    config.Core.Network = "regtest"
    
    err := sdk.Initialize(config)
    assert.NoError(t, err)
    defer sdk.Shutdown()
    
    walletID, err := sdk.Wallet.CreateWallet("Test Wallet")
    assert.NoError(t, err)
    assert.NotEmpty(t, walletID)
    
    address, err := sdk.Wallet.GenerateAddress(walletID)
    assert.NoError(t, err)
    assert.NotEmpty(t, address)
}
```

## 📚 API Documentation

Full API documentation is available at [godoc.org](https://godoc.org/github.com/satoverse/satox-sdk).

## 🔧 Building from Source

```bash
# Clone repository
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk/bindings/go

# Build
go build -o satox-sdk .

# Run tests
go test ./...

# Run benchmarks
go test -bench=.
```

## 🚀 Performance

The Go bindings provide near-native performance with minimal overhead:

- **Wallet Operations**: < 1ms for address generation
- **Transaction Signing**: < 5ms for standard transactions
- **Quantum Signatures**: < 10ms for PQC operations
- **Asset Transfers**: < 2ms for asset operations
- **IPFS Uploads**: Network-limited performance

## 🔒 Security Features

- **Quantum-Resistant Cryptography**: CRYSTALS-Kyber, CRYSTALS-Dilithium, SPHINCS+
- **Secure Key Management**: Hardware security module (HSM) support
- **Input Validation**: Comprehensive validation for all inputs
- **Rate Limiting**: Built-in rate limiting for API operations
- **Audit Logging**: Complete audit trail for all operations

## 🌍 Network Support

- **Mainnet**: Production network with real assets
- **Testnet**: Testing network with test assets
- **Regtest**: Local development network

## 📦 Dependencies

- **Go**: 1.21 or later
- **C++ Runtime**: libstdc++ or libc++
- **OpenSSL**: 1.1.1 or later
- **System Libraries**: Standard system libraries

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](https://github.com/satoverse/satox-sdk/blob/main/CONTRIBUTING.md) for details.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/satoverse/satox-sdk/blob/main/LICENSE) file for details.

## 🆘 Support

- **Documentation**: [docs.satoverse.io](https://docs.satoverse.io)
- **Issues**: [GitHub Issues](https://github.com/satoverse/satox-sdk/issues)
- **Discussions**: [GitHub Discussions](https://github.com/satoverse/satox-sdk/discussions)
- **Email**: dev@satoverse.io

## 🌟 Features

- ✅ **Wallet Management**: Create, manage, and backup wallets
- ✅ **Address Generation**: Generate secure addresses
- ✅ **Transaction Handling**: Send and receive transactions
- ✅ **Asset Management**: Create and manage custom assets
- ✅ **NFT Support**: Full NFT creation and management
- ✅ **IPFS Integration**: Decentralized file storage
- ✅ **Quantum Security**: Post-quantum cryptography
- ✅ **Multi-Network**: Support for all networks
- ✅ **Event System**: Real-time event handling
- ✅ **Database Operations**: Full database access
- ✅ **Network Management**: Peer and connection management
- ✅ **Comprehensive Testing**: Full test coverage
- ✅ **Production Ready**: Enterprise-grade reliability

---

**Built with ❤️ by the Satoverse Team** 

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

