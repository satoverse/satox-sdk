import unittest
from satox.wallet.wallet_manager import WalletManager
from satox.core.error_handler import SatoxError
from satox.core.security import SecurityManager
import pytest
import os
import tempfile

class TestTransactionSecurity(unittest.TestCase):
    def setUp(self):
        """Set up test environment."""
        self.wallet = WalletManager()
        self.security = SecurityManager()
        self.temp_dir = tempfile.mkdtemp()
        
    def tearDown(self):
        """Clean up test environment."""
        # Clean up temporary files
        for file in os.listdir(self.temp_dir):
            os.remove(os.path.join(self.temp_dir, file))
        os.rmdir(self.temp_dir)
        
    def test_key_generation(self):
        """Test secure key generation."""
        # Test key generation
        private_key = self.security.generate_private_key()
        public_key = self.security.get_public_key(private_key)
        
        # Verify key properties
        self.assertIsNotNone(private_key)
        self.assertIsNotNone(public_key)
        self.assertNotEqual(private_key, public_key)
        
        # Test key length
        self.assertEqual(len(private_key), 64)  # 32 bytes in hex
        self.assertEqual(len(public_key), 130)  # 65 bytes in hex
        
    def test_key_storage(self):
        """Test secure key storage."""
        # Generate and store key
        private_key = self.security.generate_private_key()
        key_file = os.path.join(self.temp_dir, "test_key.priv")
        
        # Test secure storage
        self.security.store_private_key(private_key, key_file, "test_password")
        
        # Verify file exists and is encrypted
        self.assertTrue(os.path.exists(key_file))
        with open(key_file, 'rb') as f:
            content = f.read()
            self.assertNotEqual(content, private_key.encode())
            
        # Test key recovery
        recovered_key = self.security.load_private_key(key_file, "test_password")
        self.assertEqual(recovered_key, private_key)
        
    def test_transaction_signing(self):
        """Test transaction signing security."""
        # Generate keys
        private_key = self.security.generate_private_key()
        public_key = self.security.get_public_key(private_key)
        
        # Create test transaction
        tx = {
            "from": "test_address",
            "to": "recipient_address",
            "amount": 100,
            "nonce": 1,
            "timestamp": 1234567890
        }
        
        # Sign transaction
        signature = self.security.sign_transaction(tx, private_key)
        
        # Verify signature
        self.assertTrue(self.security.verify_signature(tx, signature, public_key))
        
        # Test signature tampering
        tx["amount"] = 200
        self.assertFalse(self.security.verify_signature(tx, signature, public_key))
        
    def test_key_derivation(self):
        """Test secure key derivation."""
        # Test mnemonic generation
        mnemonic = self.security.generate_mnemonic()
        self.assertIsNotNone(mnemonic)
        self.assertTrue(len(mnemonic.split()) >= 12)
        
        # Test key derivation from mnemonic
        derived_key = self.security.derive_key_from_mnemonic(mnemonic)
        self.assertIsNotNone(derived_key)
        self.assertEqual(len(derived_key), 64)
        
        # Test deterministic derivation
        same_key = self.security.derive_key_from_mnemonic(mnemonic)
        self.assertEqual(derived_key, same_key)
        
    def test_transaction_validation(self):
        """Test transaction validation security."""
        # Create test transaction
        tx = {
            "from": "test_address",
            "to": "recipient_address",
            "amount": 100,
            "nonce": 1,
            "timestamp": 1234567890
        }
        
        # Test valid transaction
        self.assertTrue(self.security.validate_transaction(tx))
        
        # Test invalid amount
        invalid_tx = tx.copy()
        invalid_tx["amount"] = -100
        self.assertFalse(self.security.validate_transaction(invalid_tx))
        
        # Test missing required fields
        for field in ["from", "to", "amount", "nonce", "timestamp"]:
            invalid_tx = tx.copy()
            del invalid_tx[field]
            self.assertFalse(self.security.validate_transaction(invalid_tx))
            
    def test_key_rotation(self):
        """Test secure key rotation."""
        # Generate initial key pair
        old_private_key = self.security.generate_private_key()
        old_public_key = self.security.get_public_key(old_private_key)
        
        # Generate new key pair
        new_private_key = self.security.generate_private_key()
        new_public_key = self.security.get_public_key(new_private_key)
        
        # Test key rotation
        self.security.rotate_key(old_private_key, new_private_key)
        
        # Verify old key is invalidated
        tx = {"test": "data"}
        signature = self.security.sign_transaction(tx, old_private_key)
        self.assertFalse(self.security.verify_signature(tx, signature, old_public_key))
        
        # Verify new key works
        signature = self.security.sign_transaction(tx, new_private_key)
        self.assertTrue(self.security.verify_signature(tx, signature, new_public_key))
        
    def test_quantum_resistance(self):
        """Test quantum-resistant security features."""
        # Test quantum-resistant key generation
        qr_private_key = self.security.generate_quantum_resistant_key()
        qr_public_key = self.security.get_quantum_resistant_public_key(qr_private_key)
        
        # Verify key properties
        self.assertIsNotNone(qr_private_key)
        self.assertIsNotNone(qr_public_key)
        self.assertNotEqual(qr_private_key, qr_public_key)
        
        # Test quantum-resistant signing
        tx = {"test": "data"}
        signature = self.security.sign_quantum_resistant(tx, qr_private_key)
        self.assertTrue(self.security.verify_quantum_resistant_signature(tx, signature, qr_public_key)) 