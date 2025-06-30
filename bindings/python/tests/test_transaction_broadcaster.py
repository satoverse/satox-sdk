"""
Unit tests for TransactionBroadcaster Python bindings
"""

import pytest
from satox.transactions import TransactionBroadcaster, Transaction, TransactionStatus
from satox.types import SatoxError

@pytest.fixture
def broadcaster():
    b = TransactionBroadcaster()
    assert b.initialize()
    yield b
    b.shutdown()

@pytest.fixture
def test_transaction():
    return Transaction(
        id="tx_broadcast",
        sender="sender_address",
        recipient="recipient_address",
        amount=100,
        timestamp=1234567890,
        signature="test_signature",
        status=TransactionStatus.PENDING
    )

def test_initialization():
    b = TransactionBroadcaster()
    assert not b._initialized
    assert b.initialize()
    assert b._initialized
    assert b.shutdown()
    assert not b._initialized

def test_broadcast_transaction(broadcaster, test_transaction):
    assert broadcaster.broadcast(test_transaction)
    tx = broadcaster.get_broadcasted_transaction(test_transaction.id)
    assert tx.status == TransactionStatus.BROADCAST

def test_confirm_broadcast(broadcaster, test_transaction):
    broadcaster.broadcast(test_transaction)
    assert broadcaster.confirm_broadcast(test_transaction.id)
    tx = broadcaster.get_broadcasted_transaction(test_transaction.id)
    assert tx.status == TransactionStatus.CONFIRMED

def test_uninitialized_operations(test_transaction):
    b = TransactionBroadcaster()
    with pytest.raises(SatoxError) as exc_info:
        b.broadcast(test_transaction)
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(broadcaster):
    with pytest.raises(SatoxError) as exc_info:
        broadcaster.broadcast(None)
    assert exc_info.value.code == "INVALID_INPUT"

def test_double_initialization():
    b = TransactionBroadcaster()
    assert b.initialize()
    assert b.initialize()
    assert b.shutdown()

def test_double_shutdown():
    b = TransactionBroadcaster()
    assert b.initialize()
    assert b.shutdown()
    assert b.shutdown() 