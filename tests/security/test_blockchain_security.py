import unittest
from satox.blockchain.blockchain_manager import BlockchainManager
from satox.core.error_handler import SatoxError
import pytest

class TestBlockchainSecurity(unittest.TestCase):
    def setUp(self):
        """Set up test environment."""
        self.manager = BlockchainManager()

    def test_block_reward_manipulation(self):
        """Test security of block reward calculations against manipulation."""
        # Test negative block height
        with self.assertRaises(SatoxError):
            self.manager.get_block_rewards(-1)

        # Test extremely large block height
        with self.assertRaises(SatoxError):
            self.manager.get_block_rewards(2**64)

        # Test reward calculation integrity
        rewards = self.manager.get_block_rewards(1)
        self.assertIsInstance(rewards["mining_reward"], int)
        self.assertGreaterEqual(rewards["mining_reward"], 0)
        self.assertLessEqual(rewards["mining_reward"], 270)  # Max reward

    def test_halving_integrity(self):
        """Test security of halving calculations."""
        # Test halving boundary integrity
        before_halving = self.manager.get_block_rewards(2_099_999)
        at_halving = self.manager.get_block_rewards(2_100_000)
        after_halving = self.manager.get_block_rewards(2_100_001)

        # Verify halving occurs exactly at the right block
        self.assertEqual(before_halving["mining_reward"], 270)
        self.assertEqual(at_halving["mining_reward"], 135)
        self.assertEqual(after_halving["mining_reward"], 135)

        # Verify halving count is correct
        self.assertEqual(before_halving["halvings"], 0)
        self.assertEqual(at_halving["halvings"], 1)
        self.assertEqual(after_halving["halvings"], 1)

    def test_instamine_protection(self):
        """Test security of instamine protection."""
        # Test instamine protection boundary
        at_protection = self.manager.get_block_rewards(1000)
        after_protection = self.manager.get_block_rewards(1001)

        # Verify protection is applied correctly
        self.assertEqual(at_protection["mining_reward"], 135)  # 270/2
        self.assertEqual(after_protection["mining_reward"], 270)  # Full reward

    def test_reward_distribution(self):
        """Test security of reward distribution."""
        # Test reward distribution at different block heights
        test_heights = [1, 1001, 2_100_000, 4_200_000, 6_300_000]
        expected_rewards = [135, 270, 135, 67, 33]

        for height, expected in zip(test_heights, expected_rewards):
            rewards = self.manager.get_block_rewards(height)
            self.assertEqual(rewards["mining_reward"], expected)
            self.assertIsInstance(rewards["mining_reward"], int)
            self.assertGreaterEqual(rewards["mining_reward"], 0)

    def test_block_height_validation(self):
        """Test security of block height validation."""
        # Test invalid block heights
        invalid_heights = [
            -1,                    # Negative
            0,                     # Zero
            "invalid",             # String
            2.5,                   # Float
            None,                  # None
            True,                  # Boolean
            [],                    # List
            {},                    # Dict
            2**64,                 # Too large
        ]

        for height in invalid_heights:
            with self.assertRaises(SatoxError):
                self.manager.get_block_rewards(height)

    def test_reward_calculation_overflow(self):
        """Test security against integer overflow in reward calculations."""
        # Test very high block heights to ensure no overflow
        high_heights = [
            2_100_000 * 10,    # 10 halvings
            2_100_000 * 20,    # 20 halvings
            2_100_000 * 30,    # 30 halvings
        ]

        for height in high_heights:
            rewards = self.manager.get_block_rewards(height)
            self.assertIsInstance(rewards["mining_reward"], int)
            self.assertGreaterEqual(rewards["mining_reward"], 0)
            self.assertLessEqual(rewards["mining_reward"], 270)

    def test_halving_period_security(self):
        """Test security of halving period calculations."""
        # Test halving period calculation
        period = self.manager.get_halving_period()
        self.assertIsInstance(period, int)
        self.assertEqual(period, 2_100_000 * 60)  # 2,100,000 blocks * 60 seconds

        # Test period calculation with different block times
        test_block_times = [30, 60, 120]
        for block_time in test_block_times:
            self.manager.get_block_time = lambda: block_time
            period = self.manager.get_halving_period()
            self.assertEqual(period, 2_100_000 * block_time) 