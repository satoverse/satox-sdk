# Satox SDK WASM Bindings

## Overview
This directory will contain the WASM bindings for the Satox SDK, exposing core functionality (NFTManager, IPFS Client, Blockchain Client) to JavaScript and other WASM consumers.

## Planned Structure
- `nft_manager.cpp` / `nft_manager.h` → Compiled to WASM, exposes NFT CRUD
- `ipfs_client.cpp` / `ipfs_client.h` → Compiled to WASM, exposes add/get
- `blockchain_client.cpp` / `blockchain_client.h` → Compiled to WASM, exposes block/tx retrieval

## Build
- Use Emscripten to compile C++ sources to WASM
- Example: `emcc nft_manager.cpp -o nft_manager.wasm -s EXPORTED_FUNCTIONS=...`

## Usage Example
```
import init, { create_nft, get_nft } from './nft_manager_wasm.js';

await init();
const id = create_nft('id1', { ... });
const nft = get_nft('id1');
```

## Status
- [ ] NFTManager WASM binding
- [ ] IPFSClient WASM binding
- [ ] BlockchainClient WASM binding 

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

