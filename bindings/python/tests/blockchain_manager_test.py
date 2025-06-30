"""
Satox SDK Blockchain Manager Tests
"""

import unittest
from datetime import datetime, timedelta
import pyotp
from satox.blockchain.blockchain_manager import (
    BlockchainManager,
    BlockchainConfig,
    NetworkType,
    BlockStatus,
    TransactionStatus,
    Block,
    Transaction,
    LoginCredentials,
    LoginSession,
    TwoFactorSetup
)

class TestBlockchainManager(unittest.TestCase):
    """Test cases for Blockchain Manager"""

    def setUp(self):
        """Set up test environment"""
        self.manager = BlockchainManager()
        self.manager.initialize()
        self.test_address = "0x1234567890abcdef1234567890abcdef12345678"
        self.test_block_hash = "0" * 64
        self.test_tx_hash = "1" * 64
        self.test_username = "testuser"
        self.test_password = "testpassword123"
        self.test_api_key = "test_api_key_123"

    def test_initialize(self):
        """Test Blockchain Manager initialization"""
        # Test default initialization
        manager = BlockchainManager()
        self.assertFalse(manager.initialized)
        
        manager.initialize()
        self.assertTrue(manager.initialized)

        # Test custom configuration
        config = BlockchainConfig(
            version="2.0.0",
            network=NetworkType.TESTNET,
            min_confirmations=3,
            max_confirmations=50,
            block_time=30,
            max_block_size=4 * 1024 * 1024,  # 4MB
            max_transaction_size=200 * 1024,  # 200KB
            min_transaction_fee=0.0002,
            max_transaction_fee=2.0,
            min_block_reward=0.0,
            max_block_reward=25.0,
            halving_interval=105000,  # blocks
            initial_supply=0.0,
            max_supply=10500000.0
        )
        manager = BlockchainManager()
        manager.initialize(config)
        self.assertEqual(manager.config.version, "2.0.0")
        self.assertEqual(manager.config.network, NetworkType.TESTNET)
        self.assertEqual(manager.config.min_confirmations, 3)
        self.assertEqual(manager.config.max_confirmations, 50)
        self.assertEqual(manager.config.block_time, 30)
        self.assertEqual(manager.config.max_block_size, 4 * 1024 * 1024)
        self.assertEqual(manager.config.max_transaction_size, 200 * 1024)
        self.assertEqual(manager.config.min_transaction_fee, 0.0002)
        self.assertEqual(manager.config.max_transaction_fee, 2.0)
        self.assertEqual(manager.config.min_block_reward, 0.0)
        self.assertEqual(manager.config.max_block_reward, 25.0)
        self.assertEqual(manager.config.halving_interval, 105000)
        self.assertEqual(manager.config.initial_supply, 0.0)
        self.assertEqual(manager.config.max_supply, 10500000.0)

        # Test double initialization
        with self.assertRaises(RuntimeError):
            manager.initialize()

    def test_get_block(self):
        """Test block retrieval"""
        # Test invalid block hash format
        with self.assertRaises(ValueError):
            self.manager.get_block("invalid_hash")

        # Test non-existent block
        with self.assertRaises(RuntimeError):
            self.manager.get_block(self.test_block_hash)

    def test_get_block_by_height(self):
        """Test block retrieval by height"""
        # Test invalid height type
        with self.assertRaises(ValueError):
            self.manager.get_block_by_height("invalid_height")

        # Test negative height
        with self.assertRaises(ValueError):
            self.manager.get_block_by_height(-1)

        # Test height exceeding current height
        with self.assertRaises(RuntimeError):
            self.manager.get_block_by_height(1000)

    def test_get_latest_block(self):
        """Test latest block retrieval"""
        # Test when no blocks available
        with self.assertRaises(RuntimeError):
            self.manager.get_latest_block()

    def test_get_block_range(self):
        """Test block range retrieval"""
        # Test invalid height types
        with self.assertRaises(ValueError):
            self.manager.get_block_range("invalid_start", 100)

        with self.assertRaises(ValueError):
            self.manager.get_block_range(0, "invalid_end")

        # Test negative heights
        with self.assertRaises(ValueError):
            self.manager.get_block_range(-1, 100)

        with self.assertRaises(ValueError):
            self.manager.get_block_range(0, -1)

        # Test invalid range
        with self.assertRaises(ValueError):
            self.manager.get_block_range(100, 0)

        # Test range exceeding current height
        with self.assertRaises(RuntimeError):
            self.manager.get_block_range(0, 1000)

    def test_get_transaction(self):
        """Test transaction retrieval"""
        # Test invalid transaction hash format
        with self.assertRaises(ValueError):
            self.manager.get_transaction("invalid_hash")

        # Test non-existent transaction
        with self.assertRaises(RuntimeError):
            self.manager.get_transaction(self.test_tx_hash)

    def test_get_transaction_status(self):
        """Test transaction status retrieval"""
        # Test invalid transaction hash format
        with self.assertRaises(ValueError):
            self.manager.get_transaction_status("invalid_hash")

        # Test non-existent transaction
        with self.assertRaises(RuntimeError):
            self.manager.get_transaction_status(self.test_tx_hash)

    def test_get_transaction_confirmations(self):
        """Test transaction confirmations retrieval"""
        # Test invalid transaction hash format
        with self.assertRaises(ValueError):
            self.manager.get_transaction_confirmations("invalid_hash")

        # Test non-existent transaction
        with self.assertRaises(RuntimeError):
            self.manager.get_transaction_confirmations(self.test_tx_hash)

    def test_get_balance(self):
        """Test balance retrieval"""
        # Test invalid address format
        with self.assertRaises(ValueError):
            self.manager.get_balance("invalid_address")

        # Test valid address
        balance = self.manager.get_balance(self.test_address)
        self.assertEqual(balance, 0.0)  # Placeholder value

    def test_get_transaction_history(self):
        """Test transaction history retrieval"""
        # Test invalid address format
        with self.assertRaises(ValueError):
            self.manager.get_transaction_history("invalid_address")

        # Test valid address
        history = self.manager.get_transaction_history(self.test_address)
        self.assertEqual(len(history), 0)  # Placeholder value

    def test_estimate_transaction_fee(self):
        """Test transaction fee estimation"""
        # Test invalid size type
        with self.assertRaises(ValueError):
            self.manager.estimate_transaction_fee("invalid_size")

        # Test negative size
        with self.assertRaises(ValueError):
            self.manager.estimate_transaction_fee(-1)

        # Test size exceeding maximum
        with self.assertRaises(ValueError):
            self.manager.estimate_transaction_fee(self.manager.config.max_transaction_size + 1)

        # Test valid size
        fee = self.manager.estimate_transaction_fee(1000)
        self.assertGreaterEqual(fee, self.manager.config.min_transaction_fee)
        self.assertLessEqual(fee, self.manager.config.max_transaction_fee)

    def test_broadcast_transaction(self):
        """Test transaction broadcasting"""
        # Test invalid transaction type
        with self.assertRaises(ValueError):
            self.manager.broadcast_transaction("invalid_transaction")

        # Test missing required fields
        with self.assertRaises(ValueError):
            self.manager.broadcast_transaction({})

        # Test transaction exceeding maximum size
        large_tx = {
            "version": 1,
            "inputs": [{"txid": "0" * 64, "vout": 0}] * 1000,
            "outputs": [{"address": self.test_address, "value": 1.0}] * 1000,
            "locktime": 0
        }
        with self.assertRaises(ValueError):
            self.manager.broadcast_transaction(large_tx)

        # Test valid transaction
        valid_tx = {
            "version": 1,
            "inputs": [{"txid": "0" * 64, "vout": 0}],
            "outputs": [{"address": self.test_address, "value": 1.0}],
            "locktime": 0
        }
        tx_hash = self.manager.broadcast_transaction(valid_tx)
        self.assertEqual(len(tx_hash), 64)

    def test_get_network_status(self):
        """Test network status retrieval"""
        status = self.manager.get_network_status()
        self.assertEqual(status["network"], self.manager.config.network.value)
        self.assertEqual(status["height"], self.manager._current_height)
        self.assertEqual(status["difficulty"], self.manager._current_difficulty)
        self.assertEqual(status["block_time"], self.manager.config.block_time)
        self.assertEqual(status["version"], self.manager.config.version)

    def test_get_block_reward(self):
        """Test block reward calculation"""
        # Test invalid height type
        with self.assertRaises(ValueError):
            self.manager.get_block_reward("invalid_height")

        # Test negative height
        with self.assertRaises(ValueError):
            self.manager.get_block_reward(-1)

        # Test valid height
        reward = self.manager.get_block_reward(0)
        self.assertEqual(reward, self.manager.config.max_block_reward)

        # Test halving
        reward = self.manager.get_block_reward(self.manager.config.halving_interval)
        self.assertEqual(reward, self.manager.config.max_block_reward / 2)

    def test_get_total_supply(self):
        """Test total supply retrieval"""
        supply = self.manager.get_total_supply()
        self.assertEqual(supply, 0.0)  # Placeholder value

    def test_get_circulating_supply(self):
        """Test circulating supply retrieval"""
        supply = self.manager.get_circulating_supply()
        self.assertEqual(supply, 0.0)  # Placeholder value

    def test_get_next_halving(self):
        """Test next halving information retrieval"""
        halving_info = self.manager.get_next_halving()
        self.assertIn("next_halving_height", halving_info)
        self.assertIn("blocks_until_halving", halving_info)
        self.assertIn("estimated_time", halving_info)
        self.assertIn("current_reward", halving_info)
        self.assertIn("next_reward", halving_info)

    def test_register_user(self):
        """Test user registration"""
        # Test valid registration
        self.manager.register_user(self.test_username, self.test_password, self.test_api_key)
        self.assertIn(self.test_username, self.manager._user_credentials)

        # Test duplicate username
        with self.assertRaises(ValueError):
            self.manager.register_user(self.test_username, "another_password")

        # Test invalid username
        with self.assertRaises(ValueError):
            self.manager.register_user("", self.test_password)

        # Test invalid password
        with self.assertRaises(ValueError):
            self.manager.register_user("newuser", "short")

    def test_login(self):
        """Test user login"""
        # Register test user
        self.manager.register_user(self.test_username, self.test_password, self.test_api_key)

        # Test valid login
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password,
            api_key=self.test_api_key
        )
        session = self.manager.login(credentials)
        self.assertIsInstance(session, LoginSession)
        self.assertEqual(session.username, self.test_username)
        self.assertIn(session.session_id, self.manager._active_sessions)

        # Test invalid username
        with self.assertRaises(ValueError):
            self.manager.login(LoginCredentials(
                username="nonexistent",
                password=self.test_password
            ))

        # Test invalid password
        with self.assertRaises(ValueError):
            self.manager.login(LoginCredentials(
                username=self.test_username,
                password="wrongpassword"
            ))

        # Test invalid API key
        with self.assertRaises(ValueError):
            self.manager.login(LoginCredentials(
                username=self.test_username,
                password=self.test_password,
                api_key="wrong_api_key"
            ))

    def test_logout(self):
        """Test user logout"""
        # Register and login test user
        self.manager.register_user(self.test_username, self.test_password)
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password
        )
        session = self.manager.login(credentials)

        # Test valid logout
        self.manager.logout(session.session_id)
        self.assertNotIn(session.session_id, self.manager._active_sessions)

        # Test invalid session
        with self.assertRaises(ValueError):
            self.manager.logout("invalid_session")

    def test_refresh_session(self):
        """Test session refresh"""
        # Register and login test user
        self.manager.register_user(self.test_username, self.test_password)
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password
        )
        session = self.manager.login(credentials)

        # Test valid refresh
        new_session = self.manager.refresh_session(session.refresh_token)
        self.assertIsInstance(new_session, LoginSession)
        self.assertEqual(new_session.username, self.test_username)
        self.assertNotEqual(new_session.token, session.token)
        self.assertNotEqual(new_session.refresh_token, session.refresh_token)

        # Test invalid refresh token
        with self.assertRaises(ValueError):
            self.manager.refresh_session("invalid_token")

    def test_validate_session(self):
        """Test session validation"""
        # Register and login test user
        self.manager.register_user(self.test_username, self.test_password)
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password
        )
        session = self.manager.login(credentials)

        # Test valid session
        validated_session = self.manager.validate_session(session.token)
        self.assertEqual(validated_session.session_id, session.session_id)

        # Test invalid token
        with self.assertRaises(ValueError):
            self.manager.validate_session("invalid_token")

    def test_update_permissions(self):
        """Test permission updates"""
        # Register and login test user
        self.manager.register_user(self.test_username, self.test_password)
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password
        )
        session = self.manager.login(credentials)

        # Test valid permission update
        new_permissions = ["read", "write", "admin"]
        self.manager.update_permissions(session.session_id, new_permissions)
        updated_session = self.manager.validate_session(session.token)
        self.assertEqual(updated_session.permissions, new_permissions)

        # Test invalid session
        with self.assertRaises(ValueError):
            self.manager.update_permissions("invalid_session", new_permissions)

        # Test invalid permissions
        with self.assertRaises(ValueError):
            self.manager.update_permissions(session.session_id, "not_a_list")

    def test_change_password(self):
        """Test password change"""
        # Register and login test user
        self.manager.register_user(self.test_username, self.test_password)
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password
        )
        session = self.manager.login(credentials)

        # Test valid password change
        new_password = "newpassword123"
        self.manager.change_password(session.session_id, self.test_password, new_password)

        # Verify new password works
        new_credentials = LoginCredentials(
            username=self.test_username,
            password=new_password
        )
        new_session = self.manager.login(new_credentials)
        self.assertIsInstance(new_session, LoginSession)

        # Test invalid old password
        with self.assertRaises(ValueError):
            self.manager.change_password(session.session_id, "wrongpassword", new_password)

        # Test invalid new password
        with self.assertRaises(ValueError):
            self.manager.change_password(session.session_id, new_password, "short")

        # Test invalid session
        with self.assertRaises(ValueError):
            self.manager.change_password("invalid_session", new_password, "newpassword123")

    def test_2fa(self):
        """Test two-factor authentication"""
        # Register and login test user
        self.manager.register_user(self.test_username, self.test_password)
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password
        )
        session = self.manager.login(credentials)

        # Test 2FA setup
        setup = self.manager.enable_2fa(session.session_id)
        self.assertIsInstance(setup, TwoFactorSetup)
        self.assertTrue(setup.secret)
        self.assertTrue(setup.qr_code)
        self.assertEqual(len(setup.backup_codes), 8)

        # Test 2FA verification with TOTP
        totp = pyotp.TOTP(setup.secret)
        code = totp.now()
        self.assertTrue(self.manager.verify_2fa(self.test_username, code))

        # Test 2FA verification with backup code
        backup_code = setup.backup_codes[0]
        self.assertTrue(self.manager.verify_2fa(self.test_username, backup_code))
        # Backup code should be consumed
        self.assertFalse(self.manager.verify_2fa(self.test_username, backup_code))

        # Test login with 2FA
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password,
            two_factor_code=code
        )
        new_session = self.manager.login(credentials)
        self.assertIsInstance(new_session, LoginSession)

        # Test login without 2FA code
        with self.assertRaises(ValueError):
            self.manager.login(LoginCredentials(
                username=self.test_username,
                password=self.test_password
            ))

        # Test login with invalid 2FA code
        with self.assertRaises(ValueError):
            self.manager.login(LoginCredentials(
                username=self.test_username,
                password=self.test_password,
                two_factor_code="000000"
            ))

        # Test 2FA disable with TOTP
        self.manager.disable_2fa(session.session_id, totp.now())

        # Verify 2FA is disabled
        self.assertFalse(self.manager.verify_2fa(self.test_username, totp.now()))

        # Test 2FA disable with invalid code
        with self.assertRaises(ValueError):
            self.manager.disable_2fa(session.session_id, "000000")

        # Test 2FA disable when not enabled
        with self.assertRaises(ValueError):
            self.manager.disable_2fa(session.session_id, "000000")

    def test_2fa_edge_cases(self):
        """Test two-factor authentication edge cases"""
        # Register and login test user
        self.manager.register_user(self.test_username, self.test_password)
        credentials = LoginCredentials(
            username=self.test_username,
            password=self.test_password
        )
        session = self.manager.login(credentials)

        # Test 2FA setup with invalid session
        with self.assertRaises(ValueError):
            self.manager.enable_2fa("invalid_session")

        # Test 2FA verification for non-existent user
        self.assertFalse(self.manager.verify_2fa("nonexistent", "000000"))

        # Test 2FA verification for user without 2FA
        self.assertFalse(self.manager.verify_2fa(self.test_username, "000000"))

        # Enable 2FA
        setup = self.manager.enable_2fa(session.session_id)

        # Test 2FA verification with invalid code
        self.assertFalse(self.manager.verify_2fa(self.test_username, "invalid_code"))

        # Test 2FA verification with expired TOTP code
        old_code = pyotp.TOTP(setup.secret).now()
        # Wait for code to expire (31 seconds)
        import time
        time.sleep(31)
        self.assertFalse(self.manager.verify_2fa(self.test_username, old_code))

        # Test 2FA disable with invalid session
        with self.assertRaises(ValueError):
            self.manager.disable_2fa("invalid_session", "000000")

        # Test 2FA disable for user without 2FA
        self.manager.disable_2fa(session.session_id, pyotp.TOTP(setup.secret).now())
        with self.assertRaises(ValueError):
            self.manager.disable_2fa(session.session_id, "000000")

if __name__ == "__main__":
    unittest.main() 