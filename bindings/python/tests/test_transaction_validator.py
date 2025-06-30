"""
Unit tests for TransactionValidator Python bindings
"""

import pytest
from satox.transactions import TransactionValidator, Transaction, TransactionStatus
from satox.types import SatoxError

@pytest.fixture
def validator():
    v = TransactionValidator()
    assert v.initialize()
    yield v
    v.shutdown()

@pytest.fixture
def valid_transaction():
    return Transaction(
        id="tx_valid",
        sender="sender_address",
        recipient="recipient_address",
        amount=100,
        timestamp=1234567890,
        signature="valid_signature",
        status=TransactionStatus.PENDING
    )

@pytest.fixture
def invalid_transaction():
    return Transaction(
        id="tx_invalid",
        sender="",
        recipient="recipient_address",
        amount=-50,
        timestamp=1234567890,
        signature="",
        status=TransactionStatus.PENDING
    )

def test_initialization():
    v = TransactionValidator()
    assert not v._initialized
    assert v.initialize()
    assert v._initialized
    assert v.shutdown()
    assert not v._initialized

def test_validate_valid_transaction(validator, valid_transaction):
    assert validator.validate(valid_transaction)

def test_validate_invalid_transaction(validator, invalid_transaction):
    assert not validator.validate(invalid_transaction)

def test_uninitialized_operations(valid_transaction):
    v = TransactionValidator()
    with pytest.raises(SatoxError) as exc_info:
        v.validate(valid_transaction)
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(validator):
    with pytest.raises(SatoxError) as exc_info:
        validator.validate(None)
    assert exc_info.value.code == "INVALID_INPUT"

def test_double_initialization():
    v = TransactionValidator()
    assert v.initialize()
    assert v.initialize()
    assert v.shutdown()

def test_double_shutdown():
    v = TransactionValidator()
    assert v.initialize()
    assert v.shutdown()
    assert v.shutdown() 