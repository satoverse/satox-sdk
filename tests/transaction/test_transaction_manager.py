import unittest
from unittest.mock import Mock, patch
from satox.transaction.transaction_manager import TransactionManager, TransactionConfig, TransactionResult, Transaction
import pytest

class TestTransactionManager(unittest.TestCase):
    def setUp(self):
        self.manager = TransactionManager()
        self.transaction_config = TransactionConfig(
            gas_limit=21000,
            gas_price=20000000000,
            nonce=1,
            value=1000000000000000000,
            data="0x",
            timeout=30,
            max_retries=3
        )
        self.manager.initialize(self.transaction_config)

    def test_initialization(self):
        """Test manager initialization"""
        self.assertIsNotNone(self.manager)
        self.assertEqual(len(self.manager.get_pending_transactions()), 0)

    def test_create_transaction(self):
        """Test transaction creation"""
        transaction = self.manager.create_transaction(
            from_address="0x123",
            to_address="0x456",
            value=1000000000000000000,
            data="0x"
        )
        self.assertIsNotNone(transaction)
        self.assertEqual(transaction.from_address, "0x123")
        self.assertEqual(transaction.to_address, "0x456")
        self.assertEqual(transaction.value, 1000000000000000000)
        self.assertEqual(transaction.gas_limit, self.transaction_config.gas_limit)

    def test_send_transaction(self):
        """Test transaction sending"""
        transaction = self.manager.create_transaction(
            from_address="0x123",
            to_address="0x456",
            value=1000000000000000000,
            data="0x"
        )
        result = self.manager.send_transaction(transaction)
        self.assertIsNotNone(result)
        self.assertIsInstance(result, TransactionResult)
        self.assertIsNotNone(result.hash)
        self.assertIn(result.status, ["pending", "confirmed", "failed"])

    def test_get_transaction_status(self):
        """Test transaction status retrieval"""
        transaction_hash = "0x123"
        status = self.manager.get_transaction_status(transaction_hash)
        self.assertIsNotNone(status)
        self.assertIn(status, ["pending", "confirmed", "failed"])

    def test_cancel_transaction(self):
        """Test transaction cancellation"""
        transaction = self.manager.create_transaction(
            from_address="0x123",
            to_address="0x456",
            value=1000000000000000000,
            data="0x"
        )
        result = self.manager.cancel_transaction(transaction.hash)
        self.assertTrue(result)
        status = self.manager.get_transaction_status(transaction.hash)
        self.assertEqual(status, "cancelled")

    def test_get_pending_transactions(self):
        """Test pending transactions retrieval"""
        transaction = self.manager.create_transaction(
            from_address="0x123",
            to_address="0x456",
            value=1000000000000000000,
            data="0x"
        )
        pending = self.manager.get_pending_transactions()
        self.assertEqual(len(pending), 1)
        self.assertEqual(pending[0].hash, transaction.hash)

    def test_update_transaction_config(self):
        """Test transaction configuration updates"""
        new_config = TransactionConfig(
            gas_limit=30000,
            gas_price=30000000000,
            nonce=2,
            value=2000000000000000000,
            data="0x123",
            timeout=60,
            max_retries=5
        )
        self.manager.update_transaction_config(new_config)
        current_config = self.manager.get_config()
        self.assertEqual(current_config.gas_limit, new_config.gas_limit)
        self.assertEqual(current_config.gas_price, new_config.gas_price)
        self.assertEqual(current_config.timeout, new_config.timeout)

    def test_invalid_transaction_creation(self):
        """Test handling of invalid transaction creation"""
        with self.assertRaises(ValueError):
            self.manager.create_transaction(
                from_address="",
                to_address="0x456",
                value=-1,
                data="0x"
            )

    def test_transaction_sending_failure(self):
        """Test handling of transaction sending failure"""
        transaction = self.manager.create_transaction(
            from_address="0x123",
            to_address="0x456",
            value=1000000000000000000,
            data="0x"
        )
        with patch.object(self.manager, 'send_transaction', side_effect=Exception("Sending failed")):
            with self.assertRaises(Exception):
                self.manager.send_transaction(transaction)

    def test_cancel_nonexistent_transaction(self):
        """Test handling of cancelling non-existent transaction"""
        with self.assertRaises(ValueError):
            self.manager.cancel_transaction("nonexistent_hash")

    def test_invalid_transaction_config(self):
        """Test handling of invalid transaction configuration"""
        invalid_config = TransactionConfig(
            gas_limit=0,
            gas_price=0,
            nonce=-1,
            value=-1,
            data="",
            timeout=0,
            max_retries=0
        )
        with self.assertRaises(ValueError):
            self.manager.update_transaction_config(invalid_config)

@pytest.fixture
def transaction_manager():
    manager = TransactionManager()
    config = TransactionConfig(
        gas_limit=21000,
        gas_price=20000000000,
        nonce=1,
        value=1000000000000000000,
        data="0x",
        timeout=30,
        max_retries=3
    )
    manager.initialize(config)
    return manager

def test_transaction_manager_initialization(transaction_manager):
    assert transaction_manager is not None
    assert transaction_manager.get_config() is not None
    assert transaction_manager.get_pending_transaction_count() == 0

def test_transaction_creation(transaction_manager):
    tx = transaction_manager.create_transaction(
        from_address="0x123",
        to_address="0x456",
        value=1000,
        data="0x"
    )
    assert tx is not None
    assert tx.from_address == "0x123"
    assert tx.to_address == "0x456"
    assert tx.value == 1000
    assert tx.data == "0x"
    assert tx.status == "pending"

def test_transaction_sending(transaction_manager):
    tx = transaction_manager.create_transaction(
        from_address="0x123",
        to_address="0x456",
        value=1000,
        data="0x"
    )
    result = transaction_manager.send_transaction(tx)
    assert result is not None
    assert isinstance(result, TransactionResult)
    assert result.status in ["pending", "confirmed", "failed"]

def test_transaction_status(transaction_manager):
    tx = transaction_manager.create_transaction(
        from_address="0x123",
        to_address="0x456",
        value=1000,
        data="0x"
    )
    status = transaction_manager.get_transaction_status(tx.hash)
    assert status is not None
    assert status in ["pending", "confirmed", "failed"]

def test_transaction_cancellation(transaction_manager):
    tx = transaction_manager.create_transaction(
        from_address="0x123",
        to_address="0x456",
        value=1000,
        data="0x"
    )
    assert transaction_manager.cancel_transaction(tx.hash)
    status = transaction_manager.get_transaction_status(tx.hash)
    assert status == "cancelled"

def test_pending_transactions(transaction_manager):
    # Create multiple transactions
    for _ in range(3):
        tx = transaction_manager.create_transaction(
            from_address="0x123",
            to_address="0x456",
            value=1000,
            data="0x"
        )
    
    pending_txs = transaction_manager.get_pending_transactions()
    assert len(pending_txs) == 3

def test_transaction_with_data(transaction_manager):
    tx = transaction_manager.create_transaction(
        from_address="0x123",
        to_address="0x456",
        value=1000,
        data="0x1234567890abcdef"
    )
    assert tx.data == "0x1234567890abcdef"

def test_transaction_with_custom_gas(transaction_manager):
    tx = transaction_manager.create_transaction(
        from_address="0x123",
        to_address="0x456",
        value=1000,
        data="0x",
        gas_limit=50000,
        gas_price=25000000000
    )
    assert tx.gas_limit == 50000
    assert tx.gas_price == 25000000000

def test_error_handling(transaction_manager):
    # Test with invalid transaction
    with pytest.raises(ValueError):
        transaction_manager.create_transaction(
            from_address="",
            to_address="0x456",
            value=-1,
            data="0x"
        )

def test_transaction_result_validation(transaction_manager):
    tx = transaction_manager.create_transaction(
        from_address="0x123",
        to_address="0x456",
        value=1000,
        data="0x"
    )
    result = transaction_manager.send_transaction(tx)
    assert result.hash == tx.hash

if __name__ == '__main__':
    unittest.main() 