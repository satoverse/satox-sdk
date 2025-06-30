import unittest
from satox.blockchain.blockchain_manager import BlockchainManager
from satox.tests.utils.mock_wallet import MockWallet
import pytest
import time

# Satoxcoin network parameters
SATOX_P2P_PORT = 60777
SATOX_RPC_PORT = 7777
BLOCK_TIME = 60  # seconds
BLOCK_CONFIRMATIONS = 100
INSTAMINE_PROTECTION = 1000
HALVING_PERIOD = 4 * 365 * 24 * 60 * 60  # 4 years in seconds
POW_REWARD = 270  # SATOX
P2E_REWARD = 30   # SATOX
DGW_WINDOW = 30   # DGW retargeting window
KWP_DIFFICULTY_ADJUSTMENT = 0.5  # KawPoW difficulty adjustment factor

class TestBlockchainManager(unittest.TestCase):
    def setUp(self):
        """Set up test environment."""
        self.wallet = MockWallet()
        self.wallet.connect()
        
        # Create addresses with initial balance
        self.address1 = self.wallet.create_address()
        self.address2 = self.wallet.create_address()
        self.initial_balance = 1000000000000000000  # 1 SATOX
        self.wallet._addresses[self.address1] = {"SATOX": self.initial_balance}
        
        self.manager = BlockchainManager(self.wallet)

    def test_network_parameters(self):
        """Test Satoxcoin network parameters"""
        self.assertEqual(self.manager.get_p2p_port(), SATOX_P2P_PORT)
        self.assertEqual(self.manager.get_rpc_port(), SATOX_RPC_PORT)
        self.assertEqual(self.manager.get_block_time(), BLOCK_TIME)
        self.assertEqual(self.manager.get_confirmations(), BLOCK_CONFIRMATIONS)
        self.assertEqual(self.manager.get_instamine_protection(), INSTAMINE_PROTECTION)
        self.assertEqual(self.manager.get_halving_period(), HALVING_PERIOD)
        self.assertEqual(self.manager.get_algorithm(), "KawPoW")
        self.assertEqual(self.manager.get_retarget_algorithm(), "DGW")

    def test_kawpow_mining(self):
        """Test KawPoW mining algorithm"""
        # Get initial difficulty
        initial_difficulty = self.manager.get_difficulty()
        
        # Mine a block
        block_height = self.manager.mine_block()
        
        # Verify block was mined with KawPoW
        block = self.manager.get_block(block_height)
        self.assertEqual(block.algorithm, "KawPoW")
        self.assertIsNotNone(block.nonce)
        self.assertIsNotNone(block.mix_hash)

    def test_dgw_retargeting(self):
        """Test DGW difficulty retargeting"""
        # Get initial difficulty
        initial_difficulty = self.manager.get_difficulty()
        
        # Mine blocks with varying times to test DGW retargeting
        for i in range(DGW_WINDOW):
            # Simulate faster/slower block times
            if i % 2 == 0:
                time.sleep(BLOCK_TIME * 0.8)  # 20% faster
            else:
                time.sleep(BLOCK_TIME * 1.2)  # 20% slower
            self.manager.mine_block()
        
        # Get new difficulty
        new_difficulty = self.manager.get_difficulty()
        
        # Verify difficulty was adjusted
        self.assertNotEqual(initial_difficulty, new_difficulty)
        
        # Verify difficulty adjustment is within DGW limits
        max_adjustment = initial_difficulty * (1 + KWP_DIFFICULTY_ADJUSTMENT)
        min_adjustment = initial_difficulty * (1 - KWP_DIFFICULTY_ADJUSTMENT)
        self.assertLessEqual(new_difficulty, max_adjustment)
        self.assertGreaterEqual(new_difficulty, min_adjustment)

    def test_block_rewards(self):
        """Test block reward calculations including halving and instamine protection."""
        # Test initial block reward
        rewards = self.manager.get_block_rewards(1)
        self.assertEqual(rewards["mining_reward"], 135)  # 270/2 due to instamine protection
        self.assertEqual(rewards["halvings"], 0)
        
        # Test after instamine protection
        rewards = self.manager.get_block_rewards(1001)
        self.assertEqual(rewards["mining_reward"], 270)  # Full reward
        self.assertEqual(rewards["halvings"], 0)
        
        # Test first halving
        rewards = self.manager.get_block_rewards(2_100_000)
        self.assertEqual(rewards["mining_reward"], 135)  # 270/2
        self.assertEqual(rewards["halvings"], 1)
        
        # Test second halving
        rewards = self.manager.get_block_rewards(4_200_000)
        self.assertEqual(rewards["mining_reward"], 67)  # 270/4
        self.assertEqual(rewards["halvings"], 2)
        
        # Test third halving
        rewards = self.manager.get_block_rewards(6_300_000)
        self.assertEqual(rewards["mining_reward"], 33)  # 270/8
        self.assertEqual(rewards["halvings"], 3)
        
        # Test edge case at halving boundary
        rewards = self.manager.get_block_rewards(2_099_999)
        self.assertEqual(rewards["mining_reward"], 270)  # Full reward
        self.assertEqual(rewards["halvings"], 0)
        
        rewards = self.manager.get_block_rewards(2_100_001)
        self.assertEqual(rewards["mining_reward"], 135)  # Halved reward
        self.assertEqual(rewards["halvings"], 1)

    def test_instamine_protection(self):
        """Test instamine protection"""
        # Mine blocks up to instamine protection
        for _ in range(INSTAMINE_PROTECTION):
            self.manager.mine_block()
        
        # Verify rewards are reduced during instamine protection
        rewards = self.manager.get_block_rewards(INSTAMINE_PROTECTION)
        self.assertLess(rewards["pow"], POW_REWARD)
        self.assertLess(rewards["p2e"], P2E_REWARD)
        
        # Verify difficulty is higher during instamine protection
        protection_difficulty = self.manager.get_difficulty()
        self.assertGreater(protection_difficulty, self.manager.get_difficulty())

    def test_halving(self):
        """Test block reward halving"""
        # Mine blocks up to first halving
        blocks_to_halving = HALVING_PERIOD // BLOCK_TIME
        for _ in range(blocks_to_halving):
            self.manager.mine_block()
        
        # Verify rewards are halved
        rewards = self.manager.get_block_rewards(blocks_to_halving)
        self.assertEqual(rewards["pow"], POW_REWARD // 2)
        self.assertEqual(rewards["p2e"], P2E_REWARD // 2)
        
        # Verify total supply is tracked
        total_supply = self.manager.get_total_supply()
        self.assertGreater(total_supply, 0)

    def test_asset_creation(self):
        """Test asset creation and tracking"""
        # Create asset
        asset = self.wallet.create_asset(
            name="TEST_ASSET",
            amount=1000000,
            owner=self.address1,
            ipfs_hash="QmTest123"  # IPFS hash for asset metadata
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify asset
        self.assertEqual(self.wallet.get_balance(self.address1, "TEST_ASSET"), 1000000)
        asset_info = self.wallet.get_asset_info("TEST_ASSET")
        self.assertIsNotNone(asset_info)
        self.assertEqual(asset_info.owner, self.address1)
        self.assertEqual(asset_info.ipfs_hash, "QmTest123")

    def test_asset_transfer(self):
        """Test asset transfer and tracking"""
        # Create asset
        asset = self.wallet.create_asset(
            name="TEST_ASSET",
            amount=1000000,
            owner=self.address1,
            ipfs_hash="QmTest123"
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Transfer asset
        transfer_amount = 500000
        self.wallet.transfer_asset(
            name="TEST_ASSET",
            from_owner=self.address1,
            to_owner=self.address2,
            amount=transfer_amount
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify balances
        self.assertEqual(self.wallet.get_balance(self.address1, "TEST_ASSET"), 500000)
        self.assertEqual(self.wallet.get_balance(self.address2, "TEST_ASSET"), 500000)

    def test_nft_creation(self):
        """Test NFT creation and tracking"""
        # Create NFT
        nft = self.wallet.create_asset(
            name="TEST_NFT#1",
            amount=1,  # NFTs are always quantity 1
            owner=self.address1,
            ipfs_hash="QmNFT123",  # IPFS hash for NFT metadata
            is_nft=True
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify NFT
        self.assertEqual(self.wallet.get_balance(self.address1, "TEST_NFT#1"), 1)
        nft_info = self.wallet.get_asset_info("TEST_NFT#1")
        self.assertIsNotNone(nft_info)
        self.assertEqual(nft_info.owner, self.address1)
        self.assertEqual(nft_info.ipfs_hash, "QmNFT123")
        self.assertTrue(nft_info.is_nft)

    def test_nft_transfer(self):
        """Test NFT transfer and tracking"""
        # Create NFT
        nft = self.wallet.create_asset(
            name="TEST_NFT#1",
            amount=1,
            owner=self.address1,
            ipfs_hash="QmNFT123",
            is_nft=True
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Transfer NFT
        self.wallet.transfer_asset(
            name="TEST_NFT#1",
            from_owner=self.address1,
            to_owner=self.address2,
            amount=1
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify NFT ownership
        self.assertEqual(self.wallet.get_balance(self.address1, "TEST_NFT#1"), 0)
        self.assertEqual(self.wallet.get_balance(self.address2, "TEST_NFT#1"), 1)
        nft_info = self.wallet.get_asset_info("TEST_NFT#1")
        self.assertEqual(nft_info.owner, self.address2)

    def test_ipfs_metadata(self):
        """Test IPFS metadata handling"""
        # Create asset with IPFS metadata
        ipfs_hash = "QmTest123"
        asset = self.wallet.create_asset(
            name="TEST_ASSET",
            amount=1000000,
            owner=self.address1,
            ipfs_hash=ipfs_hash
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify IPFS metadata
        metadata = self.wallet.get_asset_metadata(ipfs_hash)
        self.assertIsNotNone(metadata)
        self.assertIn("ipfs_hash", metadata)
        self.assertEqual(metadata["ipfs_hash"], ipfs_hash)

    def test_ipfs_asset_metadata(self):
        """Test IPFS metadata for assets"""
        # Create asset with detailed IPFS metadata
        ipfs_hash = "QmAsset123"
        asset_metadata = {
            "name": "Test Asset",
            "description": "A test asset on Satoxcoin",
            "image": "ipfs://QmImage123",
            "properties": {
                "category": "test",
                "version": "1.0"
            }
        }
        
        asset = self.wallet.create_asset(
            name="TEST_ASSET",
            amount=1000000,
            owner=self.address1,
            ipfs_hash=ipfs_hash,
            metadata=asset_metadata
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify IPFS metadata
        metadata = self.wallet.get_asset_metadata(ipfs_hash)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata["name"], asset_metadata["name"])
        self.assertEqual(metadata["description"], asset_metadata["description"])
        self.assertEqual(metadata["image"], asset_metadata["image"])
        self.assertEqual(metadata["properties"], asset_metadata["properties"])

    def test_ipfs_nft_metadata(self):
        """Test IPFS metadata for NFTs"""
        # Create NFT with detailed IPFS metadata
        ipfs_hash = "QmNFT123"
        nft_metadata = {
            "name": "Test NFT",
            "description": "A unique NFT on Satoxcoin",
            "image": "ipfs://QmNFTImage123",
            "attributes": [
                {
                    "trait_type": "Rarity",
                    "value": "Legendary"
                },
                {
                    "trait_type": "Level",
                    "value": 1
                }
            ]
        }
        
        nft = self.wallet.create_asset(
            name="TEST_NFT#1",
            amount=1,
            owner=self.address1,
            ipfs_hash=ipfs_hash,
            metadata=nft_metadata,
            is_nft=True
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify IPFS metadata
        metadata = self.wallet.get_asset_metadata(ipfs_hash)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata["name"], nft_metadata["name"])
        self.assertEqual(metadata["description"], nft_metadata["description"])
        self.assertEqual(metadata["image"], nft_metadata["image"])
        self.assertEqual(metadata["attributes"], nft_metadata["attributes"])

    def test_ipfs_metadata_update(self):
        """Test updating IPFS metadata"""
        # Create asset with initial IPFS metadata
        initial_ipfs_hash = "QmInitial123"
        asset = self.wallet.create_asset(
            name="TEST_ASSET",
            amount=1000000,
            owner=self.address1,
            ipfs_hash=initial_ipfs_hash
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Update IPFS metadata
        new_ipfs_hash = "QmUpdated123"
        self.wallet.update_asset_metadata(
            name="TEST_ASSET",
            ipfs_hash=new_ipfs_hash
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify updated metadata
        asset_info = self.wallet.get_asset_info("TEST_ASSET")
        self.assertEqual(asset_info.ipfs_hash, new_ipfs_hash)
        
        # Verify old metadata is still accessible
        old_metadata = self.wallet.get_asset_metadata(initial_ipfs_hash)
        self.assertIsNotNone(old_metadata)
        
        # Verify new metadata
        new_metadata = self.wallet.get_asset_metadata(new_ipfs_hash)
        self.assertIsNotNone(new_metadata)

    def test_ipfs_metadata_verification(self):
        """Test IPFS metadata verification"""
        # Create asset with IPFS metadata
        ipfs_hash = "QmTest123"
        asset = self.wallet.create_asset(
            name="TEST_ASSET",
            amount=1000000,
            owner=self.address1,
            ipfs_hash=ipfs_hash
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify IPFS metadata integrity
        is_valid = self.wallet.verify_ipfs_metadata(ipfs_hash)
        self.assertTrue(is_valid)
        
        # Test with invalid IPFS hash
        invalid_hash = "QmInvalid123"
        is_valid = self.wallet.verify_ipfs_metadata(invalid_hash)
        self.assertFalse(is_valid)

    def test_ipfs_metadata_pinning(self):
        """Test IPFS metadata pinning"""
        # Create asset with IPFS metadata
        ipfs_hash = "QmTest123"
        asset = self.wallet.create_asset(
            name="TEST_ASSET",
            amount=1000000,
            owner=self.address1,
            ipfs_hash=ipfs_hash
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Pin IPFS metadata
        is_pinned = self.wallet.pin_ipfs_metadata(ipfs_hash)
        self.assertTrue(is_pinned)
        
        # Verify pin status
        pin_status = self.wallet.get_ipfs_pin_status(ipfs_hash)
        self.assertTrue(pin_status["is_pinned"])
        
        # Unpin IPFS metadata
        is_unpinned = self.wallet.unpin_ipfs_metadata(ipfs_hash)
        self.assertTrue(is_unpinned)
        
        # Verify unpin status
        pin_status = self.wallet.get_ipfs_pin_status(ipfs_hash)
        self.assertFalse(pin_status["is_pinned"])

    def test_block_confirmations(self):
        """Test block confirmation requirements"""
        # Create and send transaction
        amount = 500000000000000000  # 0.5 SATOX
        txid = self.wallet.send_transaction(
            from_address=self.address1,
            to_address=self.address2,
            amount=amount
        )
        
        # Mine block
        self.manager.mine_block()
        
        # Verify transaction is not confirmed yet
        tx = self.wallet.get_transaction(txid)
        self.assertEqual(tx.status, "pending")
        
        # Mine required confirmations
        for _ in range(BLOCK_CONFIRMATIONS):
            self.manager.mine_block()
        
        # Verify transaction is now confirmed
        tx = self.wallet.get_transaction(txid)
        self.assertEqual(tx.status, "confirmed")
        
        # Verify block confirmations
        confirmations = self.manager.get_transaction_confirmations(txid)
        self.assertEqual(confirmations, BLOCK_CONFIRMATIONS)

    def test_halving_period(self):
        """Test halving period calculation."""
        # Test halving period in seconds (2,100,000 blocks * 60 seconds)
        expected_period = 2_100_000 * 60
        self.assertEqual(self.manager.get_halving_period(), expected_period)

def test_performance_metrics():
    """Test blockchain manager performance"""
    wallet = MockWallet()
    wallet.connect()
    
    # Create addresses with initial balance
    address1 = wallet.create_address()
    address2 = wallet.create_address()
    wallet._addresses[address1] = {"SATOX": 1000000000000000000}  # 1 SATOX
    
    manager = BlockchainManager(wallet)
    
    # Test KawPoW mining performance
    start_time = time.time()
    manager.mine_block()
    mining_time = time.time() - start_time
    assert abs(mining_time - BLOCK_TIME) < 1.0, "Block mining time incorrect"
    
    # Test DGW retargeting performance
    start_time = time.time()
    for _ in range(DGW_WINDOW):
        manager.mine_block()
    retarget_time = time.time() - start_time
    assert retarget_time < BLOCK_TIME * (DGW_WINDOW + 1), "DGW retargeting too slow"
    
    # Test transaction processing performance
    start_time = time.time()
    for _ in range(10):
        wallet.send_transaction(
            from_address=address1,
            to_address=address2,
            amount=1000000
        )
        manager.mine_block()
    transaction_time = time.time() - start_time
    assert transaction_time < BLOCK_TIME * 11, "Transaction processing too slow"
    
    # Test asset operations performance
    start_time = time.time()
    for i in range(10):
        wallet.create_asset(
            name=f"TEST_ASSET_{i}",
            amount=1000000,
            owner=address1,
            ipfs_hash=f"QmTest{i}"
        )
        manager.mine_block()
    asset_time = time.time() - start_time
    assert asset_time < BLOCK_TIME * 11, "Asset operations too slow"

if __name__ == '__main__':
    unittest.main() 