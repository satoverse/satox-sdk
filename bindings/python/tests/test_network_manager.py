"""
Unit tests for NetworkManager Python bindings
"""

import pytest
from satox.core import NetworkManager
from satox.types import SatoxError

@pytest.fixture
def manager():
    """Create a NetworkManager instance for testing."""
    mgr = NetworkManager()
    assert mgr.initialize()
    yield mgr
    mgr.shutdown()

def test_initialization():
    """Test NetworkManager initialization."""
    mgr = NetworkManager()
    assert not mgr._initialized
    assert mgr.initialize()
    assert mgr._initialized
    assert mgr.shutdown()
    assert not mgr._initialized

def test_connect(manager):
    """Test network connection functionality."""
    # Test with valid connection parameters
    assert manager.connect("localhost", 8080)

    # Test with invalid host
    with pytest.raises(ValueError):
        manager.connect(None, 8080)

    # Test with invalid port
    with pytest.raises(ValueError):
        manager.connect("localhost", -1)

def test_uninitialized_operations():
    """Test operations when not initialized."""
    mgr = NetworkManager()
    with pytest.raises(RuntimeError) as exc_info:
        mgr.connect("localhost", 8080)
    assert "not initialized" in str(exc_info.value)

def test_double_initialization():
    """Test double initialization handling."""
    mgr = NetworkManager()
    assert mgr.initialize()
    assert mgr.initialize()  # Should not raise an error
    assert mgr.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    mgr = NetworkManager()
    assert mgr.initialize()
    assert mgr.shutdown()
    assert mgr.shutdown()  # Should not raise an error 