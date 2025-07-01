# Satox SDK JavaScript Bindings

JavaScript/Node.js bindings for the Satox blockchain SDK, providing access to blockchain operations, NFT management, asset creation, and post-quantum cryptography.

## Installation

```bash
npm install satox-sdk
```

## Quick Start

```javascript
const { SatoxManager, TransactionManager, PostQuantumAlgorithms } = require('satox-sdk');

// Initialize the SDK
const manager = new SatoxManager({
    networkId: 'mainnet',
    apiEndpoint: 'http://localhost:7777',
    debugMode: true
});

await manager.initialize();

// Create a transaction
const txManager = new TransactionManager({
    fee: 0.001,
    maxSize: 1000000,
    enableRBF: true
});

await txManager.initialize();

const transaction = await txManager.createTransaction({
    inputs: [],
    outputs: [{ address: 'test-address', amount: 1.0 }]
});

// Use post-quantum cryptography
const algorithms = new PostQuantumAlgorithms();
await algorithms.initialize();

const keyPair = await algorithms.generateKeyPair('FALCON-512');
```

## Features

- **Blockchain Operations**: Create and manage transactions
- **NFT Management**: Create and manage non-fungible tokens
- **Asset Creation**: Create and manage digital assets
- **Post-Quantum Cryptography**: FALCON-512 and CRYSTALS-Kyber support
- **IPFS Integration**: Add and retrieve data from IPFS
- **Component System**: Modular architecture for extensibility

## API Reference

### SatoxManager

Main SDK manager for blockchain operations.

```javascript
const manager = new SatoxManager(config);
await manager.initialize();
```

#### Configuration Options

- `networkId`: Network to connect to ('mainnet', 'testnet')
- `apiEndpoint`: API endpoint URL
- `debugMode`: Enable debug logging
- `maxRetries`: Maximum retry attempts
- `timeout`: Request timeout in milliseconds

#### Methods

- `initialize()`: Initialize the SDK
- `shutdown()`: Shutdown the SDK
- `registerComponent(component)`: Register a custom component
- `getComponent(name)`: Get a registered component
- `createTransaction(params)`: Create a new transaction
- `createNFT(params)`: Create a new NFT
- `createAsset(params)`: Create a new asset
- `addToIPFS(data)`: Add data to IPFS

### TransactionManager

Manages blockchain transactions.

```javascript
const txManager = new TransactionManager({
    fee: 0.001,
    maxSize: 1000000,
    enableRBF: true
});
```

#### Methods

- `createTransaction(params)`: Create a new transaction
- `signTransaction(transaction, privateKey)`: Sign a transaction
- `broadcastTransaction(transaction)`: Broadcast a transaction
- `getTransaction(txid)`: Get transaction details

### PostQuantumAlgorithms

Post-quantum cryptography operations.

```javascript
const algorithms = new PostQuantumAlgorithms();
await algorithms.initialize();
```

#### Supported Algorithms

- **FALCON-512**: Post-quantum signature algorithm
- **CRYSTALS-Kyber**: Post-quantum key encapsulation mechanism
- **CRYSTALS-Dilithium**: Post-quantum signature algorithm

#### Methods

- `generateKeyPair(algorithm)`: Generate a key pair
- `encrypt(publicKey, message)`: Encrypt a message
- `decrypt(privateKey, ciphertext)`: Decrypt a message
- `sign(privateKey, message)`: Sign a message
- `verify(publicKey, message, signature)`: Verify a signature
- `getAvailableAlgorithms()`: Get list of available algorithms
- `isAlgorithmAvailable(algorithm)`: Check if algorithm is available
- `isAlgorithmRecommended(algorithm)`: Check if algorithm is recommended

## Development

### Prerequisites

- Node.js 16+
- Python 3.8+
- CMake 3.14+
- GCC 9+ or Clang 10+

### Building from Source

```bash
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk/bindings/javascript
npm install
npm run build
npm test
```

### Running Tests

```bash
npm test
```

## License

MIT License - see LICENSE file for details.

## Support

- GitHub Issues: [https://github.com/satoverse/satox-sdk/issues](https://github.com/satoverse/satox-sdk/issues)
- Documentation: [https://docs.satoverse.io](https://docs.satoverse.io)

## Version History

- **0.9.0**: Initial release with core functionality
- Basic blockchain operations
- Post-quantum cryptography support
- Component system architecture 