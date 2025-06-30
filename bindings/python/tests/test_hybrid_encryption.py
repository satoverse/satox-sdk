"""
Unit tests for HybridEncryption Python bindings
"""

import pytest
from satox.quantum import HybridEncryption
from satox.types import SatoxError

@pytest.fixture
def encryption():
    """Create a HybridEncryption instance for testing."""
    enc = HybridEncryption()
    assert enc.initialize()
    yield enc
    enc.shutdown()

def test_initialization():
    """Test HybridEncryption initialization."""
    enc = HybridEncryption()
    assert not enc._initialized
    assert enc.initialize()
    assert enc._initialized
    assert enc.shutdown()
    assert not enc._initialized

def test_generate_key_pair(encryption):
    """Test key pair generation functionality."""
    # Test key pair generation
    public_key, private_key = encryption.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    assert len(public_key) > 0
    assert len(private_key) > 0

def test_encrypt_decrypt(encryption):
    """Test encryption and decryption functionality."""
    # Generate key pair
    public_key, private_key = encryption.generate_key_pair()
    
    # Test data
    test_data = b"Test data for hybrid encryption"
    
    # Encrypt data
    encrypted_data = encryption.encrypt(public_key, test_data)
    assert encrypted_data is not None
    assert encrypted_data != test_data
    
    # Decrypt data
    decrypted_data = encryption.decrypt(private_key, encrypted_data)
    assert decrypted_data == test_data

def test_uninitialized_operations():
    """Test operations when not initialized."""
    enc = HybridEncryption()
    with pytest.raises(SatoxError) as exc_info:
        enc.generate_key_pair()
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        enc.encrypt(b"key", b"data")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        enc.decrypt(b"key", b"data")
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(encryption):
    """Test invalid input handling."""
    # Test with None inputs
    with pytest.raises(SatoxError) as exc_info:
        encryption.encrypt(None, b"data")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        encryption.decrypt(None, b"data")
    assert exc_info.value.code == "INVALID_INPUT"

    # Test with empty inputs
    with pytest.raises(SatoxError) as exc_info:
        encryption.encrypt(b"", b"data")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        encryption.decrypt(b"", b"data")
    assert exc_info.value.code == "INVALID_INPUT"

def test_double_initialization():
    """Test double initialization handling."""
    enc = HybridEncryption()
    assert enc.initialize()
    assert enc.initialize()  # Should not raise an error
    assert enc.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    enc = HybridEncryption()
    assert enc.initialize()
    assert enc.shutdown()
    assert enc.shutdown()  # Should not raise an error

def test_session_key_management(encryption):
    """Test session key management."""
    # Get initial session key
    initial_key = encryption.get_session_key()
    assert initial_key
    assert len(initial_key) > 0

    # Rotate session key
    assert encryption.rotate_session_key()
    new_key = encryption.get_session_key()
    assert new_key
    assert new_key != initial_key

def test_get_version(encryption):
    """Test version retrieval."""
    version = encryption.get_version()
    assert version
    assert isinstance(version, str)

def test_encryption_consistency(encryption):
    """Test encryption consistency with same keys."""
    public_key, private_key = encryption.generate_key_pair()
    data = b"Test data for consistency check"
    
    # Encrypt same data twice
    encrypted1 = encryption.encrypt(public_key, data)
    encrypted2 = encryption.encrypt(public_key, data)
    
    # Encrypted data should be different due to hybrid encryption
    assert encrypted1 != encrypted2
    
    # Both should decrypt to the same data
    decrypted1 = encryption.decrypt(private_key, encrypted1)
    decrypted2 = encryption.decrypt(private_key, encrypted2)
    assert decrypted1 == data
    assert decrypted2 == data

def test_key_pair_uniqueness(encryption):
    """Test that generated key pairs are unique."""
    key_pairs = set()
    for _ in range(10):
        public_key, private_key = encryption.generate_key_pair()
        key_pairs.add((public_key, private_key))
    assert len(key_pairs) == 10  # All key pairs should be unique 