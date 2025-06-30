# Satox SDK - Rust Bindings

Rust bindings for the Satox SDK, providing a safe and ergonomic interface to interact with the Satoxcoin blockchain, manage assets, NFTs, and IPFS content.

## Features

- **Blockchain Management**: Interact with the Satoxcoin blockchain
- **Asset Management**: Create and manage Satoxcoin assets
- **NFT Support**: Create and manage non-fungible tokens
- **IPFS Integration**: Upload and manage content on IPFS
- **Network Management**: P2P and RPC communication
- **Security**: Cryptographic operations and key management

## Installation

Add this to your `Cargo.toml`:

```toml
[dependencies]
satox-sdk = { git = "https://github.com/satoverse/satox-sdk", package = "satox-sdk" }
```

## Quick Start

```rust
use satox_sdk::{initialize, shutdown, blockchain::BlockchainManager, asset::AssetManager};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize the SDK
    initialize()?;
    
    // Create a blockchain manager
    let blockchain = BlockchainManager::new(Default::default())?;
    
    // Get current block height
    let height = blockchain.get_block_height()?;
    println!("Current block height: {}", height);
    
    // Create an asset manager
    let asset_manager = AssetManager::new()?;
    
    // Create a new asset
    let config = AssetConfig {
        name: "My Token".to_string(),
        symbol: "MTK".to_string(),
        description: Some("A test token".to_string()),
        precision: 8,
        reissuable: true,
        ipfs_hash: None,
    };
    
    let txid = asset_manager.create_asset(config, "owner_address_here")?;
    println!("Asset created with TXID: {}", txid);
    
    // Shutdown the SDK
    shutdown();
    
    Ok(())
}
```

## Examples

### Blockchain Operations

```rust
use satox_sdk::blockchain::{BlockchainManager, BlockchainConfig};

let config = BlockchainConfig {
    rpc_url: "127.0.0.1".to_string(),
    rpc_port: 7777,
    username: Some("rpcuser".to_string()),
    password: Some("rpcpass".to_string()),
    timeout_seconds: 30,
};

let blockchain = BlockchainManager::new(config)?;

// Get block information
let block_info = blockchain.get_block_info("block_hash_here")?;
println!("Block height: {}", block_info.height);

// Get transaction information
let tx_info = blockchain.get_transaction_info("txid_here")?;
println!("Transaction amount: {}", tx_info.amount);
```

### Asset Management

```rust
use satox_sdk::asset::{AssetManager, AssetConfig};

let asset_manager = AssetManager::new()?;

// Create an asset
let config = AssetConfig {
    name: "My Asset".to_string(),
    symbol: "MA".to_string(),
    description: Some("A custom asset".to_string()),
    precision: 8,
    reissuable: true,
    ipfs_hash: None,
};

let txid = asset_manager.create_asset(config, "owner_address")?;

// Get asset information
let asset_info = asset_manager.get_asset_info("asset_id")?;
println!("Asset name: {}", asset_info.name);

// Transfer assets
let transfer_txid = asset_manager.transfer_asset(
    "asset_id",
    "from_address",
    "to_address",
    1000
)?;
```

### NFT Management

```rust
use satox_sdk::nft::{NftManager, NftMetadata, NftAttribute};

let nft_manager = NftManager::new()?;

// Create NFT metadata
let metadata = NftMetadata {
    name: "My NFT".to_string(),
    description: Some("A unique digital asset".to_string()),
    image_url: Some("https://example.com/image.png".to_string()),
    attributes: vec![
        NftAttribute {
            trait_type: "Rarity".to_string(),
            value: "Legendary".to_string(),
        },
    ],
    external_url: Some("https://example.com".to_string()),
};

// Create an NFT
let txid = nft_manager.create_nft("asset_id", metadata, "owner_address")?;

// Get NFT information
let nft_info = nft_manager.get_nft_info("nft_id")?;
println!("NFT name: {}", nft_info.metadata.name);
```

### IPFS Operations

```rust
use satox_sdk::ipfs::{IpfsManager, IpfsConfig};

let config = IpfsConfig {
    gateway_url: "https://ipfs.io".to_string(),
    api_url: Some("https://api.ipfs.io".to_string()),
    timeout_seconds: 30,
    max_file_size: 100 * 1024 * 1024, // 100MB
};

let ipfs = IpfsManager::new(config)?;

// Upload a file
let file_info = ipfs.upload_file("/path/to/file.jpg")?;
println!("File uploaded with hash: {}", file_info.hash);

// Get gateway URL
let url = ipfs.get_gateway_url(&file_info.hash)?;
println!("Access URL: {}", url);

// Download a file
ipfs.download_file("ipfs_hash", "/path/to/download.jpg")?;
```

### Security Operations

```rust
use satox_sdk::security::SecurityManager;

let security = SecurityManager::new()?;

// Generate a key pair
let (public_key, private_key) = security.generate_keypair()?;

// Sign data
let signature = security.sign_data("Hello, Satox!", &private_key)?;

// Verify signature
let is_valid = security.verify_signature("Hello, Satox!", &signature, &public_key)?;
println!("Signature valid: {}", is_valid);

// Hash data
let hash = security.sha256("Hello, Satox!")?;
println!("SHA256 hash: {}", hash);
```

## Error Handling

The SDK uses Rust's `Result` type for error handling. All operations return a `SatoxResult<T>` which can be either `Ok(T)` or `Err(SatoxError)`.

```rust
use satox_sdk::error::SatoxError;

match blockchain.get_block_height() {
    Ok(height) => println!("Block height: {}", height),
    Err(SatoxError::BlockchainError(msg)) => eprintln!("Blockchain error: {}", msg),
    Err(e) => eprintln!("Other error: {:?}", e),
}
```

## Configuration

The SDK components can be configured using their respective config structs:

- `BlockchainConfig`: RPC connection settings
- `NetworkConfig`: P2P and network settings
- `IpfsConfig`: IPFS gateway and API settings

## Building from Source

To build the Rust bindings from source:

```bash
cd bindings/rust
cargo build --release
```

## Testing

Run the tests:

```bash
cd bindings/rust
cargo test
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


## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Support

For support and questions, please open an issue on the GitHub repository. 
