# Quantum Security in Satox Core SDK

## What is Quantum Security?

Quantum security protects your data against attacks from quantum computers. Traditional encryption (RSA, ECC) can be broken by quantum computers, so we use post-quantum cryptography (PQC) for long-term protection.

## Why It Matters

- **Future-proof**: Protects data for years to come
- **Compliance**: Meets regulatory requirements
- **Competitive**: Early adoption advantage
- **Security**: Resistant to quantum attacks

## Supported Algorithms

### Key Exchange (KEM)
- **Kyber-512**: Lightweight, IoT devices
- **Kyber-768**: Balanced performance
- **Kyber-1024**: Maximum security (recommended)

### Digital Signatures
- **Dilithium2**: Fast signing
- **Dilithium3**: Balanced security
- **Dilithium5**: Maximum security
- **SPHINCS+**: Hash-based backup option

## Quick Start

```cpp
#include <satox/core/config.hpp>

// Enable quantum security
satox::CoreConfig config;
config.quantum_security.enabled = true;
config.quantum_security.algorithm = "kyber-1024";
config.quantum_security.hybrid_mode = true;

// Initialize SDK
auto manager = std::make_unique<satox::CoreManager>();
manager->initialize(config);
```

## Usage Examples

### Encrypting Data
```cpp
#include <satox/security/quantum_encryption.hpp>

auto quantum_enc = std::make_unique<satox::QuantumEncryption>();
auto key_pair = quantum_enc->generateKeyPair("kyber-1024");

std::string data = "Sensitive blockchain data";
auto encrypted = quantum_enc->encrypt(data, key_pair.public_key);
auto decrypted = quantum_enc->decrypt(encrypted, key_pair.private_key);
```

### Digital Signatures
```cpp
#include <satox/security/quantum_signature.hpp>

auto quantum_sig = std::make_unique<satox::QuantumSignature>();
auto sig_key_pair = quantum_sig->generateKeyPair("dilithium5");

std::string transaction = "transaction_hash";
auto signature = quantum_sig->sign(transaction, sig_key_pair.private_key);
bool valid = quantum_sig->verify(transaction, signature, sig_key_pair.public_key);
```

## Key Management

```cpp
#include <satox/security/key_manager.hpp>

auto key_manager = std::make_unique<satox::QuantumKeyManager>();

// Secure storage
key_manager->storeKey("user_key", private_key, "password123");
auto key = key_manager->getKey("user_key", "password123");

// Auto rotation
key_manager->enableAutoRotation(30); // 30 days
```

## Performance

| Algorithm | Key Size | Sign Time | Security |
|-----------|----------|-----------|----------|
| Kyber-512 | 800 bytes | 0.1ms | 128-bit |
| Kyber-1024 | 1568 bytes | 0.2ms | 256-bit |
| Dilithium5 | 1952 bytes | 0.5ms | 256-bit |

## Best Practices

1. **Use hybrid mode** during transition
2. **Choose appropriate algorithms** for your use case
3. **Implement key rotation**
4. **Secure key storage**
5. **Monitor for updates**

## Troubleshooting

**Issue**: "Quantum security not available"
**Solution**: Update SDK, check dependencies

**Issue**: "Performance slow"
**Solution**: Use appropriate algorithm, enable caching

**Issue**: "Key generation fails"
**Solution**: Check system entropy

## References

- [NIST Post-Quantum Cryptography](https://www.nist.gov/programs-projects/post-quantum-cryptography)
- [Kyber Specification](https://pq-crystals.org/kyber/)
- [Dilithium Specification](https://pq-crystals.org/dilithium/)

---

*Last updated: $(date)*
