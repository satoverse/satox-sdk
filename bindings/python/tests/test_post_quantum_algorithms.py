"""
Unit tests for PostQuantumAlgorithms
"""

import pytest
from satox.quantum.post_quantum_algorithms import PostQuantumAlgorithms, AlgorithmInfo
from satox.types import SatoxError

@pytest.fixture
def algorithms():
    """Create a PostQuantumAlgorithms instance for testing."""
    algo = PostQuantumAlgorithms()
    algo.initialize()
    yield algo
    algo.shutdown()

def test_initialization():
    """Test initialization and shutdown."""
    algo = PostQuantumAlgorithms()
    assert not algo.is_initialized()
    assert algo.initialize()
    assert algo.is_initialized()
    assert algo.shutdown()
    assert not algo.is_initialized()

def test_generate_key_pair(algorithms):
    """Test key pair generation."""
    algorithm_name = algorithms.get_default_algorithm()
    public_key, private_key = algorithms.generate_key_pair(algorithm_name)
    assert public_key
    assert private_key
    assert len(public_key) > 0
    assert len(private_key) > 0

def test_sign_verify(algorithms):
    """Test signing and verification."""
    algorithm_name = algorithms.get_default_algorithm()
    public_key, private_key = algorithms.generate_key_pair(algorithm_name)
    data = b"Test data for signing"
    
    # Sign data
    signature = algorithms.sign(algorithm_name, private_key, data)
    assert signature
    assert len(signature) > 0
    
    # Verify signature
    assert algorithms.verify(algorithm_name, public_key, data, signature)
    assert not algorithms.verify(algorithm_name, public_key, data, b"invalid signature")

def test_encrypt_decrypt(algorithms):
    """Test encryption and decryption."""
    algorithm_name = algorithms.get_default_algorithm()
    public_key, private_key = algorithms.generate_key_pair(algorithm_name)
    data = b"Test data for encryption"
    
    # Encrypt data
    encrypted = algorithms.encrypt(algorithm_name, public_key, data)
    assert encrypted
    assert encrypted != data
    
    # Decrypt data
    decrypted = algorithms.decrypt(algorithm_name, private_key, encrypted)
    assert decrypted == data

def test_get_algorithm_info(algorithms):
    """Test algorithm information retrieval."""
    algorithm_name = algorithms.get_default_algorithm()
    info = algorithms.get_algorithm_info(algorithm_name)
    assert isinstance(info, AlgorithmInfo)
    assert info.name == algorithm_name
    assert info.security_level > 0
    assert info.key_size > 0
    assert info.signature_size > 0
    assert isinstance(info.is_recommended, bool)
    assert info.description

def test_get_available_algorithms(algorithms):
    """Test available algorithms retrieval."""
    available = algorithms.get_available_algorithms()
    assert available
    assert isinstance(available, list)
    assert len(available) > 0
    assert all(isinstance(name, str) for name in available)

def test_get_recommended_algorithms(algorithms):
    """Test recommended algorithms retrieval."""
    recommended = algorithms.get_recommended_algorithms()
    assert recommended
    assert isinstance(recommended, list)
    assert len(recommended) > 0
    assert all(isinstance(name, str) for name in recommended)

def test_algorithm_availability(algorithms):
    """Test algorithm availability checks."""
    algorithm_name = algorithms.get_default_algorithm()
    assert algorithms.is_algorithm_available(algorithm_name)
    assert not algorithms.is_algorithm_available("nonexistent_algorithm")

def test_algorithm_recommendation(algorithms):
    """Test algorithm recommendation checks."""
    recommended = algorithms.get_recommended_algorithms()
    if recommended:
        assert algorithms.is_algorithm_recommended(recommended[0])
    assert not algorithms.is_algorithm_recommended("nonexistent_algorithm")

def test_algorithm_security_level(algorithms):
    """Test security level retrieval."""
    algorithm_name = algorithms.get_default_algorithm()
    security_level = algorithms.get_algorithm_security_level(algorithm_name)
    assert security_level > 0

def test_algorithm_key_size(algorithms):
    """Test key size retrieval."""
    algorithm_name = algorithms.get_default_algorithm()
    key_size = algorithms.get_algorithm_key_size(algorithm_name)
    assert key_size > 0

def test_algorithm_signature_size(algorithms):
    """Test signature size retrieval."""
    algorithm_name = algorithms.get_default_algorithm()
    signature_size = algorithms.get_algorithm_signature_size(algorithm_name)
    assert signature_size > 0

def test_default_algorithm(algorithms):
    """Test default algorithm management."""
    # Get current default
    current_default = algorithms.get_default_algorithm()
    assert current_default
    
    # Get available algorithms
    available = algorithms.get_available_algorithms()
    if len(available) > 1:
        # Set a different default
        new_default = next(algo for algo in available if algo != current_default)
        assert algorithms.set_default_algorithm(new_default)
        assert algorithms.get_default_algorithm() == new_default

def test_uninitialized_operations():
    """Test operations when not initialized."""
    algo = PostQuantumAlgorithms()
    with pytest.raises(SatoxError) as exc_info:
        algo.generate_key_pair("test")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        algo.sign("test", b"key", b"data")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        algo.verify("test", b"key", b"data", b"sig")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        algo.encrypt("test", b"key", b"data")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        algo.decrypt("test", b"key", b"data")
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(algorithms):
    """Test invalid input handling."""
    with pytest.raises(SatoxError) as exc_info:
        algorithms.generate_key_pair(None)
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        algorithms.sign(None, b"key", b"data")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        algorithms.verify(None, b"key", b"data", b"sig")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        algorithms.encrypt(None, b"key", b"data")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        algorithms.decrypt(None, b"key", b"data")
    assert exc_info.value.code == "INVALID_INPUT"

def test_algorithm_consistency(algorithms):
    """Test algorithm consistency across operations."""
    algorithm_name = algorithms.get_default_algorithm()
    public_key, private_key = algorithms.generate_key_pair(algorithm_name)
    data = b"Test data for consistency"
    
    # Test encryption/decryption consistency
    encrypted = algorithms.encrypt(algorithm_name, public_key, data)
    decrypted = algorithms.decrypt(algorithm_name, private_key, encrypted)
    assert decrypted == data
    
    # Test signing/verification consistency
    signature = algorithms.sign(algorithm_name, private_key, data)
    assert algorithms.verify(algorithm_name, public_key, data, signature)
    
    # Test key size consistency
    key_size = algorithms.get_algorithm_key_size(algorithm_name)
    assert len(public_key) * 8 >= key_size
    assert len(private_key) * 8 >= key_size
    
    # Test signature size consistency
    signature_size = algorithms.get_algorithm_signature_size(algorithm_name)
    assert len(signature) * 8 >= signature_size 