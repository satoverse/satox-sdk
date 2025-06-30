"""
$(basename "$1")
$(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')

Copyright (c) 2025 Satoxcoin Core Developers
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

"""
Example usage of quantum-resistant cryptography in Satox SDK.
"""
from satox.security.quantum_crypto import QuantumCrypto
from satox.security.security_manager import SecurityManager

def main():
    # Initialize security components
    crypto = QuantumCrypto()
    security_manager = SecurityManager()
    
    # Example 1: Key Generation and Storage
    print("Generating quantum-resistant key pair...")
    private_key, public_key = crypto.generate_keypair()
    print(f"Generated public key: {public_key.hex()[:32]}...")
    
    # Example 2: Wallet Encryption
    print("\nEncrypting wallet data...")
    wallet_data = b"Sensitive wallet information"
    password = "strong_password_123"
    
    encrypted_wallet = crypto.encrypt_wallet(wallet_data, password)
    print("Wallet encrypted successfully")
    
    # Example 3: Transaction Signing
    print("\nSigning a transaction...")
    transaction = b"Transfer 100 SATOX to address 0x123..."
    signature = crypto.sign_transaction(transaction, private_key)
    print(f"Transaction signed: {signature.hex()[:32]}...")
    
    # Verify signature
    is_valid = crypto.verify_signature(transaction, signature, public_key)
    print(f"Signature verification: {'Valid' if is_valid else 'Invalid'}")
    
    # Example 4: Secure Message Exchange
    print("\nEncrypting a message...")
    message = b"Confidential message"
    encrypted_message = crypto.encrypt_message(message, public_key)
    print("Message encrypted successfully")
    
    # Decrypt message
    decrypted_message = crypto.decrypt_message(encrypted_message, private_key)
    print(f"Decrypted message: {decrypted_message.decode()}")
    
    # Example 5: Security Configuration
    print("\nSecurity Configuration:")
    config = security_manager.get_encryption_config()
    print(f"Encryption Algorithm: {config['algorithm']}")
    print(f"Signature Algorithm: {config['signature']}")
    
    # Example 6: Security Policy
    policy = security_manager.get_network_security_policy()
    print("\nNetwork Security Policy:")
    print(f"Require Encryption: {policy['require_encryption']}")
    print(f"Minimum Key Length: {policy['min_key_length']} bits")
    print(f"Allowed Algorithms: {', '.join(policy['allowed_algorithms'])}")

if __name__ == "__main__":
    main() 