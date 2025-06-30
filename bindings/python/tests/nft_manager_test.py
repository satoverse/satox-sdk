"""
Satox SDK NFT Manager Tests
"""

import unittest
from datetime import datetime
from satox.nft.nft_manager import NFTManager, NFTConfig, NFTType, NFTStatus, NFT

class TestNFTManager(unittest.TestCase):
    """Test cases for NFT Manager"""

    def setUp(self):
        """Set up test environment"""
        self.manager = NFTManager()
        self.manager.initialize()
        self.test_owner = "0x1234567890abcdef1234567890abcdef12345678"
        self.test_creator = "0xabcdef1234567890abcdef1234567890abcdef12"

    def test_initialize(self):
        """Test NFT Manager initialization"""
        # Test default initialization
        manager = NFTManager()
        self.assertFalse(manager.initialized)
        
        manager.initialize()
        self.assertTrue(manager.initialized)

        # Test custom configuration
        config = NFTConfig(
            version="2.0.0",
            max_name_length=100,
            min_name_length=5,
            max_symbol_length=15,
            min_symbol_length=3,
            max_metadata_size=2 * 1024 * 1024,  # 2MB
            max_royalty_percentage=25.0,
            min_royalty_percentage=1.0,
            max_supply=100000,
            min_supply=10
        )
        manager = NFTManager()
        manager.initialize(config)
        self.assertEqual(manager.config.version, "2.0.0")
        self.assertEqual(manager.config.max_name_length, 100)
        self.assertEqual(manager.config.min_name_length, 5)
        self.assertEqual(manager.config.max_symbol_length, 15)
        self.assertEqual(manager.config.min_symbol_length, 3)
        self.assertEqual(manager.config.max_metadata_size, 2 * 1024 * 1024)
        self.assertEqual(manager.config.max_royalty_percentage, 25.0)
        self.assertEqual(manager.config.min_royalty_percentage, 1.0)
        self.assertEqual(manager.config.max_supply, 100000)
        self.assertEqual(manager.config.min_supply, 10)

        # Test double initialization
        with self.assertRaises(RuntimeError):
            manager.initialize()

    def test_create_nft(self):
        """Test NFT creation"""
        # Test valid NFT creation
        nft = self.manager.create_nft(
            name="Test NFT",
            symbol="TEST",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "Test NFT", "image": "ipfs://test"},
            royalty_percentage=5.0
        )
        self.assertEqual(nft.name, "Test NFT")
        self.assertEqual(nft.symbol, "TEST")
        self.assertEqual(nft.type, NFTType.IMAGE)
        self.assertEqual(nft.owner, self.test_owner)
        self.assertEqual(nft.creator, self.test_creator)
        self.assertEqual(nft.total_supply, 1000)
        self.assertEqual(nft.supply, 0)
        self.assertEqual(nft.royalty_percentage, 5.0)
        self.assertEqual(nft.status, NFTStatus.ACTIVE)
        self.assertIsInstance(nft.created_at, datetime)
        self.assertIsInstance(nft.updated_at, datetime)

        # Test invalid NFT name
        with self.assertRaises(ValueError):
            self.manager.create_nft(
                name="Test@NFT",
                symbol="TEST",
                type=NFTType.IMAGE,
                owner=self.test_owner,
                creator=self.test_creator,
                total_supply=1000,
                metadata={"description": "Test NFT"}
            )

        # Test invalid NFT symbol
        with self.assertRaises(ValueError):
            self.manager.create_nft(
                name="Test NFT",
                symbol="TEST@",
                type=NFTType.IMAGE,
                owner=self.test_owner,
                creator=self.test_creator,
                total_supply=1000,
                metadata={"description": "Test NFT"}
            )

        # Test invalid total supply
        with self.assertRaises(ValueError):
            self.manager.create_nft(
                name="Test NFT",
                symbol="TEST",
                type=NFTType.IMAGE,
                owner=self.test_owner,
                creator=self.test_creator,
                total_supply=0,
                metadata={"description": "Test NFT"}
            )

        # Test invalid owner address
        with self.assertRaises(ValueError):
            self.manager.create_nft(
                name="Test NFT",
                symbol="TEST",
                type=NFTType.IMAGE,
                owner="invalid_address",
                creator=self.test_creator,
                total_supply=1000,
                metadata={"description": "Test NFT"}
            )

        # Test invalid creator address
        with self.assertRaises(ValueError):
            self.manager.create_nft(
                name="Test NFT",
                symbol="TEST",
                type=NFTType.IMAGE,
                owner=self.test_owner,
                creator="invalid_address",
                total_supply=1000,
                metadata={"description": "Test NFT"}
            )

        # Test invalid royalty percentage
        with self.assertRaises(ValueError):
            self.manager.create_nft(
                name="Test NFT",
                symbol="TEST",
                type=NFTType.IMAGE,
                owner=self.test_owner,
                creator=self.test_creator,
                total_supply=1000,
                metadata={"description": "Test NFT"},
                royalty_percentage=60.0
            )

    def test_get_nft(self):
        """Test NFT retrieval"""
        # Create test NFT
        nft = self.manager.create_nft(
            name="Test NFT",
            symbol="TEST",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "Test NFT"}
        )

        # Test valid NFT retrieval
        retrieved = self.manager.get_nft(nft.id)
        self.assertEqual(retrieved.name, "Test NFT")
        self.assertEqual(retrieved.symbol, "TEST")

        # Test non-existent NFT
        with self.assertRaises(RuntimeError):
            self.manager.get_nft("non_existent")

    def test_list_nfts(self):
        """Test NFT listing"""
        # Create test NFTs
        self.manager.create_nft(
            name="NFT 1",
            symbol="NFT1",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "NFT 1"}
        )
        self.manager.create_nft(
            name="NFT 2",
            symbol="NFT2",
            type=NFTType.VIDEO,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=2000,
            metadata={"description": "NFT 2"}
        )

        # Test NFT listing
        nfts = self.manager.list_nfts()
        self.assertEqual(len(nfts), 2)

        # Test filtering by owner
        nfts = self.manager.list_nfts(owner=self.test_owner)
        self.assertEqual(len(nfts), 2)

        # Test filtering by creator
        nfts = self.manager.list_nfts(creator=self.test_creator)
        self.assertEqual(len(nfts), 2)

        # Test filtering by type
        nfts = self.manager.list_nfts(type=NFTType.IMAGE)
        self.assertEqual(len(nfts), 1)
        self.assertEqual(nfts[0].name, "NFT 1")

        # Test filtering by status
        nfts = self.manager.list_nfts(status=NFTStatus.ACTIVE)
        self.assertEqual(len(nfts), 2)

    def test_update_nft(self):
        """Test NFT updates"""
        # Create test NFT
        nft = self.manager.create_nft(
            name="Test NFT",
            symbol="TEST",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "Test NFT"}
        )

        # Test valid updates
        updated = self.manager.update_nft(nft.id, {
            "name": "Updated NFT",
            "symbol": "UPD",
            "metadata": {"description": "Updated NFT", "image": "ipfs://updated"},
            "royalty_percentage": 10.0
        })
        self.assertEqual(updated.name, "Updated NFT")
        self.assertEqual(updated.symbol, "UPD")
        self.assertEqual(updated.metadata["description"], "Updated NFT")
        self.assertEqual(updated.royalty_percentage, 10.0)

        # Test invalid updates
        with self.assertRaises(ValueError):
            self.manager.update_nft(nft.id, {
                "name": "Test@NFT"
            })

        with self.assertRaises(ValueError):
            self.manager.update_nft(nft.id, {
                "symbol": "TEST@"
            })

        with self.assertRaises(ValueError):
            self.manager.update_nft(nft.id, {
                "royalty_percentage": 60.0
            })

    def test_delete_nft(self):
        """Test NFT deletion"""
        # Create test NFT
        nft = self.manager.create_nft(
            name="Test NFT",
            symbol="TEST",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "Test NFT"}
        )

        # Test valid NFT deletion
        self.manager.delete_nft(nft.id)
        with self.assertRaises(RuntimeError):
            self.manager.get_nft(nft.id)

        # Test non-existent NFT deletion
        with self.assertRaises(RuntimeError):
            self.manager.delete_nft("non_existent")

    def test_nft_status(self):
        """Test NFT status changes"""
        # Create test NFT
        nft = self.manager.create_nft(
            name="Test NFT",
            symbol="TEST",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "Test NFT"}
        )

        # Test pause
        paused = self.manager.pause_nft(nft.id)
        self.assertEqual(paused.status, NFTStatus.PAUSED)

        # Test resume
        resumed = self.manager.resume_nft(nft.id)
        self.assertEqual(resumed.status, NFTStatus.ACTIVE)

        # Test freeze
        frozen = self.manager.freeze_nft(nft.id)
        self.assertEqual(frozen.status, NFTStatus.FROZEN)

    def test_nft_mint_burn(self):
        """Test NFT minting and burning"""
        # Create test NFT
        nft = self.manager.create_nft(
            name="Test NFT",
            symbol="TEST",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "Test NFT"}
        )

        # Test minting
        minted = self.manager.mint_nft(nft.id, 100)
        self.assertEqual(minted.supply, 100)

        # Test burning
        burned = self.manager.burn_nft(nft.id, 50)
        self.assertEqual(burned.supply, 50)

        # Test minting beyond total supply
        with self.assertRaises(ValueError):
            self.manager.mint_nft(nft.id, 1000)

        # Test burning more than available
        with self.assertRaises(ValueError):
            self.manager.burn_nft(nft.id, 100)

        # Test minting when paused
        self.manager.pause_nft(nft.id)
        with self.assertRaises(RuntimeError):
            self.manager.mint_nft(nft.id, 10)

        # Test burning when paused
        with self.assertRaises(RuntimeError):
            self.manager.burn_nft(nft.id, 10)

    def test_nft_transfer(self):
        """Test NFT transfers"""
        # Create test NFT
        nft = self.manager.create_nft(
            name="Test NFT",
            symbol="TEST",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "Test NFT"}
        )

        # Mint some tokens
        self.manager.mint_nft(nft.id, 100)

        # Test valid transfer
        to_address = "0xabcdef1234567890abcdef1234567890abcdef12"
        result = self.manager.transfer_nft(nft.id, self.test_owner, to_address, 50)
        self.assertTrue(result)

        # Test invalid addresses
        with self.assertRaises(ValueError):
            self.manager.transfer_nft(nft.id, "invalid_address", to_address, 50)

        with self.assertRaises(ValueError):
            self.manager.transfer_nft(nft.id, self.test_owner, "invalid_address", 50)

        # Test invalid amount
        with self.assertRaises(ValueError):
            self.manager.transfer_nft(nft.id, self.test_owner, to_address, 0)

        with self.assertRaises(ValueError):
            self.manager.transfer_nft(nft.id, self.test_owner, to_address, 200)

        # Test transfer when paused
        self.manager.pause_nft(nft.id)
        with self.assertRaises(RuntimeError):
            self.manager.transfer_nft(nft.id, self.test_owner, to_address, 10)

    def test_nft_balance(self):
        """Test NFT balance operations"""
        # Create test NFT
        nft = self.manager.create_nft(
            name="Test NFT",
            symbol="TEST",
            type=NFTType.IMAGE,
            owner=self.test_owner,
            creator=self.test_creator,
            total_supply=1000,
            metadata={"description": "Test NFT"}
        )

        # Test get balance
        balance = self.manager.get_nft_balance(nft.id, self.test_owner)
        self.assertEqual(balance, 0)  # Placeholder value

        # Test invalid address
        with self.assertRaises(ValueError):
            self.manager.get_nft_balance(nft.id, "invalid_address")

if __name__ == "__main__":
    unittest.main() 