import unittest
from satox.consensus.consensus_manager import ConsensusManager
from satox.core.error_handler import SatoxError
from satox.core.security import SecurityManager
import pytest
from unittest.mock import Mock, patch

class TestConsensusSecurity(unittest.TestCase):
    def setUp(self):
        """Set up test environment."""
        self.consensus_manager = ConsensusManager()
        self.security = SecurityManager()
        self.test_block = {
            "height": 1000,
            "hash": "test_hash",
            "previous_hash": "previous_hash",
            "timestamp": 1234567890,
            "difficulty": 1000,
            "nonce": 12345,
            "miner": "test_miner"
        }
        
    def test_block_validation(self):
        """Test block validation security."""
        # Test valid block
        block = self.consensus_manager.create_block(self.test_block)
        self.assertTrue(self.consensus_manager.validate_block(block))
        
        # Test invalid block
        invalid_block = self.test_block.copy()
        invalid_block["hash"] = "invalid_hash"
        with self.assertRaises(SatoxError):
            self.consensus_manager.validate_block(invalid_block)
            
    def test_proof_of_work(self):
        """Test proof of work security."""
        # Test valid proof of work
        block = self.consensus_manager.create_block(self.test_block)
        self.assertTrue(self.consensus_manager.verify_proof_of_work(block))
        
        # Test invalid proof of work
        invalid_block = self.test_block.copy()
        invalid_block["nonce"] = 0
        with self.assertRaises(SatoxError):
            self.consensus_manager.verify_proof_of_work(invalid_block)
            
    def test_difficulty_adjustment(self):
        """Test difficulty adjustment security."""
        # Test difficulty calculation
        block = self.consensus_manager.create_block(self.test_block)
        new_difficulty = self.consensus_manager.calculate_difficulty(block)
        self.assertIsNotNone(new_difficulty)
        self.assertGreater(new_difficulty, 0)
        
        # Test difficulty validation
        self.assertTrue(self.consensus_manager.validate_difficulty(new_difficulty))
        
    def test_block_time(self):
        """Test block time security."""
        # Test block time validation
        block = self.consensus_manager.create_block(self.test_block)
        self.assertTrue(self.consensus_manager.validate_block_time(block))
        
        # Test invalid block time
        invalid_block = self.test_block.copy()
        invalid_block["timestamp"] = 0
        with self.assertRaises(SatoxError):
            self.consensus_manager.validate_block_time(invalid_block)
            
    def test_chain_validation(self):
        """Test chain validation security."""
        # Test valid chain
        blocks = [self.consensus_manager.create_block(self.test_block) for _ in range(3)]
        self.assertTrue(self.consensus_manager.validate_chain(blocks))
        
        # Test invalid chain
        invalid_blocks = blocks.copy()
        invalid_blocks[1]["previous_hash"] = "invalid_hash"
        with self.assertRaises(SatoxError):
            self.consensus_manager.validate_chain(invalid_blocks)
            
    def test_double_spend_protection(self):
        """Test double spend protection."""
        # Test transaction validation
        transaction = self.consensus_manager.create_transaction("sender", "recipient", 100)
        self.assertTrue(self.consensus_manager.validate_transaction(transaction))
        
        # Test double spend attempt
        with self.assertRaises(SatoxError):
            self.consensus_manager.process_double_spend(transaction)
            
    def test_51_percent_attack_protection(self):
        """Test 51% attack protection."""
        # Test network validation
        self.assertTrue(self.consensus_manager.validate_network_health())
        
        # Test attack detection
        self.assertTrue(self.consensus_manager.detect_51_percent_attack())
        
    def test_block_reward_security(self):
        """Test block reward security."""
        # Test reward calculation
        block = self.consensus_manager.create_block(self.test_block)
        reward = self.consensus_manager.calculate_block_reward(block)
        self.assertIsNotNone(reward)
        self.assertGreater(reward, 0)
        
        # Test reward validation
        self.assertTrue(self.consensus_manager.validate_block_reward(block, reward))
        
    def test_consensus_rules(self):
        """Test consensus rules security."""
        # Test rule validation
        block = self.consensus_manager.create_block(self.test_block)
        self.assertTrue(self.consensus_manager.validate_consensus_rules(block))
        
        # Test rule enforcement
        self.assertTrue(self.consensus_manager.enforce_consensus_rules(block))
        
    def test_network_sync_security(self):
        """Test network sync security."""
        # Test sync validation
        self.assertTrue(self.consensus_manager.validate_network_sync())
        
        # Test sync enforcement
        self.assertTrue(self.consensus_manager.enforce_network_sync())
        
    def test_consensus_state(self):
        """Test consensus state security."""
        # Test state validation
        state = self.consensus_manager.get_consensus_state()
        self.assertIsNotNone(state)
        
        # Test state tampering
        tampered_state = self.consensus_manager.tamper_state(state)
        self.assertFalse(self.consensus_manager.validate_state(tampered_state))
        
    def test_consensus_history(self):
        """Test consensus history security."""
        # Test history validation
        history = self.consensus_manager.get_consensus_history()
        self.assertIsNotNone(history)
        
        # Test history tampering
        tampered_history = self.consensus_manager.tamper_history(history)
        self.assertFalse(self.consensus_manager.validate_history(tampered_history))
        
    def test_consensus_audit(self):
        """Test consensus audit security."""
        # Test audit logging
        self.assertTrue(self.consensus_manager.is_audit_logging_enabled())
        
        # Test audit trail
        audit_trail = self.consensus_manager.get_audit_trail()
        self.assertIsNotNone(audit_trail)
        self.assertTrue(len(audit_trail) > 0)
        
    def test_consensus_monitoring(self):
        """Test consensus monitoring security."""
        # Test monitoring setup
        self.assertTrue(self.consensus_manager.is_monitoring_enabled())
        
        # Test monitoring alerts
        alerts = self.consensus_manager.get_monitoring_alerts()
        self.assertIsNotNone(alerts)
        
    def test_consensus_policies(self):
        """Test consensus security policies."""
        # Test policy enforcement
        policies = self.consensus_manager.get_security_policies()
        self.assertIsNotNone(policies)
        
        # Test policy validation
        self.assertTrue(self.consensus_manager.validate_policies(policies))
        
    def test_consensus_validation(self):
        """Test consensus validation security."""
        # Test comprehensive validation
        validation_result = self.consensus_manager.validate_consensus_comprehensive()
        self.assertTrue(validation_result.is_valid)
        self.assertEqual(len(validation_result.errors), 0)
        
    def test_consensus_recovery(self):
        """Test consensus recovery security."""
        # Test recovery mechanism
        self.assertTrue(self.consensus_manager.has_recovery_mechanism())
        
        # Test recovery process
        recovered_state = self.consensus_manager.recover_consensus_state()
        self.assertIsNotNone(recovered_state)
        self.assertTrue(self.consensus_manager.validate_state(recovered_state))
        
    def test_consensus_quarantine(self):
        """Test consensus quarantine security."""
        # Test quarantine mechanism
        self.assertTrue(self.consensus_manager.has_quarantine_mechanism())
        
        # Test quarantine process
        quarantined_state = self.consensus_manager.quarantine_consensus_state()
        self.assertTrue(self.consensus_manager.is_quarantined(quarantined_state))
        
    def test_consensus_optimization(self):
        """Test consensus optimization security."""
        # Test optimization validation
        optimization = self.consensus_manager.optimize_consensus()
        self.assertIsNotNone(optimization)
        
        # Test optimization verification
        self.assertTrue(self.consensus_manager.verify_optimization(optimization)) 