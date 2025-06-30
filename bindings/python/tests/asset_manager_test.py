"""
Satox SDK Asset Manager Tests
"""

import unittest
from satox.asset.asset_manager import AssetManager, AssetConfig, AssetType, AssetStatus, Asset

class TestAssetManager(unittest.TestCase):
    """Test cases for Asset Manager"""

    def setUp(self):
        """Set up test environment"""
        self.manager = AssetManager()
        self.manager.initialize()
        self.test_owner = "0x1234567890abcdef1234567890abcdef12345678"

    def test_initialize(self):
        """Test Asset Manager initialization"""
        # Test default initialization
        manager = AssetManager()
        self.assertFalse(manager.initialized)
        
        manager.initialize()
        self.assertTrue(manager.initialized)

        # Test custom configuration
        config = AssetConfig(
            version="2.0.0",
            max_name_length=100,
            min_name_length=5,
            max_symbol_length=15,
            min_symbol_length=3,
            max_decimals=20,
            min_decimals=2
        )
        manager = AssetManager()
        manager.initialize(config)
        self.assertEqual(manager.config.version, "2.0.0")
        self.assertEqual(manager.config.max_name_length, 100)
        self.assertEqual(manager.config.min_name_length, 5)
        self.assertEqual(manager.config.max_symbol_length, 15)
        self.assertEqual(manager.config.min_symbol_length, 3)
        self.assertEqual(manager.config.max_decimals, 20)
        self.assertEqual(manager.config.min_decimals, 2)

        # Test double initialization
        with self.assertRaises(RuntimeError):
            manager.initialize()

    def test_create_asset(self):
        """Test asset creation"""
        # Test valid asset creation
        asset = self.manager.create_asset(
            name="Test Asset",
            symbol="TEST",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=1000000,
            owner=self.test_owner
        )
        self.assertEqual(asset.name, "Test Asset")
        self.assertEqual(asset.symbol, "TEST")
        self.assertEqual(asset.type, AssetType.TOKEN)
        self.assertEqual(asset.decimals, 18)
        self.assertEqual(asset.total_supply, 1000000)
        self.assertEqual(asset.owner, self.test_owner)
        self.assertEqual(asset.status, AssetStatus.ACTIVE)

        # Test invalid asset name
        with self.assertRaises(ValueError):
            self.manager.create_asset(
                name="Test@Asset",
                symbol="TEST",
                type=AssetType.TOKEN,
                decimals=18,
                total_supply=1000000,
                owner=self.test_owner
            )

        # Test invalid asset symbol
        with self.assertRaises(ValueError):
            self.manager.create_asset(
                name="Test Asset",
                symbol="TEST@",
                type=AssetType.TOKEN,
                decimals=18,
                total_supply=1000000,
                owner=self.test_owner
            )

        # Test invalid decimals
        with self.assertRaises(ValueError):
            self.manager.create_asset(
                name="Test Asset",
                symbol="TEST",
                type=AssetType.TOKEN,
                decimals=20,
                total_supply=1000000,
                owner=self.test_owner
            )

        # Test invalid total supply
        with self.assertRaises(ValueError):
            self.manager.create_asset(
                name="Test Asset",
                symbol="TEST",
                type=AssetType.TOKEN,
                decimals=18,
                total_supply=-1000000,
                owner=self.test_owner
            )

        # Test invalid owner address
        with self.assertRaises(ValueError):
            self.manager.create_asset(
                name="Test Asset",
                symbol="TEST",
                type=AssetType.TOKEN,
                decimals=18,
                total_supply=1000000,
                owner="invalid_address"
            )

    def test_get_asset(self):
        """Test asset retrieval"""
        # Create test asset
        asset = self.manager.create_asset(
            name="Test Asset",
            symbol="TEST",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=1000000,
            owner=self.test_owner
        )

        # Test valid asset retrieval
        retrieved = self.manager.get_asset(asset.id)
        self.assertEqual(retrieved.name, "Test Asset")
        self.assertEqual(retrieved.symbol, "TEST")

        # Test non-existent asset
        with self.assertRaises(RuntimeError):
            self.manager.get_asset("non_existent")

    def test_list_assets(self):
        """Test asset listing"""
        # Create test assets
        self.manager.create_asset(
            name="Asset 1",
            symbol="A1",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=1000000,
            owner=self.test_owner
        )
        self.manager.create_asset(
            name="Asset 2",
            symbol="A2",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=2000000,
            owner=self.test_owner
        )

        # Test asset listing
        assets = self.manager.list_assets()
        self.assertEqual(len(assets), 2)

    def test_update_asset(self):
        """Test asset updates"""
        # Create test asset
        asset = self.manager.create_asset(
            name="Test Asset",
            symbol="TEST",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=1000000,
            owner=self.test_owner
        )

        # Test valid updates
        updated = self.manager.update_asset(asset.id, {
            "name": "Updated Asset",
            "symbol": "UPD",
            "decimals": 6,
            "total_supply": 2000000,
            "metadata": {"key": "value"}
        })
        self.assertEqual(updated.name, "Updated Asset")
        self.assertEqual(updated.symbol, "UPD")
        self.assertEqual(updated.decimals, 6)
        self.assertEqual(updated.total_supply, 2000000)
        self.assertEqual(updated.metadata["key"], "value")

        # Test invalid updates
        with self.assertRaises(ValueError):
            self.manager.update_asset(asset.id, {
                "name": "Test@Asset"
            })

        with self.assertRaises(ValueError):
            self.manager.update_asset(asset.id, {
                "symbol": "TEST@"
            })

        with self.assertRaises(ValueError):
            self.manager.update_asset(asset.id, {
                "decimals": 20
            })

        with self.assertRaises(ValueError):
            self.manager.update_asset(asset.id, {
                "total_supply": -1000000
            })

    def test_delete_asset(self):
        """Test asset deletion"""
        # Create test asset
        asset = self.manager.create_asset(
            name="Test Asset",
            symbol="TEST",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=1000000,
            owner=self.test_owner
        )

        # Test valid asset deletion
        self.manager.delete_asset(asset.id)
        with self.assertRaises(RuntimeError):
            self.manager.get_asset(asset.id)

        # Test non-existent asset deletion
        with self.assertRaises(RuntimeError):
            self.manager.delete_asset("non_existent")

    def test_asset_status(self):
        """Test asset status changes"""
        # Create test asset
        asset = self.manager.create_asset(
            name="Test Asset",
            symbol="TEST",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=1000000,
            owner=self.test_owner
        )

        # Test pause
        paused = self.manager.pause_asset(asset.id)
        self.assertEqual(paused.status, AssetStatus.PAUSED)

        # Test resume
        resumed = self.manager.resume_asset(asset.id)
        self.assertEqual(resumed.status, AssetStatus.ACTIVE)

        # Test freeze
        frozen = self.manager.freeze_asset(asset.id)
        self.assertEqual(frozen.status, AssetStatus.FROZEN)

    def test_asset_balance(self):
        """Test asset balance operations"""
        # Create test asset
        asset = self.manager.create_asset(
            name="Test Asset",
            symbol="TEST",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=1000000,
            owner=self.test_owner
        )

        # Test get balance
        balance = self.manager.get_asset_balance(asset.id, self.test_owner)
        self.assertEqual(balance, 0)  # Placeholder value

        # Test invalid address
        with self.assertRaises(ValueError):
            self.manager.get_asset_balance(asset.id, "invalid_address")

    def test_asset_transfer(self):
        """Test asset transfers"""
        # Create test asset
        asset = self.manager.create_asset(
            name="Test Asset",
            symbol="TEST",
            type=AssetType.TOKEN,
            decimals=18,
            total_supply=1000000,
            owner=self.test_owner
        )

        # Test valid transfer
        to_address = "0xabcdef1234567890abcdef1234567890abcdef12"
        result = self.manager.transfer_asset(asset.id, self.test_owner, to_address, 1000)
        self.assertTrue(result)  # Placeholder value

        # Test invalid addresses
        with self.assertRaises(ValueError):
            self.manager.transfer_asset(asset.id, "invalid_address", to_address, 1000)

        with self.assertRaises(ValueError):
            self.manager.transfer_asset(asset.id, self.test_owner, "invalid_address", 1000)

        # Test invalid amount
        with self.assertRaises(ValueError):
            self.manager.transfer_asset(asset.id, self.test_owner, to_address, -1000)

if __name__ == "__main__":
    unittest.main() 