# Satox SDK TypeScript Bindings

TypeScript bindings for the Satox SDK, providing native access to Satoxcoin blockchain functionality through Node.js.

## Features

- **Full TypeScript Support**: Complete type definitions for all SDK components
- **Native Performance**: Direct C++ bindings using Node-API (N-API)
- **Comprehensive API**: Access to all Satox SDK managers
- **Modern JavaScript**: ES2020+ features and async/await support
- **Error Handling**: Proper error handling and validation

## Installation

```bash
npm install satox-sdk-typescript
```

## Prerequisites

- Node.js 16.0.0 or higher
- C++ compiler (GCC, Clang, or MSVC)
- CMake 3.15 or higher
- OpenSSL development libraries

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libssl-dev
```

### macOS
```bash
brew install cmake openssl
```

### Windows
- Install Visual Studio Build Tools
- Install CMake from https://cmake.org/
- Install OpenSSL from https://slproweb.com/products/Win32OpenSSL.html

## Quick Start

```typescript
import { 
  BlockchainManager, 
  AssetManager, 
  SecurityManager 
} from 'satox-sdk-typescript';

async function example() {
  // Initialize blockchain manager
  const blockchain = new BlockchainManager();
  blockchain.initialize({ host: 'localhost', port: 7777 });
  
  // Get latest block
  const latestBlock = blockchain.getLatestBlock();
  console.log('Latest block:', latestBlock);
  
  // Initialize security manager
  const security = new SecurityManager();
  security.initialize();
  
  // Generate key pair
  const keyPair = security.generateKeyPair();
  console.log('Generated address:', keyPair.address);
  
  // Initialize asset manager
  const assets = new AssetManager();
  assets.initialize();
  
  // Create an asset
  const assetId = assets.createAsset({
    name: 'My Token',
    symbol: 'MTK',
    description: 'A test token',
    quantity: 1000000,
    decimals: 8,
    reissuable: true,
    owner: keyPair.address
  });
  
  console.log('Created asset:', assetId);
}
```

## API Reference

### BlockchainManager

Manages blockchain connections and operations.

```typescript
import { BlockchainManager, BlockchainConfig } from 'satox-sdk-typescript';

const blockchain = new BlockchainManager();

// Initialize with configuration
blockchain.initialize({
  host: 'localhost',
  port: 7777,
  enableSSL: false
});

// Get blockchain information
const info = blockchain.getInfo();
console.log('Current height:', info.currentHeight);

// Get latest block
const block = blockchain.getLatestBlock();
console.log('Block hash:', block?.hash);

// Get transaction
const tx = blockchain.getTransaction('tx_hash_here');
console.log('Transaction:', tx);
```

### AssetManager

Manages Satoxcoin assets and transfers.

```typescript
import { AssetManager } from 'satox-sdk-typescript';

const assets = new AssetManager();
assets.initialize();

// Create an asset
const assetId = assets.createAsset({
  name: 'My Asset',
  symbol: 'ASSET',
  description: 'Description here',
  quantity: 1000000,
  decimals: 8,
  reissuable: true,
  owner: 'address_here'
});

// Get asset details
const asset = assets.getAsset(assetId);
console.log('Asset:', asset);

// Transfer assets
const txHash = assets.transferAsset({
  assetId: assetId,
  fromAddress: 'from_address',
  toAddress: 'to_address',
  quantity: 100,
  fee: 0.001
});
```

### SecurityManager

Handles cryptographic operations and key management.

```typescript
import { SecurityManager } from 'satox-sdk-typescript';

const security = new SecurityManager();
security.initialize();

// Generate key pair
const keyPair = security.generateKeyPair();
console.log('Address:', keyPair.address);

// Sign a message
const signature = security.signMessage('Hello Satox!', keyPair.privateKey);
console.log('Signature:', signature.signature);

// Verify signature
const isValid = security.verifySignature(
  'Hello Satox!',
  signature.signature,
  signature.publicKey
);
console.log('Valid signature:', isValid);

// Hash data
const hash = security.hashData('data to hash', 'sha256');
console.log('Hash:', hash);
```

### NetworkManager

Manages P2P network connections.

```typescript
import { NetworkManager } from 'satox-sdk-typescript';

const network = new NetworkManager();
network.initialize({
  port: 60777,
  maxConnections: 10,
  enableDiscovery: true
});

// Connect to a peer
network.connect('peer_address:port');

// Get peer information
const peers = network.getPeers();
console.log('Connected peers:', peers.length);

// Send message
network.sendMessage('peer_address', 'Hello peer!');
```

### NFTManager

Manages non-fungible tokens.

```typescript
import { NFTManager } from 'satox-sdk-typescript';

const nfts = new NFTManager();
nfts.initialize();

// Create an NFT
const nftId = nfts.createNFT({
  name: 'My NFT',
  description: 'A unique digital asset',
  creator: 'creator_address',
  owner: 'owner_address',
  assetId: 'asset_id',
  tokenId: 'token_id',
  metadata: JSON.stringify({ image: 'ipfs://...' })
});

// Get NFT details
const nft = nfts.getNFT(nftId);
console.log('NFT:', nft);

// Transfer NFT
const txHash = nfts.transferNFT({
  nftId: nftId,
  fromAddress: 'from_address',
  toAddress: 'to_address',
  fee: 0.001
});
```

### IPFSManager

Manages IPFS file operations.

```typescript
import { IPFSManager } from 'satox-sdk-typescript';

const ipfs = new IPFSManager();
ipfs.initialize({
  gateway: 'https://ipfs.io',
  timeout: 30000
});

// Add file to IPFS
const cid = ipfs.addFile('file content here', 'filename.txt');
console.log('File CID:', cid);

// Get file from IPFS
const file = ipfs.getFile(cid);
console.log('File data:', file?.data);

// Pin file
ipfs.pinFile(cid);

// Add directory
const dirCid = ipfs.addDirectory({
  name: 'my_directory',
  files: [
    { data: 'file1 content', name: 'file1.txt' },
    { data: 'file2 content', name: 'file2.txt' }
  ]
});
```

## Error Handling

All managers provide error handling through the `getLastError()` method:

```typescript
const blockchain = new BlockchainManager();
const success = blockchain.initialize(config);

if (!success) {
  console.error('Initialization failed:', blockchain.getLastError());
  blockchain.clearLastError();
}
```

## Development

### Building from Source

```bash
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk/bindings/typescript
npm install
npm run build
```

### Running Tests

```bash
npm test
```

### Development Scripts

- `npm run build` - Build TypeScript and native bindings
- `npm run clean` - Clean build artifacts
- `npm run test` - Run tests
- `npm run lint` - Run ESLint
- `npm run format` - Format code with Prettier

## Configuration

### Blockchain Configuration

```typescript
interface BlockchainConfig {
  host?: string;        // RPC host (default: localhost)
  port?: number;        // RPC port (default: 7777)
  enableSSL?: boolean;  // Enable SSL (default: false)
}
```

### Network Configuration

```typescript
interface NetworkConfig {
  port?: number;           // P2P port (default: 60777)
  maxConnections?: number; // Max connections (default: 10)
  enableDiscovery?: boolean; // Enable peer discovery
  seedNodes?: string[];    // Seed node addresses
}
```

### IPFS Configuration

```typescript
interface IPFSConfig {
  gateway?: string;     // IPFS gateway URL
  apiEndpoint?: string; // IPFS API endpoint
  timeout?: number;     // Request timeout in ms
  maxFileSize?: number; // Max file size in bytes
}
```

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure you have the required dependencies installed
2. **Runtime Errors**: Check that the Satox SDK C++ library is properly built
3. **Connection Errors**: Verify network configuration and firewall settings

### Getting Help

- Check the [main SDK documentation](../README.md)
- Open an issue on GitHub
- Join the Satox community Discord

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

MIT License - see LICENSE file for details.

## Contributing

Contributions are welcome! Please read the contributing guidelines before submitting pull requests. 
