"""
Unit tests for SecurityManager Python bindings
"""

import pytest
from satox.core import SecurityManager
from satox.types import SatoxError

@pytest.fixture
def manager():
    """Create a SecurityManager instance for testing."""
    mgr = SecurityManager()
    assert mgr.initialize()
    yield mgr
    mgr.shutdown()

def test_initialization():
    """Test SecurityManager initialization."""
    mgr = SecurityManager()
    assert not mgr._initialized
    assert mgr.initialize()
    assert mgr._initialized
    assert mgr.shutdown()
    assert not mgr._initialized

def test_generate_key_pair(manager):
    """Test key pair generation functionality."""
    # Test key pair generation
    private_key, public_key = manager.generate_key_pair()
    assert private_key is not None
    assert public_key is not None
    assert len(private_key) > 0
    assert len(public_key) > 0

def test_uninitialized_operations():
    """Test operations when not initialized."""
    mgr = SecurityManager()
    with pytest.raises(RuntimeError) as exc_info:
        mgr.generate_key_pair()
    assert "not initialized" in str(exc_info.value)

def test_double_initialization():
    """Test double initialization handling."""
    mgr = SecurityManager()
    assert mgr.initialize()
    assert mgr.initialize()  # Should not raise an error
    assert mgr.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    mgr = SecurityManager()
    assert mgr.initialize()
    assert mgr.shutdown()
    assert mgr.shutdown()  # Should not raise an error 