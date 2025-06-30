"""
Unit tests for BlockchainManager Python bindings
"""

import pytest
from satox.blockchain import BlockchainManager, Block, Transaction, Account
from satox.types import SatoxError

@pytest.fixture
def blockchain():
    """Create a BlockchainManager instance for testing."""
    manager = BlockchainManager()
    assert manager.initialize()
    yield manager
    manager.shutdown()

@pytest.fixture
def test_block():
    """Create a test block."""
    return Block(
        height=1,
        timestamp=1234567890,
        previous_hash="0000000000000000000000000000000000000000000000000000000000000000",
        transactions=[],
        hash="1111111111111111111111111111111111111111111111111111111111111111"
    )

@pytest.fixture
def test_transaction():
    """Create a test transaction."""
    return Transaction(
        id="test_tx_1",
        sender="sender_address",
        recipient="recipient_address",
        amount=100,
        timestamp=1234567890,
        signature="test_signature"
    )

@pytest.fixture
def test_account():
    """Create a test account."""
    return Account(
        address="test_address",
        balance=1000,
        nonce=0,
        created_at=1234567890
    )

def test_initialization():
    """Test BlockchainManager initialization."""
    manager = BlockchainManager()
    assert not manager._initialized
    assert manager.initialize()
    assert manager._initialized
    assert manager.shutdown()
    assert not manager._initialized

def test_block_operations(blockchain, test_block):
    """Test block operations."""
    # Test block creation
    assert blockchain.create_block(test_block)
    
    # Test block retrieval
    retrieved_block = blockchain.get_block(test_block.height)
    assert retrieved_block.height == test_block.height
    assert retrieved_block.hash == test_block.hash
    
    # Test block validation
    assert blockchain.validate_block(test_block)
    
    # Test block deletion
    assert blockchain.delete_block(test_block.height)
    
    # Verify block is deleted
    with pytest.raises(SatoxError) as exc_info:
        blockchain.get_block(test_block.height)
    assert exc_info.value.code == "BLOCK_NOT_FOUND"

def test_transaction_operations(blockchain, test_transaction):
    """Test transaction operations."""
    # Test transaction creation
    assert blockchain.create_transaction(test_transaction)
    
    # Test transaction retrieval
    retrieved_tx = blockchain.get_transaction(test_transaction.id)
    assert retrieved_tx.id == test_transaction.id
    assert retrieved_tx.amount == test_transaction.amount
    
    # Test transaction validation
    assert blockchain.validate_transaction(test_transaction)
    
    # Test transaction deletion
    assert blockchain.delete_transaction(test_transaction.id)
    
    # Verify transaction is deleted
    with pytest.raises(SatoxError) as exc_info:
        blockchain.get_transaction(test_transaction.id)
    assert exc_info.value.code == "TRANSACTION_NOT_FOUND"

def test_account_operations(blockchain, test_account):
    """Test account operations."""
    # Test account creation
    assert blockchain.create_account(test_account)
    
    # Test account retrieval
    retrieved_account = blockchain.get_account(test_account.address)
    assert retrieved_account.address == test_account.address
    assert retrieved_account.balance == test_account.balance
    
    # Test account update
    test_account.balance = 2000
    assert blockchain.update_account(test_account)
    updated_account = blockchain.get_account(test_account.address)
    assert updated_account.balance == 2000
    
    # Test account deletion
    assert blockchain.delete_account(test_account.address)
    
    # Verify account is deleted
    with pytest.raises(SatoxError) as exc_info:
        blockchain.get_account(test_account.address)
    assert exc_info.value.code == "ACCOUNT_NOT_FOUND"

def test_network_operations(blockchain):
    """Test network operations."""
    # Test network connection
    assert blockchain.connect_to_network("test_network")
    
    # Test network status
    assert blockchain.get_network_status() == "connected"
    
    # Test network disconnection
    assert blockchain.disconnect_from_network()
    assert blockchain.get_network_status() == "disconnected"

def test_uninitialized_operations():
    """Test operations when not initialized."""
    manager = BlockchainManager()
    with pytest.raises(SatoxError) as exc_info:
        manager.create_block(None)
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        manager.create_transaction(None)
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        manager.create_account(None)
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(blockchain):
    """Test invalid input handling."""
    # Test with None inputs
    with pytest.raises(SatoxError) as exc_info:
        blockchain.create_block(None)
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        blockchain.create_transaction(None)
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        blockchain.create_account(None)
    assert exc_info.value.code == "INVALID_INPUT"

def test_double_initialization():
    """Test double initialization handling."""
    manager = BlockchainManager()
    assert manager.initialize()
    assert manager.initialize()  # Should not raise an error
    assert manager.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    manager = BlockchainManager()
    assert manager.initialize()
    assert manager.shutdown()
    assert manager.shutdown()  # Should not raise an error 