"""
Unit tests for TransactionManager Python bindings
"""

import pytest
from satox.transactions import TransactionManager, Transaction, TransactionStatus
from satox.types import SatoxError

@pytest.fixture
def transaction_manager():
    """Create a TransactionManager instance for testing."""
    manager = TransactionManager()
    assert manager.initialize()
    yield manager
    manager.shutdown()

@pytest.fixture
def test_transaction():
    """Create a test transaction."""
    return Transaction(
        id="test_tx_1",
        sender="sender_address",
        recipient="recipient_address",
        amount=100,
        timestamp=1234567890,
        signature="test_signature",
        status=TransactionStatus.PENDING
    )

def test_initialization():
    """Test TransactionManager initialization."""
    manager = TransactionManager()
    assert not manager._initialized
    assert manager.initialize()
    assert manager._initialized
    assert manager.shutdown()
    assert not manager._initialized

def test_transaction_creation(transaction_manager, test_transaction):
    """Test transaction creation and validation."""
    # Test transaction creation
    assert transaction_manager.create_transaction(test_transaction)
    
    # Test transaction retrieval
    retrieved_tx = transaction_manager.get_transaction(test_transaction.id)
    assert retrieved_tx.id == test_transaction.id
    assert retrieved_tx.amount == test_transaction.amount
    assert retrieved_tx.status == TransactionStatus.PENDING
    
    # Test transaction validation
    assert transaction_manager.validate_transaction(test_transaction)
    
    # Test invalid transaction
    invalid_tx = Transaction(
        id="invalid_tx",
        sender="",  # Invalid sender
        recipient="recipient_address",
        amount=-100,  # Invalid amount
        timestamp=1234567890,
        signature="",
        status=TransactionStatus.PENDING
    )
    assert not transaction_manager.validate_transaction(invalid_tx)

def test_transaction_signing(transaction_manager, test_transaction):
    """Test transaction signing and verification."""
    # Test transaction signing
    signed_tx = transaction_manager.sign_transaction(test_transaction, "private_key")
    assert signed_tx.signature != test_transaction.signature
    
    # Test signature verification
    assert transaction_manager.verify_signature(signed_tx)
    
    # Test invalid signature
    invalid_signed_tx = Transaction(
        id="invalid_signed_tx",
        sender="sender_address",
        recipient="recipient_address",
        amount=100,
        timestamp=1234567890,
        signature="invalid_signature",
        status=TransactionStatus.PENDING
    )
    assert not transaction_manager.verify_signature(invalid_signed_tx)

def test_transaction_broadcasting(transaction_manager, test_transaction):
    """Test transaction broadcasting."""
    # Test transaction broadcast
    assert transaction_manager.broadcast_transaction(test_transaction)
    
    # Verify transaction status
    broadcasted_tx = transaction_manager.get_transaction(test_transaction.id)
    assert broadcasted_tx.status == TransactionStatus.BROADCAST
    
    # Test broadcast confirmation
    assert transaction_manager.confirm_broadcast(test_transaction.id)
    confirmed_tx = transaction_manager.get_transaction(test_transaction.id)
    assert confirmed_tx.status == TransactionStatus.CONFIRMED

def test_transaction_status_tracking(transaction_manager, test_transaction):
    """Test transaction status tracking."""
    # Create transaction
    transaction_manager.create_transaction(test_transaction)
    
    # Test status updates
    assert transaction_manager.update_status(test_transaction.id, TransactionStatus.PROCESSING)
    tx = transaction_manager.get_transaction(test_transaction.id)
    assert tx.status == TransactionStatus.PROCESSING
    
    assert transaction_manager.update_status(test_transaction.id, TransactionStatus.COMPLETED)
    tx = transaction_manager.get_transaction(test_transaction.id)
    assert tx.status == TransactionStatus.COMPLETED
    
    # Test invalid status transition
    assert not transaction_manager.update_status(test_transaction.id, TransactionStatus.PENDING)

def test_transaction_deletion(transaction_manager, test_transaction):
    """Test transaction deletion."""
    # Create transaction
    transaction_manager.create_transaction(test_transaction)
    
    # Delete transaction
    assert transaction_manager.delete_transaction(test_transaction.id)
    
    # Verify deletion
    with pytest.raises(SatoxError) as exc_info:
        transaction_manager.get_transaction(test_transaction.id)
    assert exc_info.value.code == "TRANSACTION_NOT_FOUND"

def test_transaction_listing(transaction_manager, test_transaction):
    """Test transaction listing functionality."""
    # Create multiple transactions
    for i in range(3):
        tx = Transaction(
            id=f"test_tx_{i}",
            sender="sender_address",
            recipient="recipient_address",
            amount=100,
            timestamp=1234567890,
            signature="test_signature",
            status=TransactionStatus.PENDING
        )
        transaction_manager.create_transaction(tx)
    
    # List all transactions
    transactions = transaction_manager.list_transactions()
    assert len(transactions) == 3
    
    # List transactions by status
    pending_txs = transaction_manager.list_transactions_by_status(TransactionStatus.PENDING)
    assert len(pending_txs) == 3

def test_uninitialized_operations():
    """Test operations when not initialized."""
    manager = TransactionManager()
    with pytest.raises(SatoxError) as exc_info:
        manager.create_transaction(None)
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        manager.sign_transaction(None, "key")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        manager.broadcast_transaction(None)
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(transaction_manager):
    """Test invalid input handling."""
    # Test with None inputs
    with pytest.raises(SatoxError) as exc_info:
        transaction_manager.create_transaction(None)
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        transaction_manager.sign_transaction(None, "key")
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        transaction_manager.broadcast_transaction(None)
    assert exc_info.value.code == "INVALID_INPUT"

def test_double_initialization():
    """Test double initialization handling."""
    manager = TransactionManager()
    assert manager.initialize()
    assert manager.initialize()  # Should not raise an error
    assert manager.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    manager = TransactionManager()
    assert manager.initialize()
    assert manager.shutdown()
    assert manager.shutdown()  # Should not raise an error 