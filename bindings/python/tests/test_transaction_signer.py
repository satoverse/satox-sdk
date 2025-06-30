"""
Unit tests for TransactionSigner Python bindings
"""

import pytest
from satox.transactions import TransactionSigner, Transaction, TransactionStatus
from satox.types import SatoxError

@pytest.fixture
def signer():
    s = TransactionSigner()
    assert s.initialize()
    yield s
    s.shutdown()

@pytest.fixture
def unsigned_transaction():
    return Transaction(
        id="tx_unsigned",
        sender="sender_address",
        recipient="recipient_address",
        amount=100,
        timestamp=1234567890,
        signature="",
        status=TransactionStatus.PENDING
    )

def test_initialization():
    s = TransactionSigner()
    assert not s._initialized
    assert s.initialize()
    assert s._initialized
    assert s.shutdown()
    assert not s._initialized

def test_sign_transaction(signer, unsigned_transaction):
    signed_tx = signer.sign(unsigned_transaction, "private_key")
    assert signed_tx.signature != ""
    assert signed_tx.id == unsigned_transaction.id

def test_uninitialized_operations(unsigned_transaction):
    s = TransactionSigner()
    with pytest.raises(SatoxError) as exc_info:
        s.sign(unsigned_transaction, "private_key")
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(signer):
    with pytest.raises(SatoxError) as exc_info:
        signer.sign(None, "private_key")
    assert exc_info.value.code == "INVALID_INPUT"
    with pytest.raises(SatoxError) as exc_info:
        signer.sign(unsigned_transaction, None)
    assert exc_info.value.code == "INVALID_INPUT"

def test_double_initialization():
    s = TransactionSigner()
    assert s.initialize()
    assert s.initialize()
    assert s.shutdown()

def test_double_shutdown():
    s = TransactionSigner()
    assert s.initialize()
    assert s.shutdown()
    assert s.shutdown() 