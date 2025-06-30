"""
Unit tests for QuantumManager Python bindings
"""

import pytest
from satox.quantum import QuantumManager
from satox.types import SatoxError

@pytest.fixture
def manager():
    """Create a QuantumManager instance for testing."""
    mgr = QuantumManager()
    assert mgr.initialize()
    yield mgr
    mgr.shutdown()

def test_initialization():
    """Test QuantumManager initialization."""
    mgr = QuantumManager()
    assert not mgr._initialized
    assert mgr.initialize()
    assert mgr._initialized
    assert mgr.shutdown()
    assert not mgr._initialized

def test_generate_key_pair(manager):
    """Test key pair generation functionality."""
    # Test key pair generation
    public_key, private_key = manager.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    assert len(public_key) > 0
    assert len(private_key) > 0

def test_encrypt_decrypt(manager):
    """Test encryption and decryption."""
    public_key, private_key = manager.generate_key_pair()
    data = b"Test data for encryption"
    encrypted = manager.encrypt(public_key, data)
    assert encrypted
    assert encrypted != data
    decrypted = manager.decrypt(private_key, encrypted)
    assert decrypted == data

def test_sign_verify(manager):
    """Test signing and verification."""
    public_key, private_key = manager.generate_key_pair()
    data = b"Test data for signing"
    signature = manager.sign(private_key, data)
    assert signature
    assert manager.verify(public_key, data, signature)
    assert not manager.verify(public_key, data, b"invalid signature")

def test_random_number_generation(manager):
    """Test random number generation."""
    min_val = 0
    max_val = 100
    for _ in range(100):
        num = manager.generate_random_number(min_val, max_val)
        assert min_val <= num <= max_val

def test_random_bytes_generation(manager):
    """Test random bytes generation."""
    length = 32
    bytes1 = manager.generate_random_bytes(length)
    bytes2 = manager.generate_random_bytes(length)
    assert len(bytes1) == length
    assert len(bytes2) == length
    assert bytes1 != bytes2  # Very unlikely to be equal

def test_get_version(manager):
    """Test version retrieval."""
    version = manager.get_version()
    assert version
    assert isinstance(version, str)

def test_get_algorithm(manager):
    """Test algorithm retrieval functionality."""
    algorithm = manager.get_algorithm()
    assert algorithm is not None
    assert isinstance(algorithm, str)
    assert len(algorithm) > 0

def test_get_available_algorithms(manager):
    """Test available algorithms retrieval."""
    algorithms = manager.get_available_algorithms()
    assert algorithms is not None
    assert isinstance(algorithms, list)
    assert len(algorithms) > 0
    assert all(isinstance(alg, str) for alg in algorithms)

def test_uninitialized_operations():
    """Test operations when not initialized."""
    mgr = QuantumManager()
    with pytest.raises(SatoxError) as exc_info:
        mgr.generate_key_pair()
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        mgr.get_algorithm()
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        mgr.get_available_algorithms()
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_double_initialization():
    """Test double initialization handling."""
    mgr = QuantumManager()
    assert mgr.initialize()
    assert mgr.initialize()  # Should not raise an error
    assert mgr.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    mgr = QuantumManager()
    assert mgr.initialize()
    assert mgr.shutdown()
    assert mgr.shutdown()  # Should not raise an error

def test_invalid_inputs(manager):
    """Test invalid input handling."""
    with pytest.raises(SatoxError) as exc_info:
        manager.encrypt(None, b"data")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        manager.decrypt(None, b"data")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        manager.sign(None, b"data")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        manager.verify(None, b"data", b"sig")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        manager.generate_random_number(10, 5)  # min > max
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        manager.generate_random_bytes(0)  # invalid length
    assert exc_info.value.code == "INVALID_INPUT" 