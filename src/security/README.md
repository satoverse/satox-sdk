# Satox Security Component

## Overview
This component provides comprehensive security functionality for the Satox SDK, including:
- Post-Quantum Cryptography (PQC) implementation
- Input validation and sanitization
- Rate limiting and security monitoring
- General security management

## Features
- **PQC Support**: ML-DSA, ML-KEM, Hybrid encryption
- **Input Validation**: Email, URL, IP address validation
- **Rate Limiting**: Configurable rate limiting per operation
- **Security Monitoring**: Comprehensive security event tracking
- **Thread Safety**: All operations are thread-safe

## Usage
```cpp
#include "satox/security/security_manager.hpp"

// Initialize the security manager
auto& security = satox::security::SecurityManager::getInstance();

// Configure security
satox::security::SecurityConfig config;
config.enablePQC = true;
config.enableInputValidation = true;
config.enableRateLimiting = true;

if (!security.initialize(config)) {
    std::cerr << "Failed to initialize security: " << security.getLastError() << std::endl;
    return 1;
}

// Use PQC for signing
if (security.signWithPQC("ML-DSA", "data_to_sign")) {
    std::cout << "Data signed successfully!" << std::endl;
}

// Validate input
if (security.validateEmail("user@example.com")) {
    std::cout << "Valid email address!" << std::endl;
}

// Check rate limit
if (security.checkRateLimit("user123", "login")) {
    std::cout << "Rate limit OK!" << std::endl;
}
```

## Building
```bash
mkdir build && cd build
cmake ..
make
```

## Testing
```bash
make test
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

