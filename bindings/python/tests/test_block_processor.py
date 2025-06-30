"""
Unit tests for BlockProcessor Python bindings
"""

import pytest
from satox.core import BlockProcessor
from satox.types import SatoxError

@pytest.fixture
def processor():
    """Create a BlockProcessor instance for testing."""
    proc = BlockProcessor()
    assert proc.initialize()
    yield proc
    proc.shutdown()

def test_initialization():
    """Test BlockProcessor initialization."""
    proc = BlockProcessor()
    assert not proc._initialized
    assert proc.initialize()
    assert proc._initialized
    assert proc.shutdown()
    assert not proc._initialized

def test_process_block(processor):
    """Test block processing functionality."""
    # Test with valid block data
    valid_block = b"test_block_data"
    assert processor.process_block(valid_block)

    # Test with invalid block data
    with pytest.raises(ValueError):
        processor.process_block(None)

    # Test with empty block data
    with pytest.raises(ValueError):
        processor.process_block(b"")

def test_uninitialized_operations():
    """Test operations when not initialized."""
    proc = BlockProcessor()
    with pytest.raises(RuntimeError) as exc_info:
        proc.process_block(b"test")
    assert "not initialized" in str(exc_info.value)

def test_double_initialization():
    """Test double initialization handling."""
    proc = BlockProcessor()
    assert proc.initialize()
    assert proc.initialize()  # Should not raise an error
    assert proc.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    proc = BlockProcessor()
    assert proc.initialize()
    assert proc.shutdown()
    assert proc.shutdown()  # Should not raise an error 