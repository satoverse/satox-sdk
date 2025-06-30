"""
Unit tests for KeyStorage Python bindings
"""

import pytest
from satox.quantum import KeyStorage, KeyMetadata
from satox.types import SatoxError

@pytest.fixture
def storage():
    """Create a KeyStorage instance for testing."""
    store = KeyStorage()
    assert store.initialize()
    yield store
    store.shutdown()

@pytest.fixture
def test_key():
    """Create a test key for storage."""
    return b"test_key_data"

@pytest.fixture
def test_metadata():
    """Create test metadata for a key."""
    return KeyMetadata(
        algorithm="test_algorithm",
        created=1234567890,
        expires=1234567899,
        access=["read", "write"],
        tags=["test", "key"]
    )

def test_initialization():
    """Test KeyStorage initialization."""
    store = KeyStorage()
    assert not store._initialized
    assert store.initialize()
    assert store._initialized
    assert store.shutdown()
    assert not store._initialized

def test_store_key(storage, test_key, test_metadata):
    """Test key storage functionality."""
    # Test storing key with metadata
    assert storage.store_key("test_key_1", test_key, test_metadata)
    
    # Test storing key without metadata
    assert storage.store_key("test_key_2", test_key)
    
    # Test storing key with None metadata
    assert storage.store_key("test_key_3", test_key, None)

def test_retrieve_key(storage, test_key, test_metadata):
    """Test key retrieval functionality."""
    # Store a key first
    storage.store_key("test_key", test_key, test_metadata)
    
    # Retrieve the key
    retrieved_key = storage.retrieve_key("test_key")
    assert retrieved_key == test_key
    
    # Retrieve metadata
    retrieved_metadata = storage.retrieve_metadata("test_key")
    assert retrieved_metadata.algorithm == test_metadata.algorithm
    assert retrieved_metadata.created == test_metadata.created
    assert retrieved_metadata.expires == test_metadata.expires
    assert retrieved_metadata.access == test_metadata.access
    assert retrieved_metadata.tags == test_metadata.tags

def test_delete_key(storage, test_key):
    """Test key deletion functionality."""
    # Store a key first
    storage.store_key("test_key", test_key)
    
    # Delete the key
    assert storage.delete_key("test_key")
    
    # Verify key is deleted
    with pytest.raises(SatoxError) as exc_info:
        storage.retrieve_key("test_key")
    assert exc_info.value.code == "KEY_NOT_FOUND"

def test_list_keys(storage, test_key):
    """Test key listing functionality."""
    # Store multiple keys
    storage.store_key("key1", test_key)
    storage.store_key("key2", test_key)
    storage.store_key("key3", test_key)
    
    # List all keys
    keys = storage.list_keys()
    assert len(keys) == 3
    assert "key1" in keys
    assert "key2" in keys
    assert "key3" in keys

def test_uninitialized_operations():
    """Test operations when not initialized."""
    store = KeyStorage()
    with pytest.raises(SatoxError) as exc_info:
        store.store_key("test", b"key")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        store.retrieve_key("test")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        store.delete_key("test")
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(storage):
    """Test invalid input handling."""
    # Test with None inputs
    with pytest.raises(SatoxError) as exc_info:
        storage.store_key(None, b"key")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        storage.store_key("test", None)
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        storage.retrieve_key(None)
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        storage.delete_key(None)
    assert exc_info.value.code == "INVALID_INPUT"

def test_double_initialization():
    """Test double initialization handling."""
    store = KeyStorage()
    assert store.initialize()
    assert store.initialize()  # Should not raise an error
    assert store.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    store = KeyStorage()
    assert store.initialize()
    assert store.shutdown()
    assert store.shutdown()  # Should not raise an error 