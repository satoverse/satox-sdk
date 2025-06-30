import unittest
from satox.tests.utils.mock_wallet import MockWallet, CoinType
import pytest
import time
from satox.satox_bindings.core import BlockProcessor

class TestMockWallet(unittest.TestCase):
    def setUp(self):
        """Set up test environment with initial balances."""
        self.wallet = MockWallet()
        self.wallet.connect()
        
        # Create addresses
        self.address1 = self.wallet.create_address()
        self.address2 = self.wallet.create_address()
        
        # Initialize address1 with initial balance
        self.initial_balance = 1000000000000000000  # 1 SATOX
        self.wallet._addresses[self.address1] = {"SATOX": self.initial_balance}

    def test_connection(self):
        """Test wallet connection"""
        self.assertTrue(self.wallet.is_connected())
        self.wallet.disconnect()
        self.assertFalse(self.wallet.is_connected())
        self.wallet.connect()
        self.assertTrue(self.wallet.is_connected())

    def test_coin_type(self):
        """Test coin type management"""
        self.assertEqual(self.wallet.get_coin_type(), CoinType.CURRENT)
        self.wallet.set_coin_type(CoinType.LEGACY)
        self.assertEqual(self.wallet.get_coin_type(), CoinType.LEGACY)
        self.wallet.set_coin_type(CoinType.CURRENT)
        self.assertEqual(self.wallet.get_coin_type(), CoinType.CURRENT)

    def test_address_creation(self):
        """Test address creation with different coin types"""
        # Test current coin type
        self.wallet.set_coin_type(CoinType.CURRENT)
        address1 = self.wallet.create_address()
        self.assertIn(str(CoinType.CURRENT.value), address1)
        
        # Test legacy coin type
        self.wallet.set_coin_type(CoinType.LEGACY)
        address2 = self.wallet.create_address()
        self.assertIn(str(CoinType.LEGACY.value), address2)

    def test_get_balance(self):
        """Test balance retrieval"""
        # Send initial balance to address2
        self.wallet.send_transaction(
            from_address=self.address1,
            to_address=self.address2,
            amount=self.initial_balance
        )
        
        self.assertEqual(self.wallet.get_balance(self.address1), 0)
        self.assertEqual(self.wallet.get_balance(self.address2), self.initial_balance)

    def test_send_transaction(self):
        """Test transaction sending"""
        # Send half of initial balance to address2
        amount = 500000000000000000  # 0.5 SATOX
        txid = self.wallet.send_transaction(
            from_address=self.address1,
            to_address=self.address2,
            amount=amount
        )
        
        tx = self.wallet.get_transaction(txid)
        self.assertIsNotNone(tx)
        self.assertEqual(tx.sender, self.address1)
        self.assertEqual(tx.recipient, self.address2)
        self.assertEqual(tx.amount, amount)
        self.assertEqual(tx.coin_type, self.wallet.get_coin_type())
        
        # Verify balances
        self.assertEqual(self.wallet.get_balance(self.address1), self.initial_balance - amount)
        self.assertEqual(self.wallet.get_balance(self.address2), amount)

    def test_create_asset(self):
        """Test asset creation"""
        asset = self.wallet.create_asset(
            name="Test Asset",
            type="TOKEN",
            amount=1000000,
            owner=self.address1,
            metadata={"name": "Test Token", "symbol": "TEST"}
        )
        
        self.assertIsNotNone(asset)
        self.assertEqual(asset.name, "Test Asset")
        self.assertEqual(asset.type, "TOKEN")
        self.assertEqual(asset.amount, 1000000)
        self.assertEqual(asset.owner, self.address1)
        self.assertEqual(asset.coin_type, self.wallet.get_coin_type())
        
        # Verify asset balance
        self.assertEqual(self.wallet.get_balance(self.address1, "Test Asset"), 1000000)

    def test_transfer_asset(self):
        """Test asset transfer"""
        # Create asset
        asset = self.wallet.create_asset(
            name="Test Asset",
            type="TOKEN",
            amount=1000000,
            owner=self.address1,
            metadata={"name": "Test Token", "symbol": "TEST"}
        )
        
        # Transfer asset
        transfer_amount = 500000
        self.wallet.transfer_asset(
            name="Test Asset",
            from_owner=self.address1,
            to_owner=self.address2,
            amount=transfer_amount
        )
        
        # Check balances
        self.assertEqual(self.wallet.get_balance(self.address1, "Test Asset"), 500000)
        self.assertEqual(self.wallet.get_balance(self.address2, "Test Asset"), 500000)

    def test_asset_metadata(self):
        """Test asset metadata operations"""
        # Create asset
        asset = self.wallet.create_asset(
            name="Test Asset",
            type="TOKEN",
            amount=1000000,
            owner=self.address1,
            metadata={"name": "Test Token", "symbol": "TEST"}
        )
        
        # Store metadata
        metadata = {"description": "Test Description", "website": "https://test.com"}
        ipfs_hash = self.wallet.store_asset_metadata("Test Asset", metadata)
        
        # Get metadata
        retrieved_metadata = self.wallet.get_asset_metadata(ipfs_hash)
        self.assertIsNotNone(retrieved_metadata)
        self.assertIn("ipfs_hash", retrieved_metadata)

    def test_mine_block(self):
        """Test block mining"""
        # Create and send transaction
        amount = 500000000000000000  # 0.5 SATOX
        txid = self.wallet.send_transaction(
            from_address=self.address1,
            to_address=self.address2,
            amount=amount
        )
        
        # Mine block
        block_height = self.wallet.mine_block()
        self.assertEqual(block_height, 1)
        
        # Check transaction status
        tx = self.wallet.get_transaction(txid)
        self.assertEqual(tx.status, "confirmed")
        self.assertEqual(tx.block_height, 1)

    def test_wallet_export_import(self):
        """Test wallet export and import"""
        # Create some state
        amount = 500000000000000000  # 0.5 SATOX
        txid = self.wallet.send_transaction(
            from_address=self.address1,
            to_address=self.address2,
            amount=amount
        )
        
        asset = self.wallet.create_asset(
            name="Test Asset",
            type="TOKEN",
            amount=1000000,
            owner=self.address1,
            metadata={"name": "Test Token", "symbol": "TEST"}
        )
        
        # Export wallet
        wallet_data = self.wallet.export_wallet()
        
        # Create new wallet and import data
        new_wallet = MockWallet()
        new_wallet.connect()
        new_wallet.import_wallet(wallet_data)
        
        # Verify state
        self.assertEqual(new_wallet.get_block_height(), self.wallet.get_block_height())
        self.assertEqual(len(new_wallet.list_transactions()), len(self.wallet.list_transactions()))
        self.assertEqual(len(new_wallet.list_assets()), len(self.wallet.list_assets()))

def test_performance_metrics():
    """Test wallet performance"""
    wallet = MockWallet()
    wallet.connect()
    
    # Create addresses with initial balance
    address1 = wallet.create_address()
    address2 = wallet.create_address()
    wallet._addresses[address1] = {"SATOX": 1000000000000000000}  # 1 SATOX
    
    # Test address creation performance
    start_time = time.time()
    for _ in range(100):
        wallet.create_address()
    address_creation_time = time.time() - start_time
    assert address_creation_time < 1.0, "Address creation too slow"
    
    # Test transaction performance
    start_time = time.time()
    for _ in range(100):
        wallet.send_transaction(
            from_address=address1,
            to_address=address2,
            amount=1000000
        )
    transaction_time = time.time() - start_time
    assert transaction_time < 1.0, "Transaction processing too slow"
    
    # Test asset operations performance
    start_time = time.time()
    for i in range(100):
        wallet.create_asset(
            name=f"Test Asset {i}",
            type="TOKEN",
            amount=1000000,
            owner=address1,
            metadata={"name": f"Test Token {i}", "symbol": f"TEST{i}"}
        )
    asset_creation_time = time.time() - start_time
    assert asset_creation_time < 1.0, "Asset creation too slow"

if __name__ == '__main__':
    unittest.main() 