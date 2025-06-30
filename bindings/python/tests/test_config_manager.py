"""
Unit tests for ConfigManager Python bindings
"""

import pytest
import os
from satox.core import ConfigManager
from satox.types import SatoxError

@pytest.fixture
def manager():
    """Create a ConfigManager instance for testing."""
    mgr = ConfigManager()
    assert mgr.initialize()
    yield mgr
    mgr.shutdown()

@pytest.fixture
def test_config_file(tmp_path):
    """Create a temporary test config file."""
    config_path = tmp_path / "test_config.json"
    config_content = """
    {
        "network": {
            "host": "localhost",
            "port": 8080
        },
        "security": {
            "key_size": 2048,
            "algorithm": "RSA"
        }
    }
    """
    config_path.write_text(config_content)
    return str(config_path)

def test_initialization():
    """Test ConfigManager initialization."""
    mgr = ConfigManager()
    assert not mgr._initialized
    assert mgr.initialize()
    assert mgr._initialized
    assert mgr.shutdown()
    assert not mgr._initialized

def test_load_config(manager, test_config_file):
    """Test configuration loading functionality."""
    # Test with valid config file
    assert manager.load_config(test_config_file)

    # Test with invalid config path
    with pytest.raises(ValueError):
        manager.load_config(None)

    # Test with non-existent config file
    with pytest.raises(ValueError):
        manager.load_config("nonexistent_config.json")

def test_uninitialized_operations():
    """Test operations when not initialized."""
    mgr = ConfigManager()
    with pytest.raises(RuntimeError) as exc_info:
        mgr.load_config("test_config.json")
    assert "not initialized" in str(exc_info.value)

def test_double_initialization():
    """Test double initialization handling."""
    mgr = ConfigManager()
    assert mgr.initialize()
    assert mgr.initialize()  # Should not raise an error
    assert mgr.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    mgr = ConfigManager()
    assert mgr.initialize()
    assert mgr.shutdown()
    assert mgr.shutdown()  # Should not raise an error 