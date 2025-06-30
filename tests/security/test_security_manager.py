import unittest
import os
import tempfile
from satox.security.security_manager import SecurityManager
from satox.core.error_handler import SatoxError

class TestSecurityManager(unittest.TestCase):
    """Test cases for SecurityManager."""
    
    def setUp(self):
        """Set up test environment."""
        self.temp_dir = tempfile.mkdtemp()
        self.config = {
            "audit_logging": True,
            "encryption": {
                "algorithm": "AES-256-GCM",
                "signature": "RSA-4096"
            },
            "rate_limiting": {
                "enabled": True,
                "max_requests": 100,
                "window_seconds": 60
            },
            "authentication": {
                "required": True,
                "min_password_length": 12
            },
            "backup": {
                "encrypted": True,
                "max_backups": 5
            }
        }
        self.security_manager = SecurityManager(self.config)
        
    def tearDown(self):
        """Clean up test environment."""
        if os.path.exists("master.key"):
            os.remove("master.key")
        os.rmdir(self.temp_dir)
        
    def test_initialization(self):
        """Test SecurityManager initialization."""
        self.assertIsNotNone(self.security_manager)
        self.assertIsNotNone(self.security_manager.master_key)
        self.assertIsNotNone(self.security_manager.fernet)
        self.assertIsNotNone(self.security_manager.private_key)
        self.assertIsNotNone(self.security_manager.public_key)
        
    def test_encryption_decryption(self):
        """Test data encryption and decryption."""
        test_data = b"Test data for encryption"
        encrypted_data = self.security_manager.encrypt_data(test_data)
        decrypted_data = self.security_manager.decrypt_data(encrypted_data)
        self.assertEqual(test_data, decrypted_data)
        
    def test_key_pair_generation(self):
        """Test RSA key pair generation."""
        key_pair = self.security_manager.generate_key_pair()
        self.assertIn("private_key", key_pair)
        self.assertIn("public_key", key_pair)
        self.assertIsInstance(key_pair["private_key"], bytes)
        self.assertIsInstance(key_pair["public_key"], bytes)
        
    def test_signing_verification(self):
        """Test data signing and verification."""
        test_data = b"Test data for signing"
        key_pair = self.security_manager.generate_key_pair()
        signature = self.security_manager.sign_data(test_data, key_pair["private_key"])
        is_valid = self.security_manager.verify_signature(
            test_data,
            signature,
            key_pair["public_key"]
        )
        self.assertTrue(is_valid)
        
    def test_quantum_resistant_key(self):
        """Test quantum-resistant key generation."""
        key = self.security_manager.generate_quantum_resistant_key()
        self.assertIsInstance(key, bytes)
        self.assertEqual(len(key), 32)
        
    def test_key_rotation(self):
        """Test key rotation."""
        old_master_key = self.security_manager.master_key
        self.security_manager.rotate_keys()
        new_master_key = self.security_manager.master_key
        self.assertNotEqual(old_master_key, new_master_key)
        
    def test_security_policies(self):
        """Test security policies."""
        policies = self.security_manager.get_security_policies()
        self.assertIn("min_key_length", policies)
        self.assertIn("min_password_length", policies)
        self.assertIn("require_special_chars", policies)
        self.assertIn("require_numbers", policies)
        self.assertIn("require_uppercase", policies)
        self.assertIn("require_lowercase", policies)
        
    def test_policy_update(self):
        """Test security policy updates."""
        new_policies = {
            "min_password_length": 16,
            "require_special_chars": True,
            "require_numbers": True,
            "require_uppercase": True,
            "require_lowercase": True,
            "max_login_attempts": 5,
            "session_timeout": 7200,
            "password_expiry": 60,
            "encryption_algorithm": "AES-256-GCM",
            "hash_algorithm": "SHA-512",
            "key_rotation_period": 15
        }
        self.security_manager.update_security_policies(new_policies)
        updated_policies = self.security_manager.get_security_policies()
        self.assertEqual(updated_policies["min_password_length"], 16)
        self.assertEqual(updated_policies["session_timeout"], 7200)
        
    def test_password_validation(self):
        """Test password validation."""
        valid_password = "Test@123Password"
        invalid_password = "weak"
        
        self.assertTrue(self.security_manager.validate_password(valid_password))
        self.assertFalse(self.security_manager.validate_password(invalid_password))
        
    def test_password_hashing(self):
        """Test password hashing and verification."""
        password = "Test@123Password"
        hashed_password = self.security_manager.hash_password(password)
        self.assertIsInstance(hashed_password, bytes)
        self.assertTrue(self.security_manager.verify_password(password, hashed_password))
        self.assertFalse(self.security_manager.verify_password("WrongPassword", hashed_password))
        
    def test_session_token(self):
        """Test session token generation and validation."""
        token = self.security_manager.generate_session_token()
        self.assertIsInstance(token, str)
        self.assertTrue(self.security_manager.validate_session_token(token))
        self.assertFalse(self.security_manager.validate_session_token("invalid_token"))
        
    def test_invalid_config(self):
        """Test initialization with invalid configuration."""
        invalid_config = {"invalid_key": "invalid_value"}
        with self.assertRaises(SatoxError):
            SecurityManager(invalid_config)
            
    def test_missing_policy_field(self):
        """Test policy update with missing required field."""
        invalid_policies = {
            "min_password_length": 16,
            "require_special_chars": True
        }
        with self.assertRaises(SatoxError):
            self.security_manager.update_security_policies(invalid_policies)
            
    def test_encryption_error(self):
        """Test encryption with invalid data."""
        with self.assertRaises(SatoxError):
            self.security_manager.encrypt_data(None)
            
    def test_decryption_error(self):
        """Test decryption with invalid data."""
        with self.assertRaises(SatoxError):
            self.security_manager.decrypt_data(b"invalid_encrypted_data")
            
    def test_signing_error(self):
        """Test signing with invalid data."""
        with self.assertRaises(SatoxError):
            self.security_manager.sign_data(None, b"invalid_key")
            
    def test_verification_error(self):
        """Test verification with invalid data."""
        with self.assertRaises(SatoxError):
            self.security_manager.verify_signature(None, b"invalid_signature", b"invalid_key")
            
    def test_key_rotation_error(self):
        """Test key rotation with invalid state."""
        self.security_manager.master_key = None
        with self.assertRaises(SatoxError):
            self.security_manager.rotate_keys()
            
    def test_password_validation_error(self):
        """Test password validation with invalid input."""
        with self.assertRaises(SatoxError):
            self.security_manager.validate_password(None)
            
    def test_password_hashing_error(self):
        """Test password hashing with invalid input."""
        with self.assertRaises(SatoxError):
            self.security_manager.hash_password(None)
            
    def test_password_verification_error(self):
        """Test password verification with invalid input."""
        with self.assertRaises(SatoxError):
            self.security_manager.verify_password(None, b"invalid_hash")
            
    def test_session_token_error(self):
        """Test session token validation with invalid input."""
        with self.assertRaises(SatoxError):
            self.security_manager.validate_session_token(None)
            
if __name__ == "__main__":
    unittest.main() 