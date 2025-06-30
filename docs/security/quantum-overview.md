# Quantum Security Overview for Satox Core SDK Users

## What is Quantum Security?
Quantum security refers to cryptographic techniques designed to protect data against attacks from quantum computers. Quantum computers can break many traditional encryption methods, so new algorithms—called post-quantum cryptography (PQC)—are needed to keep data safe in the future.

## Why Does Quantum Security Matter?
- **Quantum computers** can solve certain mathematical problems much faster than classical computers.
- Many widely used encryption methods (like RSA and ECC) can be broken by quantum computers using Shor's algorithm.
- To ensure your data and transactions remain secure for years to come, it's important to use quantum-resistant cryptography now.

## What are Quantum Attacks?
A quantum attack is when an attacker uses a quantum computer to break traditional cryptographic protections. This could allow them to:
- Decrypt sensitive data
- Forge digital signatures
- Impersonate users or services

## How Does Satox Core SDK Protect You?
Satox Core SDK integrates post-quantum cryptography (PQC) algorithms, including:
- **Kyber** (for key exchange)
- **Dilithium** and **SPHINCS+** (for digital signatures)
- **Hybrid mode** (combines classical and quantum-safe algorithms for extra security)

### Key Features
- **Automatic key management**: Quantum-safe keys are generated and rotated automatically.
- **Hybrid cryptography**: Both classical and PQC algorithms are used together for a smooth migration.
- **NIST PQC compliance**: Only algorithms recommended by global standards bodies are used.

## How to Enable Quantum Security
1. Set `quantum_security.enabled=true` in your SDK configuration file.
2. Choose your preferred algorithm (e.g., `kyber-1024`, `dilithium5`).
3. Use the provided API functions for encryption, signing, and verification.

### Example
```cpp
// Encrypt data
std::string encrypted = quantum_encrypt(data, "kyber-1024");
// Sign data
std::string signature = quantum_sign(data, private_key, "dilithium5");
// Verify signature
bool valid = quantum_verify(data, signature, public_key, "dilithium5");
```

## Frequently Asked Questions (FAQ)
**Q: Do I need to understand quantum physics to use this?**
A: No! The SDK handles all the complexity for you.

**Q: Is quantum security slower?**
A: PQC algorithms are designed to be efficient. Some operations may be slightly slower, but the SDK optimizes performance.

**Q: Can I use both classical and quantum-safe algorithms?**
A: Yes! Hybrid mode is supported for maximum compatibility and security.

**Q: What if quantum computers aren't available yet?**
A: Data encrypted today may be stored and attacked in the future. Using quantum-safe cryptography now protects you long-term.

## More Resources
- [NIST Post-Quantum Cryptography Project](https://csrc.nist.gov/projects/post-quantum-cryptography)
- [Satox SDK Documentation](./QUANTUM_SECURITY.md) 