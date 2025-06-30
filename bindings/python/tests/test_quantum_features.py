"""
Integration tests for quantum feature Python bindings.
"""

import pytest
import os
import time
from typing import Tuple, Dict, Any

from satox.quantum import (
    QuantumManager,
    HybridEncryption,
    KeyStorage,
    PostQuantumAlgorithms
)

@pytest.fixture
def quantum_manager():
    """Fixture for QuantumManager instance."""
    manager = QuantumManager()
    assert manager.initialize()
    yield manager
    manager.shutdown()

@pytest.fixture
def hybrid_encryption():
    """Fixture for HybridEncryption instance."""
    hybrid = HybridEncryption()
    assert hybrid.initialize()
    yield hybrid
    hybrid.shutdown()

@pytest.fixture
def key_storage():
    """Fixture for KeyStorage instance."""
    storage = KeyStorage()
    assert storage.initialize()
    yield storage
    storage.shutdown()

@pytest.fixture
def post_quantum_algorithms():
    """Fixture for PostQuantumAlgorithms instance."""
    algorithms = PostQuantumAlgorithms()
    assert algorithms.initialize()
    yield algorithms
    algorithms.shutdown()

def test_quantum_manager_basic_operations(quantum_manager):
    """Test basic operations of QuantumManager."""
    # Test key pair generation
    public_key, private_key = quantum_manager.generate_key_pair()
    assert public_key and private_key
    assert len(public_key) > 0
    assert len(private_key) > 0
    
    # Test encryption and decryption
    message = b"Hello, Quantum World!"
    ciphertext = quantum_manager.encrypt(message, public_key)
    assert ciphertext
    decrypted = quantum_manager.decrypt(ciphertext, private_key)
    assert decrypted == message
    
    # Test signing and verification
    signature = quantum_manager.sign(message, private_key)
    assert signature
    assert quantum_manager.verify(message, signature, public_key)
    
    # Test version and algorithm info
    version = quantum_manager.get_version()
    assert version
    algorithm = quantum_manager.get_algorithm()
    assert algorithm

def test_hybrid_encryption_operations(hybrid_encryption):
    """Test operations of HybridEncryption."""
    # Test key pair generation
    public_key, private_key = hybrid_encryption.generate_key_pair()
    assert public_key and private_key
    
    # Test encryption and decryption
    message = b"Hello, Hybrid World!"
    ciphertext = hybrid_encryption.encrypt(message, public_key)
    assert ciphertext
    decrypted = hybrid_encryption.decrypt(ciphertext, private_key)
    assert decrypted == message
    
    # Test session key management
    session_key = hybrid_encryption.get_session_key()
    assert session_key
    assert hybrid_encryption.rotate_session_key()
    new_session_key = hybrid_encryption.get_session_key()
    assert new_session_key != session_key

def test_key_storage_operations(key_storage):
    """Test operations of KeyStorage."""
    # Test key storage and retrieval
    key_id = "test_key_1"
    key = b"test_key_data"
    metadata = {"purpose": "testing", "created": int(time.time())}
    
    assert key_storage.store_key(key_id, key, metadata)
    retrieved_key = key_storage.retrieve_key(key_id)
    assert retrieved_key == key
    
    # Test metadata operations
    retrieved_metadata = key_storage.get_key_metadata(key_id)
    assert retrieved_metadata == metadata
    
    # Test key update
    new_key = b"new_key_data"
    new_metadata = {"purpose": "testing", "updated": int(time.time())}
    assert key_storage.update_key(key_id, new_key, new_metadata)
    assert key_storage.retrieve_key(key_id) == new_key
    
    # Test key rotation
    rotated_key = b"rotated_key_data"
    assert key_storage.rotate_key(key_id, rotated_key)
    assert key_storage.retrieve_key(key_id) == rotated_key
    
    # Test key validation
    assert key_storage.validate_key(key_id, rotated_key)
    assert not key_storage.validate_key(key_id, b"wrong_key")
    
    # Test key expiration
    expiration_time = int(time.time()) + 3600  # 1 hour from now
    assert key_storage.set_key_expiration(key_id, expiration_time)
    assert not key_storage.is_key_expired(key_id)
    
    # Test key access control
    user_id = "test_user"
    access_level = 1
    assert key_storage.set_key_access(key_id, user_id, access_level)
    assert key_storage.check_key_access(key_id, user_id, access_level)
    assert not key_storage.check_key_access(key_id, user_id, access_level + 1)
    
    # Test key deletion
    assert key_storage.delete_key(key_id)
    assert key_storage.retrieve_key(key_id) is None

def test_post_quantum_algorithms_operations(post_quantum_algorithms):
    """Test operations of PostQuantumAlgorithms."""
    # Test CRYSTALS-Kyber operations
    kyber_public, kyber_private = post_quantum_algorithms.generate_kyber_key_pair()
    assert kyber_public and kyber_private
    
    message = b"Hello, Kyber!"
    kyber_ciphertext = post_quantum_algorithms.kyber_encrypt(message, kyber_public)
    assert kyber_ciphertext
    kyber_decrypted = post_quantum_algorithms.kyber_decrypt(kyber_ciphertext, kyber_private)
    assert kyber_decrypted == message
    
    # Test NTRU operations
    ntru_public, ntru_private = post_quantum_algorithms.generate_ntru_key_pair()
    assert ntru_public and ntru_private
    
    ntru_ciphertext = post_quantum_algorithms.ntru_encrypt(message, ntru_public)
    assert ntru_ciphertext
    ntru_decrypted = post_quantum_algorithms.ntru_decrypt(ntru_ciphertext, ntru_private)
    assert ntru_decrypted == message
    
    # Test algorithm information
    version = post_quantum_algorithms.get_version()
    assert version
    algorithm = post_quantum_algorithms.get_algorithm()
    assert algorithm
    available_algorithms = post_quantum_algorithms.get_available_algorithms()
    assert available_algorithms
    assert "CRYSTALS-Kyber" in available_algorithms
    assert "NTRU" in available_algorithms

def test_error_handling(quantum_manager, hybrid_encryption, key_storage, post_quantum_algorithms):
    """Test error handling in all components."""
    # Test uninitialized operations
    uninit_manager = QuantumManager()
    with pytest.raises(RuntimeError):
        uninit_manager.generate_key_pair()
    
    # Test invalid inputs
    with pytest.raises(ValueError):
        quantum_manager.encrypt(b"", b"")
    
    with pytest.raises(ValueError):
        key_storage.store_key("", b"")
    
    with pytest.raises(ValueError):
        post_quantum_algorithms.kyber_encrypt(b"", b"")
    
    # Test invalid key operations
    with pytest.raises(RuntimeError):
        quantum_manager.decrypt(b"invalid", b"invalid")
    
    with pytest.raises(RuntimeError):
        hybrid_encryption.decrypt(b"invalid", b"invalid")
    
    with pytest.raises(RuntimeError):
        post_quantum_algorithms.ntru_decrypt(b"invalid", b"invalid")

def test_concurrent_operations(quantum_manager, key_storage):
    """Test concurrent operations."""
    import threading
    
    def worker():
        public_key, private_key = quantum_manager.generate_key_pair()
        message = b"Concurrent test message"
        ciphertext = quantum_manager.encrypt(message, public_key)
        decrypted = quantum_manager.decrypt(ciphertext, private_key)
        assert decrypted == message
    
    threads = [threading.Thread(target=worker) for _ in range(10)]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()

def test_large_data_operations(quantum_manager, hybrid_encryption):
    """Test operations with large data."""
    # Generate 1MB of random data
    large_data = os.urandom(1024 * 1024)
    
    # Test QuantumManager
    public_key, private_key = quantum_manager.generate_key_pair()
    ciphertext = quantum_manager.encrypt(large_data, public_key)
    decrypted = quantum_manager.decrypt(ciphertext, private_key)
    assert decrypted == large_data
    
    # Test HybridEncryption
    hybrid_public, hybrid_private = hybrid_encryption.generate_key_pair()
    hybrid_ciphertext = hybrid_encryption.encrypt(large_data, hybrid_public)
    hybrid_decrypted = hybrid_encryption.decrypt(hybrid_ciphertext, hybrid_private)
    assert hybrid_decrypted == large_data

def test_key_rotation_and_reencryption(key_storage):
    """Test key rotation and reencryption features."""
    # Store initial key
    key_id = "rotation_test_key"
    initial_key = b"initial_key_data"
    assert key_storage.store_key(key_id, initial_key)
    
    # Test key rotation
    new_key = b"new_key_data"
    assert key_storage.rotate_key(key_id, new_key)
    assert key_storage.retrieve_key(key_id) == new_key
    
    # Test key reencryption
    assert key_storage.reencrypt_key(key_id)
    retrieved_key = key_storage.retrieve_key(key_id)
    assert retrieved_key == new_key  # Content should remain the same
    
    # Verify key is still valid after reencryption
    assert key_storage.validate_key(key_id, new_key)

def test_metadata_operations(key_storage):
    """Test metadata operations."""
    key_id = "metadata_test_key"
    key = b"test_key_data"
    initial_metadata = {
        "purpose": "testing",
        "created": int(time.time()),
        "tags": ["test", "metadata"],
        "permissions": {"read": True, "write": False}
    }
    
    # Store key with metadata
    assert key_storage.store_key(key_id, key, initial_metadata)
    
    # Retrieve and verify metadata
    retrieved_metadata = key_storage.get_key_metadata(key_id)
    assert retrieved_metadata == initial_metadata
    
    # Update metadata
    updated_metadata = {
        "purpose": "testing",
        "updated": int(time.time()),
        "tags": ["test", "metadata", "updated"],
        "permissions": {"read": True, "write": True}
    }
    assert key_storage.set_key_metadata(key_id, updated_metadata)
    
    # Verify updated metadata
    retrieved_metadata = key_storage.get_key_metadata(key_id)
    assert retrieved_metadata == updated_metadata 