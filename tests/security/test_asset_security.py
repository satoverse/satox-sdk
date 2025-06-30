import unittest
import os
import tempfile
import json
from unittest.mock import Mock, patch
from satox.assets.asset_manager import AssetManager
from satox.core.error_handler import SatoxError
from satox.security.security_manager import SecurityManager

class TestAssetSecurity(unittest.TestCase):
    """Test cases for asset security features."""
    
    def setUp(self):
        """Set up test environment."""
        self.temp_dir = tempfile.mkdtemp()
        self.config = {
            "assets": {
                "security": {
                    "enabled": True,
                    "require_verification": True,
                    "require_metadata": True,
                    "require_permissions": True
                },
                "validation": {
                    "enabled": True,
                    "verify_ownership": True,
                    "verify_supply": True,
                    "verify_metadata": True
                },
                "permissions": {
                    "enabled": True,
                    "require_approval": True,
                    "max_approvers": 3
                },
                "quarantine": {
                    "enabled": True,
                    "auto_quarantine": True,
                    "quarantine_duration": 30
                }
            }
        }
        self.security_manager = SecurityManager()
        self.asset_manager = AssetManager(self.config, self.security_manager)
        
    def tearDown(self):
        """Clean up test environment."""
        os.rmdir(self.temp_dir)
        
    def test_asset_creation_validation(self):
        """Test asset creation validation."""
        asset_data = {
            "name": "Test Asset",
            "symbol": "TEST",
            "supply": 1000000,
            "metadata": {
                "description": "Test asset description",
                "category": "Test"
            }
        }
        self.assertTrue(self.asset_manager.validate_asset_creation(asset_data))
        
    def test_asset_creation_validation_invalid(self):
        """Test asset creation validation with invalid data."""
        invalid_asset_data = {
            "name": "",  # Invalid empty name
            "symbol": "T",  # Invalid short symbol
            "supply": -1,  # Invalid negative supply
            "metadata": {}  # Invalid empty metadata
        }
        with self.assertRaises(SatoxError):
            self.asset_manager.validate_asset_creation(invalid_asset_data)
            
    def test_ownership_verification(self):
        """Test asset ownership verification."""
        asset_id = "test_asset_123"
        owner_address = "test_owner_address"
        self.asset_manager.register_asset_ownership(asset_id, owner_address)
        self.assertTrue(self.asset_manager.verify_asset_ownership(asset_id, owner_address))
        self.assertFalse(self.asset_manager.verify_asset_ownership(asset_id, "wrong_address"))
        
    def test_transfer_security(self):
        """Test asset transfer security."""
        asset_id = "test_asset_123"
        from_address = "sender_address"
        to_address = "receiver_address"
        amount = 100
        
        # Register initial ownership
        self.asset_manager.register_asset_ownership(asset_id, from_address)
        
        # Test transfer
        self.assertTrue(self.asset_manager.validate_transfer(asset_id, from_address, to_address, amount))
        
        # Test invalid transfer
        with self.assertRaises(SatoxError):
            self.asset_manager.validate_transfer(asset_id, "wrong_sender", to_address, amount)
            
    def test_supply_validation(self):
        """Test asset supply validation."""
        asset_id = "test_asset_123"
        initial_supply = 1000000
        self.asset_manager.register_asset_supply(asset_id, initial_supply)
        
        # Test valid supply
        self.assertTrue(self.asset_manager.validate_supply(asset_id, 500000))
        
        # Test invalid supply
        with self.assertRaises(SatoxError):
            self.asset_manager.validate_supply(asset_id, initial_supply + 1)
            
    def test_metadata_security(self):
        """Test asset metadata security."""
        asset_id = "test_asset_123"
        metadata = {
            "description": "Test asset",
            "category": "Test",
            "version": "1.0"
        }
        
        # Test metadata validation
        self.assertTrue(self.asset_manager.validate_metadata(asset_id, metadata))
        
        # Test invalid metadata
        invalid_metadata = {
            "description": "",  # Invalid empty description
            "category": None,  # Invalid null category
            "version": "invalid"  # Invalid version format
        }
        with self.assertRaises(SatoxError):
            self.asset_manager.validate_metadata(asset_id, invalid_metadata)
            
    def test_permission_management(self):
        """Test asset permission management."""
        asset_id = "test_asset_123"
        user_address = "user_address"
        
        # Test permission assignment
        self.asset_manager.assign_permission(asset_id, user_address, "read")
        self.assertTrue(self.asset_manager.has_permission(asset_id, user_address, "read"))
        
        # Test permission removal
        self.asset_manager.remove_permission(asset_id, user_address, "read")
        self.assertFalse(self.asset_manager.has_permission(asset_id, user_address, "read"))
        
    def test_operation_validation(self):
        """Test asset operation validation."""
        asset_id = "test_asset_123"
        operation = "transfer"
        user_address = "user_address"
        
        # Test valid operation
        self.asset_manager.assign_permission(asset_id, user_address, operation)
        self.assertTrue(self.asset_manager.validate_operation(asset_id, user_address, operation))
        
        # Test invalid operation
        self.assertFalse(self.asset_manager.validate_operation(asset_id, user_address, "invalid_operation"))
        
    def test_state_security(self):
        """Test asset state security."""
        asset_id = "test_asset_123"
        initial_state = {
            "status": "active",
            "last_modified": "2024-01-01T00:00:00Z"
        }
        
        # Test state validation
        self.assertTrue(self.asset_manager.validate_state(asset_id, initial_state))
        
        # Test invalid state
        invalid_state = {
            "status": "invalid_status",
            "last_modified": "invalid_date"
        }
        with self.assertRaises(SatoxError):
            self.asset_manager.validate_state(asset_id, invalid_state)
            
    def test_history_validation(self):
        """Test asset history validation."""
        asset_id = "test_asset_123"
        history_entry = {
            "action": "transfer",
            "from": "sender",
            "to": "receiver",
            "amount": 100,
            "timestamp": "2024-01-01T00:00:00Z"
        }
        
        # Test history validation
        self.assertTrue(self.asset_manager.validate_history_entry(asset_id, history_entry))
        
        # Test invalid history
        invalid_history = {
            "action": "invalid_action",
            "timestamp": "invalid_date"
        }
        with self.assertRaises(SatoxError):
            self.asset_manager.validate_history_entry(asset_id, invalid_history)
            
    def test_audit_logging(self):
        """Test asset audit logging."""
        asset_id = "test_asset_123"
        action = "transfer"
        details = {
            "from": "sender",
            "to": "receiver",
            "amount": 100
        }
        
        # Test audit logging
        log_entry = self.asset_manager.log_audit(asset_id, action, details)
        self.assertIsNotNone(log_entry)
        self.assertEqual(log_entry["asset_id"], asset_id)
        self.assertEqual(log_entry["action"], action)
        
    def test_recovery_mechanism(self):
        """Test asset recovery mechanism."""
        asset_id = "test_asset_123"
        backup_data = {
            "state": "active",
            "owner": "original_owner",
            "metadata": {"version": "1.0"}
        }
        
        # Test recovery
        self.asset_manager.backup_asset(asset_id, backup_data)
        recovered_data = self.asset_manager.recover_asset(asset_id)
        self.assertEqual(recovered_data, backup_data)
        
    def test_quarantine_system(self):
        """Test asset quarantine system."""
        asset_id = "test_asset_123"
        
        # Test quarantine
        self.asset_manager.quarantine_asset(asset_id)
        self.assertTrue(self.asset_manager.is_asset_quarantined(asset_id))
        
        # Test release
        self.asset_manager.release_asset(asset_id)
        self.assertFalse(self.asset_manager.is_asset_quarantined(asset_id))
        
    def test_monitoring_system(self):
        """Test asset monitoring system."""
        asset_id = "test_asset_123"
        
        # Test monitoring setup
        self.asset_manager.enable_monitoring(asset_id)
        self.assertTrue(self.asset_manager.is_monitoring_enabled(asset_id))
        
        # Test alert generation
        alert = self.asset_manager.generate_alert(asset_id, "suspicious_activity")
        self.assertIsNotNone(alert)
        self.assertEqual(alert["asset_id"], asset_id)
        
    def test_security_policies(self):
        """Test asset security policies."""
        policies = self.asset_manager.get_security_policies()
        self.assertIsNotNone(policies)
        self.assertTrue(policies["enabled"])
        self.assertTrue(policies["require_verification"])
        
    def test_invalid_config(self):
        """Test handling of invalid configuration."""
        invalid_config = {
            "assets": {
                "security": {
                    "enabled": True,
                    "invalid_field": "value"
                }
            }
        }
        with self.assertRaises(SatoxError):
            AssetManager(invalid_config, self.security_manager)
            
    def test_invalid_validation_config(self):
        """Test handling of invalid validation configuration."""
        invalid_config = {
            "assets": {
                "validation": {
                    "enabled": True,
                    "verify_ownership": "invalid_value"
                }
            }
        }
        with self.assertRaises(SatoxError):
            AssetManager(invalid_config, self.security_manager)
            
    def test_invalid_permissions_config(self):
        """Test handling of invalid permissions configuration."""
        invalid_config = {
            "assets": {
                "permissions": {
                    "enabled": True,
                    "max_approvers": -1
                }
            }
        }
        with self.assertRaises(SatoxError):
            AssetManager(invalid_config, self.security_manager)
            
    def test_invalid_quarantine_config(self):
        """Test handling of invalid quarantine configuration."""
        invalid_config = {
            "assets": {
                "quarantine": {
                    "enabled": True,
                    "quarantine_duration": 0
                }
            }
        }
        with self.assertRaises(SatoxError):
            AssetManager(invalid_config, self.security_manager)
            
if __name__ == "__main__":
    unittest.main() 