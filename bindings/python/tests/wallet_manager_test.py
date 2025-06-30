"""
Satox SDK Wallet Manager Tests
"""

import unittest
from satox.wallet.wallet_manager import WalletManager, WalletConfig, NetworkType, Wallet

class TestWalletManager(unittest.TestCase):
    """Test cases for Wallet Manager"""

    def setUp(self):
        """Set up test environment"""
        self.manager = WalletManager()
        self.manager.initialize()

    def test_initialize(self):
        """Test Wallet Manager initialization"""
        # Test default initialization
        manager = WalletManager()
        self.assertFalse(manager.initialized)
        
        manager.initialize()
        self.assertTrue(manager.initialized)

        # Test custom configuration
        config = WalletConfig(
            version="2.0.0",
            min_password_length=10,
            max_wallet_name_length=40,
            min_wallet_name_length=5
        )
        manager = WalletManager()
        manager.initialize(config)
        self.assertEqual(manager.config.version, "2.0.0")
        self.assertEqual(manager.config.min_password_length, 10)
        self.assertEqual(manager.config.max_wallet_name_length, 40)
        self.assertEqual(manager.config.min_wallet_name_length, 5)

        # Test double initialization
        with self.assertRaises(RuntimeError):
            manager.initialize()

    def test_create_wallet(self):
        """Test wallet creation"""
        # Test valid wallet creation
        wallet = self.manager.create_wallet("test_wallet", "password123")
        self.assertEqual(wallet.name, "test_wallet")
        self.assertEqual(wallet.network, NetworkType.MAINNET)
        self.assertTrue(wallet.private_key.startswith("0x"))
        self.assertTrue(wallet.public_key.startswith("0x"))
        self.assertTrue(wallet.address.startswith("0x"))

        # Test duplicate wallet
        with self.assertRaises(RuntimeError):
            self.manager.create_wallet("test_wallet", "password123")

        # Test invalid wallet name
        with self.assertRaises(ValueError):
            self.manager.create_wallet("test@wallet", "password123")

        # Test invalid password
        with self.assertRaises(ValueError):
            self.manager.create_wallet("test_wallet2", "short")

    def test_import_wallet(self):
        """Test wallet import"""
        # Test valid wallet import
        private_key = "0x" + "1" * 64
        wallet = self.manager.import_wallet("imported_wallet", private_key, "password123")
        self.assertEqual(wallet.name, "imported_wallet")
        self.assertEqual(wallet.private_key, private_key)
        self.assertEqual(wallet.network, NetworkType.MAINNET)

        # Test duplicate wallet
        with self.assertRaises(RuntimeError):
            self.manager.import_wallet("imported_wallet", private_key, "password123")

        # Test invalid private key
        with self.assertRaises(ValueError):
            self.manager.import_wallet("invalid_wallet", "invalid_key", "password123")

    def test_get_wallet(self):
        """Test wallet retrieval"""
        # Create test wallet
        self.manager.create_wallet("test_wallet", "password123")

        # Test valid wallet retrieval
        wallet = self.manager.get_wallet("test_wallet")
        self.assertEqual(wallet.name, "test_wallet")

        # Test non-existent wallet
        with self.assertRaises(RuntimeError):
            self.manager.get_wallet("non_existent")

    def test_list_wallets(self):
        """Test wallet listing"""
        # Create test wallets
        self.manager.create_wallet("wallet1", "password123")
        self.manager.create_wallet("wallet2", "password123")

        # Test wallet listing
        wallets = self.manager.list_wallets()
        self.assertEqual(len(wallets), 2)
        self.assertIn("wallet1", wallets)
        self.assertIn("wallet2", wallets)

    def test_delete_wallet(self):
        """Test wallet deletion"""
        # Create test wallet
        self.manager.create_wallet("test_wallet", "password123")

        # Test valid wallet deletion
        self.manager.delete_wallet("test_wallet", "password123")
        with self.assertRaises(RuntimeError):
            self.manager.get_wallet("test_wallet")

        # Test non-existent wallet deletion
        with self.assertRaises(RuntimeError):
            self.manager.delete_wallet("non_existent", "password123")

    def test_sign_transaction(self):
        """Test transaction signing"""
        # Create test wallet
        wallet = self.manager.create_wallet("test_wallet", "password123")

        # Test transaction signing
        transaction = {"data": "test"}
        signature = self.manager.sign_transaction("test_wallet", transaction, "password123")
        self.assertTrue(signature.startswith("0x"))

        # Test non-existent wallet
        with self.assertRaises(RuntimeError):
            self.manager.sign_transaction("non_existent", transaction, "password123")

    def test_verify_signature(self):
        """Test signature verification"""
        # Create test wallet
        wallet = self.manager.create_wallet("test_wallet", "password123")

        # Test signature verification
        transaction = {"data": "test"}
        signature = self.manager.sign_transaction("test_wallet", transaction, "password123")
        self.assertTrue(self.manager.verify_signature(transaction, signature, wallet.public_key))

        # Test invalid signature
        self.assertFalse(self.manager.verify_signature(transaction, "0xinvalid", wallet.public_key))

    def test_change_password(self):
        """Test password change"""
        # Create test wallet
        self.manager.create_wallet("test_wallet", "password123")

        # Test password change
        self.manager.change_password("test_wallet", "password123", "new_password123")

        # Test non-existent wallet
        with self.assertRaises(RuntimeError):
            self.manager.change_password("non_existent", "password123", "new_password123")

    def test_backup_wallet(self):
        """Test wallet backup"""
        # Create test wallet
        self.manager.create_wallet("test_wallet", "password123")

        # Test wallet backup
        backup = self.manager.backup_wallet("test_wallet", "password123")
        self.assertEqual(backup.name, "test_wallet")
        self.assertTrue(isinstance(backup, Wallet))

        # Test non-existent wallet
        with self.assertRaises(RuntimeError):
            self.manager.backup_wallet("non_existent", "password123")

    def test_restore_wallet(self):
        """Test wallet restoration"""
        # Create and backup test wallet
        self.manager.create_wallet("test_wallet", "password123")
        backup = self.manager.backup_wallet("test_wallet", "password123")

        # Delete original wallet
        self.manager.delete_wallet("test_wallet", "password123")

        # Test wallet restoration
        restored = self.manager.restore_wallet(backup, "password123")
        self.assertEqual(restored.name, "test_wallet")
        self.assertEqual(restored.private_key, backup.private_key)

        # Test duplicate wallet
        with self.assertRaises(RuntimeError):
            self.manager.restore_wallet(backup, "password123")

    def test_export_wallet(self):
        """Test wallet export"""
        # Create test wallet
        self.manager.create_wallet("test_wallet", "password123")

        # Test wallet export
        exported = self.manager.export_wallet("test_wallet", "password123")
        self.assertEqual(exported.name, "test_wallet")
        self.assertTrue(isinstance(exported, Wallet))

        # Test non-existent wallet
        with self.assertRaises(RuntimeError):
            self.manager.export_wallet("non_existent", "password123")

if __name__ == "__main__":
    unittest.main() 