"""Tests for the transaction processor module."""

import pytest
import json
import hashlib
from satox_bindings.core import TransactionProcessor

@pytest.fixture
def transaction_processor():
    """Create a transaction processor instance for testing."""
    processor = TransactionProcessor()
    processor.initialize()
    yield processor
    processor.shutdown()

@pytest.fixture
def sample_transaction():
    """Create a sample transaction for testing."""
    transaction = {
        "sender": "0xabc",
        "receiver": "0xdef",
        "amount": 100,
        "timestamp": 1234567890,
        "signature": "0x123"
    }
    # Calculate hash
    tx_copy = transaction.copy()
    tx_str = json.dumps(tx_copy, sort_keys=True)
    transaction["hash"] = hashlib.sha256(tx_str.encode()).hexdigest()
    return transaction

def test_initialize(transaction_processor):
    """Test transaction processor initialization."""
    assert transaction_processor.initialize() is True
    assert transaction_processor.get_last_error() == ""

def test_shutdown(transaction_processor):
    """Test transaction processor shutdown."""
    assert transaction_processor.shutdown() is True
    assert transaction_processor.get_transaction_pool() == []

def test_process_transaction(transaction_processor, sample_transaction):
    """Test transaction processing."""
    transaction_data = json.dumps(sample_transaction).encode()
    assert transaction_processor.process_transaction(transaction_data) is True
    assert transaction_processor.get_transaction(sample_transaction["hash"]) == sample_transaction

def test_get_transaction(transaction_processor, sample_transaction):
    """Test getting a transaction by hash."""
    transaction_data = json.dumps(sample_transaction).encode()
    transaction_processor.process_transaction(transaction_data)
    assert transaction_processor.get_transaction(sample_transaction["hash"]) == sample_transaction

def test_get_transaction_pool(transaction_processor, sample_transaction):
    """Test getting the transaction pool."""
    transaction_data = json.dumps(sample_transaction).encode()
    transaction_processor.process_transaction(transaction_data)
    pool = transaction_processor.get_transaction_pool()
    assert len(pool) == 1
    assert pool[0] == sample_transaction

def test_mark_transaction_processed(transaction_processor, sample_transaction):
    """Test marking a transaction as processed."""
    transaction_data = json.dumps(sample_transaction).encode()
    transaction_processor.process_transaction(transaction_data)
    assert transaction_processor.mark_transaction_processed(sample_transaction["hash"]) is True
    assert transaction_processor.get_transaction_pool() == []
    assert transaction_processor.get_transaction(sample_transaction["hash"]) == sample_transaction

def test_invalid_transaction(transaction_processor):
    """Test processing an invalid transaction."""
    invalid_transaction = {
        "hash": "0x123",
        "sender": "0xabc"
        # Missing required fields
    }
    transaction_data = json.dumps(invalid_transaction).encode()
    assert transaction_processor.process_transaction(transaction_data) is False
    assert "Missing required field" in transaction_processor.get_last_error()

def test_duplicate_transaction(transaction_processor, sample_transaction):
    """Test processing a duplicate transaction."""
    transaction_data = json.dumps(sample_transaction).encode()
    assert transaction_processor.process_transaction(transaction_data) is True
    assert transaction_processor.process_transaction(transaction_data) is False
    assert "already processed" in transaction_processor.get_last_error()

def test_invalid_hash(transaction_processor):
    """Test processing a transaction with invalid hash."""
    transaction = {
        "hash": "0x123",  # Invalid hash
        "sender": "0xabc",
        "receiver": "0xdef",
        "amount": 100,
        "timestamp": 1234567890,
        "signature": "0x456"
    }
    transaction_data = json.dumps(transaction).encode()
    assert transaction_processor.process_transaction(transaction_data) is False
    assert "Invalid transaction hash" in transaction_processor.get_last_error()

def test_error_handling(transaction_processor):
    """Test error handling."""
    transaction_processor.shutdown()
    assert transaction_processor.process_transaction(b"{}") is False
    assert "not initialized" in transaction_processor.get_last_error()

def test_clear_error(transaction_processor):
    """Test clearing error message."""
    transaction_processor.shutdown()
    transaction_processor.process_transaction(b"{}")
    assert transaction_processor.get_last_error() != ""
    transaction_processor.clear_last_error()
    assert transaction_processor.get_last_error() == "" 