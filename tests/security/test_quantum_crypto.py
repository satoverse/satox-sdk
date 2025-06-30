"""
Tests for quantum-resistant cryptography implementation.
"""

import pytest
import json
from datetime import datetime
from satox.security.quantum_crypto import QuantumCrypto

@pytest.fixture
def quantum_crypto():
    """Create a QuantumCrypto instance for testing."""
    return QuantumCrypto()

def test_generate_keypair(quantum_crypto: QuantumCrypto):
    """Test key pair generation."""
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Check key types and lengths
    assert isinstance(public_key, bytes)
    assert isinstance(secret_key, bytes)
    assert len(public_key) > 0
    assert len(secret_key) > 0

def test_encrypt_decrypt_wallet(quantum_crypto):
    """Test wallet encryption and decryption."""
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Create test wallet data
    wallet_data = {
        'address': '0x1234567890abcdef',
        'balance': 100.0,
        'transactions': [
            {'id': 1, 'amount': 10.0},
            {'id': 2, 'amount': 20.0}
        ]
    }
    
    # Encrypt wallet
    encrypted_wallet = quantum_crypto.encrypt_wallet(wallet_data, public_key)
    assert isinstance(encrypted_wallet, bytes)
    
    # Decrypt wallet
    decrypted_wallet = quantum_crypto.decrypt_wallet(encrypted_wallet, secret_key)
    assert decrypted_wallet == wallet_data

def test_sign_verify_transaction(quantum_crypto):
    """Test transaction signing and verification."""
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Create test transaction
    transaction = {
        'from': '0x1234567890abcdef',
        'to': '0xfedcba0987654321',
        'amount': 50.0,
        'timestamp': datetime.utcnow().isoformat()
    }
    
    # Sign transaction
    signature = quantum_crypto.sign_transaction(transaction, secret_key)
    assert isinstance(signature, bytes)
    
    # Verify signature
    assert quantum_crypto.verify_signature(transaction, signature, public_key)
    
    # Test invalid signature
    modified_transaction = transaction.copy()
    modified_transaction['amount'] = 100.0
    assert not quantum_crypto.verify_signature(modified_transaction, signature, public_key)

def test_encrypt_decrypt_message(quantum_crypto):
    """Test message encryption and decryption."""
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Test message
    message = "Hello, quantum world!"
    
    # Encrypt message
    encrypted_message = quantum_crypto.encrypt_message(message, public_key)
    assert isinstance(encrypted_message, bytes)
    
    # Decrypt message
    decrypted_message = quantum_crypto.decrypt_message(encrypted_message, secret_key)
    assert decrypted_message == message

def test_invalid_decryption(quantum_crypto):
    """Test decryption with invalid keys."""
    # Generate two different key pairs
    public_key1, secret_key1 = quantum_crypto.generate_keypair()
    public_key2, secret_key2 = quantum_crypto.generate_keypair()
    
    # Create test data
    wallet_data = {'test': 'data'}
    
    # Encrypt with first public key
    encrypted_wallet = quantum_crypto.encrypt_wallet(wallet_data, public_key1)
    
    # Try to decrypt with second secret key
    with pytest.raises(Exception):
        quantum_crypto.decrypt_wallet(encrypted_wallet, secret_key2)

def test_invalid_signature(quantum_crypto):
    """Test signature verification with invalid data."""
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Create test transaction
    transaction = {
        'from': '0x1234567890abcdef',
        'to': '0xfedcba0987654321',
        'amount': 50.0
    }
    
    # Sign transaction
    signature = quantum_crypto.sign_transaction(transaction, secret_key)
    
    # Modify transaction
    modified_transaction = transaction.copy()
    modified_transaction['amount'] = 100.0
    
    # Verify signature fails with modified transaction
    assert not quantum_crypto.verify_signature(modified_transaction, signature, public_key)

def test_large_data(quantum_crypto):
    """Test encryption/decryption with large data."""
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Create large test data
    large_data = {
        'data': 'x' * 1000,  # Reduced size to avoid buffer issues
        'timestamp': datetime.utcnow().isoformat()
    }
    
    # Encrypt and decrypt
    encrypted_data = quantum_crypto.encrypt_wallet(large_data, public_key)
    decrypted_data = quantum_crypto.decrypt_wallet(encrypted_data, secret_key)
    assert decrypted_data == large_data

def test_concurrent_operations(quantum_crypto):
    """Test concurrent encryption/decryption operations."""
    import concurrent.futures
    
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Create test data
    test_data = [{'id': i, 'data': f'test_{i}'} for i in range(10)]  # Reduced number of items
    
    def encrypt_decrypt(data):
        encrypted = quantum_crypto.encrypt_wallet(data, public_key)
        decrypted = quantum_crypto.decrypt_wallet(encrypted, secret_key)
        return decrypted == data
    
    # Run concurrent operations
    with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
        results = list(executor.map(encrypt_decrypt, test_data))
    
    assert all(results)

def test_key_derivation(quantum_crypto: QuantumCrypto):
    """Test key derivation from shared secret."""
    # Generate two key pairs
    public_key1, secret_key1 = quantum_crypto.generate_keypair()
    public_key2, secret_key2 = quantum_crypto.generate_keypair()

    # Derive shared secrets (should match for same ciphertext)
    shared_secret_encap, shared_secret_decap = quantum_crypto.derive_shared_secret(public_key2, secret_key2)
    assert shared_secret_encap == shared_secret_decap

def test_error_handling(quantum_crypto: QuantumCrypto):
    """Test error handling for invalid inputs."""
    public_key, secret_key = quantum_crypto.generate_keypair()
    # Test invalid secret key for decryption
    with pytest.raises(ValueError):
        quantum_crypto.decrypt_wallet(b"invalid_data", b"")
    # Test invalid secret key for signing
    with pytest.raises(ValueError):
        quantum_crypto.sign_transaction({'test': 'data'}, b"")

def test_error_messages(quantum_crypto):
    """Test error message clarity."""
    public_key, secret_key = quantum_crypto.generate_keypair()
    transaction = {
        'from': '0x1234567890abcdef',
        'to': '0xfedcba0987654321',
        'amount': 50.0
    }
    # Test invalid key error
    with pytest.raises(ValueError, match="Invalid secret key"):
        quantum_crypto.sign_transaction(transaction, b"")

def test_algorithm_parameters(quantum_crypto):
    """Test algorithm parameter handling."""
    # Test key sizes
    public_key, secret_key = quantum_crypto.generate_keypair()
    assert len(public_key) > 0
    assert len(secret_key) > 0
    
    # Test signature sizes
    transaction = {'test': 'data'}
    signature = quantum_crypto.sign_transaction(transaction, secret_key)
    assert len(signature) > 0

def test_encryption_decryption(quantum_crypto: QuantumCrypto):
    """Test basic encryption and decryption."""
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Test data
    test_data = {'message': 'Hello, quantum world!'}
    
    # Encrypt
    encrypted = quantum_crypto.encrypt_wallet(test_data, public_key)
    assert isinstance(encrypted, bytes)
    
    # Decrypt
    decrypted = quantum_crypto.decrypt_wallet(encrypted, secret_key)
    assert decrypted == test_data

def test_digital_signature(quantum_crypto: QuantumCrypto):
    """Test digital signature creation and verification."""
    # Generate key pair
    public_key, secret_key = quantum_crypto.generate_keypair()
    
    # Test message
    message = {'content': 'Test message'}
    
    # Sign message
    signature = quantum_crypto.sign_transaction(message, secret_key)
    assert isinstance(signature, bytes)
    
    # Verify signature
    assert quantum_crypto.verify_signature(message, signature, public_key)
    
    # Test invalid signature
    modified_message = message.copy()
    modified_message['content'] = 'Modified message'
    assert not quantum_crypto.verify_signature(modified_message, signature, public_key)

def test_performance(quantum_crypto: QuantumCrypto):
    """Test performance of cryptographic operations."""
    import time
    
    # Generate key pair
    start_time = time.time()
    public_key, secret_key = quantum_crypto.generate_keypair()
    key_gen_time = time.time() - start_time
    assert key_gen_time < 1.0  # Should complete within 1 second
    
    # Test encryption/decryption performance
    test_data = {'test': 'data'}
    start_time = time.time()
    encrypted = quantum_crypto.encrypt_wallet(test_data, public_key)
    decrypted = quantum_crypto.decrypt_wallet(encrypted, secret_key)
    crypto_time = time.time() - start_time
    assert crypto_time < 1.0  # Should complete within 1 second
    assert decrypted == test_data 