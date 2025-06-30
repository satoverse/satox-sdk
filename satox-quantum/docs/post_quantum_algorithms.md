# Post-Quantum Algorithms Documentation

## Overview
This document provides detailed information about the post-quantum cryptographic algorithms implemented in the Satox Quantum module. These algorithms are designed to be resistant to attacks from both classical and quantum computers.

## Implemented Algorithms

### 1. CRYSTALS-Kyber
- **Type**: Lattice-based
- **Security Level**: 128-bit
- **Key Size**: 1632 bytes (public key), 3168 bytes (private key)
- **Ciphertext Size**: 1568 bytes
- **Performance**: Fast key generation and encryption/decryption
- **Use Case**: General-purpose encryption

### 2. NTRU
- **Type**: Lattice-based
- **Security Level**: 128-bit
- **Key Size**: 699 bytes (public key), 935 bytes (private key)
- **Ciphertext Size**: 699 bytes
- **Performance**: Moderate key generation, fast encryption/decryption
- **Use Case**: Resource-constrained environments

### 3. SABER
- **Type**: Lattice-based
- **Security Level**: 128-bit
- **Key Size**: 992 bytes (public key), 2304 bytes (private key)
- **Ciphertext Size**: 1088 bytes
- **Performance**: Fast key generation and encryption/decryption
- **Use Case**: High-performance applications

### 4. Classic McEliece
- **Type**: Code-based
- **Security Level**: 128-bit
- **Key Size**: 261120 bytes (public key), 6451 bytes (private key)
- **Ciphertext Size**: 128 bytes
- **Performance**: Slow key generation, fast encryption/decryption
- **Use Case**: Long-term security requirements

### 5. BIKE (Bit Flipping Key Encapsulation)
- **Type**: Code-based
- **Security Level**: 128-bit
- **Key Size**: 12323 bytes (public key), 24646 bytes (private key)
- **Ciphertext Size**: 12323 bytes
- **Performance**: Fast key generation, moderate encryption/decryption
- **Use Case**: General-purpose encryption

### 6. HQC (Hamming Quasi-Cyclic)
- **Type**: Code-based
- **Security Level**: 128-bit
- **Key Size**: 17669 bytes (public key), 35338 bytes (private key)
- **Ciphertext Size**: 17669 bytes
- **Performance**: Balanced performance across operations
- **Use Case**: General-purpose encryption

### 7. SIKE (Supersingular Isogeny Key Encapsulation)
- **Type**: Isogeny-based
- **Security Level**: 128-bit
- **Key Size**: 32 bytes (public key), 32 bytes (private key)
- **Ciphertext Size**: Variable (32 + message length)
- **Performance**: Slow key generation, fast encryption/decryption
- **Use Case**: Long-term security requirements

### 8. NTRU Prime
- **Type**: Lattice-based
- **Security Level**: 128-bit
- **Key Size**: 3044 bytes (public key), 6088 bytes (private key)
- **Ciphertext Size**: 3044 bytes
- **Performance**: Moderate key generation, fast encryption/decryption
- **Use Case**: High-security applications
- **Parameters**:
  - Ring dimension (N): 761
  - Small modulus (p): 3
  - Large modulus (q): 4591
  - Number of 1's in f (df): 250
  - Number of 1's in g (dg): 250
  - Number of 1's in r (dr): 250

## Implementation Details

### Polynomial Operations
The implementation includes efficient polynomial operations for code-based algorithms (BIKE and HQC):
- Polynomial multiplication in R = Z[x]/(x^n - 1)
- Polynomial division with error handling
- Polynomial inversion using extended Euclidean algorithm

### Isogeny Computations
The SIKE implementation includes:
- Isogeny point computation
- Isogeny map computation
- Efficient key generation and exchange

### NTRU Prime Specific Operations
- Polynomial inversion in Z[x]/(x^N - 1)
- Polynomial multiplication with modular reduction
- Polynomial centering in [-q/2, q/2]
- Message to polynomial conversion
- Polynomial to message conversion

## Security Considerations

### Key Management
- All keys are generated using cryptographically secure random number generation
- Private keys are never exposed outside the module
- Key rotation is supported for all algorithms
- Key validation is performed before use

### Algorithm Selection
- CRYSTALS-Kyber is recommended for general-purpose encryption
- NTRU is recommended for resource-constrained environments
- Classic McEliece is recommended for long-term security
- SIKE is recommended for applications requiring small key sizes
- NTRU Prime is recommended for high-security applications

## Performance Considerations

### Memory Usage
- All algorithms use efficient memory management
- Large keys (e.g., Classic McEliece) are handled with care
- Memory is cleared after use

### CPU Usage
- Key generation is the most CPU-intensive operation
- Encryption/decryption operations are optimized
- Thread safety is maintained without significant performance impact

## Usage Examples

### Basic Usage
```cpp
#include <satox/quantum/post_quantum_algorithms.hpp>

// Initialize the algorithms
PostQuantumAlgorithms algorithms;
algorithms.initialize();

// Generate key pair
std::string publicKey, privateKey;
algorithms.generateNtruPrimeKeyPair(publicKey, privateKey);

// Encrypt message
std::string message = "Hello, World!";
std::string ciphertext;
algorithms.ntruPrimeEncrypt(message, publicKey, ciphertext);

// Decrypt message
std::string decryptedMessage;
algorithms.ntruPrimeDecrypt(ciphertext, privateKey, decryptedMessage);
```

### Advanced Usage
```cpp
// Use different algorithms
algorithms.generateBikeKeyPair(publicKey, privateKey);
algorithms.bikeEncrypt(message, publicKey, ciphertext);
algorithms.bikeDecrypt(ciphertext, privateKey, decryptedMessage);

// Use HQC
algorithms.generateHqcKeyPair(publicKey, privateKey);
algorithms.hqcEncrypt(message, publicKey, ciphertext);
algorithms.hqcDecrypt(ciphertext, privateKey, decryptedMessage);

// Use SIKE
algorithms.generateSikeKeyPair(publicKey, privateKey);
algorithms.sikeEncrypt(message, publicKey, ciphertext);
algorithms.sikeDecrypt(ciphertext, privateKey, decryptedMessage);

// Use NTRU Prime
algorithms.generateNtruPrimeKeyPair(publicKey, privateKey);
algorithms.ntruPrimeEncrypt(message, publicKey, ciphertext);
algorithms.ntruPrimeDecrypt(ciphertext, privateKey, decryptedMessage);
```

## Error Handling
All operations include comprehensive error handling:
- Invalid input validation
- Memory allocation checks
- Cryptographic operation verification
- Thread safety checks

## Testing
The implementation includes extensive tests:
- Unit tests for each algorithm
- Performance benchmarks
- Memory leak tests
- Edge case tests
- Stress tests

## Dependencies
- libsodium for random number generation
- OpenSSL for additional cryptographic operations
- C++17 or later

## Build Requirements
- CMake 3.10 or later
- C++17 compatible compiler
- libsodium development files
- OpenSSL development files

## Future Work
1. Implement additional post-quantum algorithms:
   - Three Bears
   - Round5
2. Optimize performance for specific use cases
3. Add support for hardware acceleration
4. Enhance key management features
5. Add more comprehensive testing
6. Improve documentation and examples 