import unittest
import os
import tempfile
import json
from unittest.mock import Mock, patch
from satox.wallet.wallet import Wallet
from satox.wallet.wallet_manager import WalletManager
from satox.core.exceptions import WalletError, SecurityError

class TestWalletSecurity(unittest.TestCase):
    def setUp(self):
        self.temp_dir = tempfile.mkdtemp()
        self.config = {
            "wallet": {
                "security": {
                    "encryption": {
                        "enabled": True,
                        "algorithm": "AES-256-GCM",
                        "key_derivation": "PBKDF2",
                        "iterations": 100000
                    },
                    "key_management": {
                        "rotation": {
                            "enabled": True,
                            "interval": 30
                        },
                        "backup": {
                            "enabled": True,
                            "location": self.temp_dir
                        }
                    },
                    "authentication": {
                        "max_attempts": 3,
                        "lockout_duration": 300
                    },
                    "validation": {
                        "address_format": True,
                        "checksum": True
                    }
                }
            }
        }
        self.wallet_manager = WalletManager(self.config)
        self.wallet = self.wallet_manager.create_wallet("test_wallet", "test_password")

    def tearDown(self):
        self.wallet_manager.delete_wallet("test_wallet")
        os.rmdir(self.temp_dir)

    def test_wallet_creation_security(self):
        """Test wallet creation security measures"""
        # Test wallet creation with strong password
        wallet = self.wallet_manager.create_wallet("secure_wallet", "StrongP@ssw0rd123!")
        self.assertIsNotNone(wallet)
        self.assertTrue(wallet.is_encrypted())

        # Test wallet creation with weak password
        with self.assertRaises(SecurityError):
            self.wallet_manager.create_wallet("insecure_wallet", "weak")

        # Test duplicate wallet creation
        with self.assertRaises(WalletError):
            self.wallet_manager.create_wallet("test_wallet", "test_password")

    def test_wallet_encryption(self):
        """Test wallet encryption and decryption"""
        # Test encryption
        self.assertTrue(self.wallet.is_encrypted())
        
        # Test decryption with correct password
        self.wallet.unlock("test_password")
        self.assertTrue(self.wallet.is_unlocked())
        
        # Test decryption with incorrect password
        with self.assertRaises(SecurityError):
            self.wallet.unlock("wrong_password")

    def test_key_management(self):
        """Test key management security"""
        # Test key generation
        key = self.wallet.generate_key()
        self.assertIsNotNone(key)
        self.assertTrue(self.wallet.has_key(key))

        # Test key backup
        backup = self.wallet.backup_keys()
        self.assertIsNotNone(backup)
        self.assertTrue(os.path.exists(backup))

        # Test key restoration
        self.wallet.delete_key(key)
        self.assertFalse(self.wallet.has_key(key))
        self.wallet.restore_keys(backup)
        self.assertTrue(self.wallet.has_key(key))

    def test_address_validation(self):
        """Test address validation security"""
        # Test valid address
        address = self.wallet.generate_address()
        self.assertTrue(self.wallet.validate_address(address))

        # Test invalid address
        invalid_address = "invalid_address"
        self.assertFalse(self.wallet.validate_address(invalid_address))

    def test_transaction_security(self):
        """Test transaction security measures"""
        # Test transaction signing
        transaction = {
            "from": self.wallet.get_address(),
            "to": "recipient_address",
            "amount": 100,
            "timestamp": 1234567890
        }
        signed_tx = self.wallet.sign_transaction(transaction)
        self.assertIsNotNone(signed_tx)
        self.assertTrue(self.wallet.verify_transaction(signed_tx))

        # Test transaction verification
        self.assertTrue(self.wallet.verify_transaction(signed_tx))

        # Test invalid transaction
        invalid_tx = signed_tx.copy()
        invalid_tx["amount"] = 200
        self.assertFalse(self.wallet.verify_transaction(invalid_tx))

    def test_authentication_security(self):
        """Test wallet authentication security"""
        # Test successful authentication
        self.assertTrue(self.wallet.authenticate("test_password"))

        # Test failed authentication
        self.assertFalse(self.wallet.authenticate("wrong_password"))

        # Test lockout after max attempts
        for _ in range(3):
            self.wallet.authenticate("wrong_password")
        self.assertTrue(self.wallet.is_locked())

    def test_backup_security(self):
        """Test wallet backup security"""
        # Test backup creation
        backup = self.wallet.create_backup()
        self.assertIsNotNone(backup)
        self.assertTrue(os.path.exists(backup))

        # Test backup encryption
        self.assertTrue(self.wallet.is_backup_encrypted(backup))

        # Test backup restoration
        new_wallet = self.wallet_manager.restore_wallet(backup, "test_password")
        self.assertIsNotNone(new_wallet)
        self.assertEqual(new_wallet.get_address(), self.wallet.get_address())

    def test_key_rotation(self):
        """Test key rotation security"""
        # Test key rotation
        old_key = self.wallet.get_active_key()
        self.wallet.rotate_key()
        new_key = self.wallet.get_active_key()
        self.assertNotEqual(old_key, new_key)

        # Test key rotation interval
        self.wallet.rotate_key()
        self.assertGreaterEqual(
            self.wallet.get_key_rotation_time(),
            self.wallet.get_last_rotation_time()
        )

    def test_wallet_deletion(self):
        """Test wallet deletion security"""
        # Test secure deletion
        self.wallet_manager.delete_wallet("test_wallet")
        self.assertFalse(self.wallet_manager.wallet_exists("test_wallet"))

        # Test deletion of non-existent wallet
        with self.assertRaises(WalletError):
            self.wallet_manager.delete_wallet("non_existent_wallet")

    def test_wallet_import_export(self):
        """Test wallet import/export security"""
        # Test wallet export
        exported = self.wallet.export()
        self.assertIsNotNone(exported)
        self.assertTrue(self.wallet.is_export_encrypted(exported))

        # Test wallet import
        imported = self.wallet_manager.import_wallet(exported, "test_password")
        self.assertIsNotNone(imported)
        self.assertEqual(imported.get_address(), self.wallet.get_address())

    def test_wallet_recovery(self):
        """Test wallet recovery security"""
        # Test recovery phrase generation
        recovery_phrase = self.wallet.generate_recovery_phrase()
        self.assertIsNotNone(recovery_phrase)
        self.assertEqual(len(recovery_phrase.split()), 24)

        # Test wallet recovery
        recovered = self.wallet_manager.recover_wallet(recovery_phrase, "new_password")
        self.assertIsNotNone(recovered)
        self.assertEqual(recovered.get_address(), self.wallet.get_address())

    def test_wallet_permissions(self):
        """Test wallet permissions security"""
        # Test permission assignment
        self.wallet.set_permission("admin", True)
        self.assertTrue(self.wallet.has_permission("admin"))

        # Test permission removal
        self.wallet.remove_permission("admin")
        self.assertFalse(self.wallet.has_permission("admin"))

    def test_wallet_audit(self):
        """Test wallet audit logging"""
        # Test audit log creation
        self.wallet.log_audit("test_action", "test_details")
        audit_log = self.wallet.get_audit_log()
        self.assertIsNotNone(audit_log)
        self.assertTrue(len(audit_log) > 0)

        # Test audit log encryption
        self.assertTrue(self.wallet.is_audit_log_encrypted())

    def test_wallet_monitoring(self):
        """Test wallet monitoring security"""
        # Test balance monitoring
        self.wallet.enable_balance_monitoring()
        self.assertTrue(self.wallet.is_balance_monitoring_enabled())

        # Test transaction monitoring
        self.wallet.enable_transaction_monitoring()
        self.assertTrue(self.wallet.is_transaction_monitoring_enabled())

    def test_wallet_quarantine(self):
        """Test wallet quarantine security"""
        # Test wallet quarantine
        self.wallet.quarantine("suspicious_activity")
        self.assertTrue(self.wallet.is_quarantined())

        # Test wallet unquarantine
        self.wallet.unquarantine()
        self.assertFalse(self.wallet.is_quarantined())

    def test_wallet_validation(self):
        """Test wallet validation security"""
        # Test wallet validation
        self.assertTrue(self.wallet.validate())

        # Test invalid wallet
        self.wallet.corrupt()
        self.assertFalse(self.wallet.validate())

    def test_wallet_recovery_mechanism(self):
        """Test wallet recovery mechanism security"""
        # Test recovery mechanism
        self.wallet.enable_recovery_mechanism()
        self.assertTrue(self.wallet.is_recovery_mechanism_enabled())

        # Test recovery trigger
        self.wallet.trigger_recovery()
        self.assertTrue(self.wallet.is_recovery_triggered())

    def test_wallet_security_policy(self):
        """Test wallet security policy"""
        # Test security policy
        policy = self.wallet.get_security_policy()
        self.assertIsNotNone(policy)
        self.assertTrue(policy["encryption_enabled"])
        self.assertTrue(policy["key_rotation_enabled"])
        self.assertTrue(policy["backup_enabled"])

        # Test policy update
        self.wallet.update_security_policy({"encryption_enabled": False})
        policy = self.wallet.get_security_policy()
        self.assertFalse(policy["encryption_enabled"])

    def test_wallet_error_handling(self):
        """Test wallet error handling security"""
        # Test error handling
        with self.assertRaises(WalletError):
            self.wallet.authenticate("wrong_password")

        # Test error logging
        self.wallet.log_error("test_error", "test_details")
        error_log = self.wallet.get_error_log()
        self.assertIsNotNone(error_log)
        self.assertTrue(len(error_log) > 0)

    def test_wallet_performance(self):
        """Test wallet performance security"""
        # Test performance monitoring
        self.wallet.enable_performance_monitoring()
        self.assertTrue(self.wallet.is_performance_monitoring_enabled())

        # Test performance metrics
        metrics = self.wallet.get_performance_metrics()
        self.assertIsNotNone(metrics)
        self.assertTrue("response_time" in metrics)
        self.assertTrue("memory_usage" in metrics)

    def test_wallet_resource_management(self):
        """Test wallet resource management security"""
        # Test resource limits
        self.wallet.set_resource_limits({"memory": 1024, "cpu": 50})
        limits = self.wallet.get_resource_limits()
        self.assertEqual(limits["memory"], 1024)
        self.assertEqual(limits["cpu"], 50)

        # Test resource usage
        usage = self.wallet.get_resource_usage()
        self.assertIsNotNone(usage)
        self.assertTrue("memory" in usage)
        self.assertTrue("cpu" in usage)

    def test_wallet_cleanup(self):
        """Test wallet cleanup security"""
        # Test cleanup
        self.wallet.cleanup()
        self.assertTrue(self.wallet.is_clean())

        # Test cleanup verification
        self.assertTrue(self.wallet.verify_cleanup())

if __name__ == "__main__":
    unittest.main() 