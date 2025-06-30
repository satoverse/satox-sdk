# Satox SDK Performance Benchmarks

## Overview
This directory will contain performance benchmarks for core SDK operations.

## Planned Benchmarks
- Blockchain: get info, get block by height
- Asset: create asset, get asset info
- NFT: create NFT, get NFT info
- IPFS: add data, get data
- Security: encryption/decryption, signature verification
- Concurrent: parallel asset creation

## Frameworks
- C++: Google Benchmark
- Python: pytest-benchmark
- JavaScript/TypeScript: benchmark.js

## Example (C++ Google Benchmark)
```cpp
#include <benchmark/benchmark.h>
static void BM_CreateAsset(benchmark::State& state) {
  for (auto _ : state) {
    // Call createAsset here
  }
}
BENCHMARK(BM_CreateAsset);
BENCHMARK_MAIN();
```

## Status
- [ ] Blockchain benchmarks
- [ ] Asset benchmarks
- [ ] NFT benchmarks
- [ ] IPFS benchmarks
- [ ] Security benchmarks
- [ ] Concurrent benchmarks 

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

