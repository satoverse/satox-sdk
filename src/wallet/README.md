# Satox Wallet Library

A standalone C++ wallet library for the Satox blockchain, providing key management, address derivation, and cryptographic utilities.

## Features
- Key pair generation and validation
- Address derivation
- Secure cryptographic operations using OpenSSL EVP API
- Designed for standalone use or SDK integration

## Build Instructions
```sh
mkdir build
cd build
cmake ..
make
```

## Install
```sh
make install
```

## Standalone Usage Example
```cpp
#include <satox/wallet/key_manager.hpp>
using namespace satox::wallet;

int main() {
    KeyManager km;
    km.initialize();
    KeyManager::KeyPair kp;
    if (km.generateKeyPair(kp)) {
        // Use kp.privateKey, kp.publicKey, kp.address
    }
}
```

## SDK Integration Example
In your SDK's CMakeLists.txt:
```cmake
add_subdirectory(path/to/satox-wallet)
target_link_libraries(satox-sdk PRIVATE satox-wallet)
target_include_directories(satox-sdk PRIVATE path/to/satox-wallet/include)
```

## Testing
```sh
cd build
ctest
```

## API Documentation
- See `include/satox/wallet/key_manager.hpp` for Doxygen-style comments and API details.

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
MIT
