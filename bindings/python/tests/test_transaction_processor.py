"""
Unit tests for TransactionProcessor Python bindings
"""

import pytest
from satox.core import TransactionProcessor
from satox.types import SatoxError

@pytest.fixture
def processor():
    """Create a TransactionProcessor instance for testing."""
    proc = TransactionProcessor()
    assert proc.initialize()
    yield proc
    proc.shutdown()

def test_initialization():
    """Test TransactionProcessor initialization."""
    proc = TransactionProcessor()
    assert not proc._initialized
    assert proc.initialize()
    assert proc._initialized
    assert proc.shutdown()
    assert not proc._initialized

def test_process_transaction(processor):
    """Test transaction processing functionality."""
    # Test with valid transaction data
    valid_transaction = b"test_transaction_data"
    assert processor.process_transaction(valid_transaction)

    # Test with invalid transaction data
    with pytest.raises(ValueError):
        processor.process_transaction(None)

    # Test with empty transaction data
    with pytest.raises(ValueError):
        processor.process_transaction(b"")

def test_uninitialized_operations():
    """Test operations when not initialized."""
    proc = TransactionProcessor()
    with pytest.raises(RuntimeError) as exc_info:
        proc.process_transaction(b"test")
    assert "not initialized" in str(exc_info.value)

def test_double_initialization():
    """Test double initialization handling."""
    proc = TransactionProcessor()
    assert proc.initialize()
    assert proc.initialize()  # Should not raise an error
    assert proc.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    proc = TransactionProcessor()
    assert proc.initialize()
    assert proc.shutdown()
    assert proc.shutdown()  # Should not raise an error
