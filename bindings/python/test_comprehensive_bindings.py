#!/usr/bin/env python3
"""
MIT License

Copyright (c) 2025 Satoxcoin Core Developer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

"""
Comprehensive test suite for Satox SDK Python bindings.

This test suite verifies that all major functionality of the Python bindings
works correctly and that the native C++ SDK is properly accessible.
"""

import unittest
import sys
import os
import tempfile
import json
from typing import Dict, Any

# Add the parent directory to the path so we can import satox_bindings
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    import satox_bindings as satox
    from satox_bindings import (
        SDK, CoreManager, DatabaseManager, BlockchainManager, AssetManager,
        NFTManager, IPFSManager, NetworkManager, SecurityManager, WalletManager,
        CoreConfig, DatabaseConfig, BlockchainConfig, AssetConfig, NFTConfig,
        IPFSConfig, NetworkConfig, SecurityConfig, WalletConfig,
        LogLevel, DatabaseType, NetworkType, SecurityLevel,
        initialize_sdk, create_simple_wallet, get_blockchain_info, create_asset
    )
    BINDINGS_AVAILABLE = True
except ImportError as e:
    print(f"Warning: Could not import satox_bindings: {e}")
    BINDINGS_AVAILABLE = False


class TestSatoxBindings(unittest.TestCase):
    """Test suite for Satox SDK Python bindings."""
    
    @classmethod
    def setUpClass(cls):
        """Set up test environment."""
        if not BINDINGS_AVAILABLE:
            raise unittest.SkipTest("Satox bindings not available")
        
        # Initialize SDK for testing
        cls.sdk = SDK.getInstance()
        cls.config = {
            "name": "test_sdk",
            "enableLogging": True,
            "logPath": "/tmp/satox_test.log"
        }
        
        # Initialize SDK
        success = cls.sdk.initialize(cls.config)
        if not success:
            raise RuntimeError("Failed to initialize SDK for testing")
    
    @classmethod
    def tearDownClass(cls):
        """Clean up test environment."""
        if hasattr(cls, 'sdk'):
            cls.sdk.shutdown()
    
    def test_sdk_initialization(self):
        """Test SDK initialization and basic functionality."""
        self.assertTrue(self.sdk.isInitialized())
        self.assertIsNotNone(self.sdk.getVersion())
        print(f"SDK Version: {self.sdk.getVersion()}")
    
    def test_core_manager(self):
        """Test Core Manager functionality."""
        core = self.sdk.getCoreManager()
        self.assertIsNotNone(core)
        
        # Test basic functionality
        self.assertTrue(core.isInitialized())
        status = core.getStatus()
        self.assertIsNotNone(status)
        stats = core.getStats()
        self.assertIsNotNone(stats)
        self.assertTrue(core.isHealthy())
        
        health_status = core.getHealthStatus()
        self.assertIsNotNone(health_status)
    
    def test_database_manager(self):
        """Test Database Manager functionality."""
        db = self.sdk.getDatabaseManager()
        self.assertIsNotNone(db)
        
        # Test basic functionality
        self.assertTrue(db.isInitialized())
        
        # Test database operations
        test_db_name = "test_database"
        success = db.createDatabase(test_db_name)
        self.assertTrue(success)
        
        databases = db.listDatabases()
        self.assertIsNotNone(databases)
        self.assertIn(test_db_name, databases)
        
        # Test cleanup
        success = db.deleteDatabase(test_db_name)
        self.assertTrue(success)
        
        stats = db.getStats()
        self.assertIsNotNone(stats)
        self.assertTrue(db.isHealthy())
    
    def test_blockchain_manager(self):
        """Test Blockchain Manager functionality."""
        blockchain = self.sdk.getBlockchainManager()
        self.assertIsNotNone(blockchain)
        
        # Test basic functionality
        self.assertTrue(blockchain.isInitialized())
        
        # Test blockchain operations
        height = blockchain.getBlockHeight()
        self.assertIsNotNone(height)
        self.assertGreaterEqual(height, 0)
        
        best_hash = blockchain.getBestBlockHash()
        self.assertIsNotNone(best_hash)
        
        # Test block info
        if best_hash:
            block_info = blockchain.getBlockInfo(best_hash)
            self.assertIsNotNone(block_info)
        
        stats = blockchain.getStats()
        self.assertIsNotNone(stats)
        self.assertTrue(blockchain.isHealthy())
    
    def test_asset_manager(self):
        """Test Asset Manager functionality."""
        asset = self.sdk.getAssetManager()
        self.assertIsNotNone(asset)
        
        # Test basic functionality
        self.assertTrue(asset.isInitialized())
        
        # Test asset creation
        asset_config = {
            "name": "Test Asset",
            "symbol": "TEST",
            "decimals": 8,
            "total_supply": 1000000,
            "description": "Test asset for unit testing"
        }
        owner_address = "test_owner_address"
        
        result = asset.createAsset(asset_config, owner_address)
        self.assertIsNotNone(result)
        
        # Test asset info retrieval
        if result and "id" in result:
            asset_info = asset.getAssetInfo(result["id"])
            self.assertIsNotNone(asset_info)
        
        stats = asset.getStats()
        self.assertIsNotNone(stats)
        self.assertTrue(asset.isHealthy())
    
    def test_nft_manager(self):
        """Test NFT Manager functionality."""
        nft = self.sdk.getNFTManager()
        self.assertIsNotNone(nft)
        
        # Test basic functionality
        self.assertTrue(nft.isInitialized())
        
        # Test NFT creation
        asset_id = "test_asset_id"
        metadata = {
            "name": "Test NFT",
            "description": "Test NFT for unit testing",
            "image": "ipfs://test_image_hash"
        }
        owner_address = "test_owner_address"
        
        result = nft.createNFT(asset_id, metadata, owner_address)
        self.assertIsNotNone(result)
        
        # Test NFT info retrieval
        if result and "id" in result:
            nft_info = nft.getNFTInfo(result["id"])
            self.assertIsNotNone(nft_info)
        
        stats = nft.getStats()
        self.assertIsNotNone(stats)
        self.assertTrue(nft.isHealthy())
    
    def test_ipfs_manager(self):
        """Test IPFS Manager functionality."""
        ipfs = self.sdk.getIPFSManager()
        self.assertIsNotNone(ipfs)
        
        # Test basic functionality
        self.assertTrue(ipfs.isInitialized())
        
        # Test IPFS operations
        test_data = b"Hello, IPFS! This is test data."
        filename = "test_file.txt"
        
        result = ipfs.uploadData(test_data, filename)
        self.assertIsNotNone(result)
        
        # Test file info retrieval
        if result and "hash" in result:
            file_info = ipfs.getFileInfo(result["hash"])
            self.assertIsNotNone(file_info)
            
            # Test pinning
            success = ipfs.pinFile(result["hash"])
            self.assertTrue(success)
        
        stats = ipfs.getStats()
        self.assertIsNotNone(stats)
        self.assertTrue(ipfs.isHealthy())
    
    def test_network_manager(self):
        """Test Network Manager functionality."""
        network = self.sdk.getNetworkManager()
        self.assertIsNotNone(network)
        
        # Test basic functionality
        self.assertTrue(network.isInitialized())
        
        # Test network operations
        success = network.start()
        self.assertTrue(success)
        
        peers = network.getPeers()
        self.assertIsNotNone(peers)
        
        stats = network.getStats()
        self.assertIsNotNone(stats)
        
        # Test peer management
        success = network.addPeer("127.0.0.1", 8333)
        self.assertTrue(success)
        
        success = network.removePeer("127.0.0.1", 8333)
        self.assertTrue(success)
        
        success = network.stop()
        self.assertTrue(success)
        
        self.assertTrue(network.isHealthy())
    
    def test_security_manager(self):
        """Test Security Manager functionality."""
        security = self.sdk.getSecurityManager()
        self.assertIsNotNone(security)
        
        # Test basic functionality
        self.assertTrue(security.isInitialized())
        
        # Test key generation
        success = security.generatePQCKeyPair("DEFAULT")
        self.assertTrue(success)
        
        # Test signing and verification
        test_data = "Hello, Security! This is test data."
        signature = security.signWithPQC("DEFAULT", test_data)
        self.assertIsNotNone(signature)
        
        success = security.verifyWithPQC("DEFAULT", test_data, signature)
        self.assertTrue(success)
        
        # Test encryption and decryption
        encrypted = security.encryptWithPQC("DEFAULT", test_data)
        self.assertIsNotNone(encrypted)
        
        decrypted = security.decryptWithPQC("DEFAULT", encrypted)
        self.assertIsNotNone(decrypted)
        
        stats = security.getStats()
        self.assertIsNotNone(stats)
        self.assertTrue(security.isHealthy())
    
    def test_wallet_manager(self):
        """Test Wallet Manager functionality."""
        wallet = self.sdk.getWalletManager()
        self.assertIsNotNone(wallet)
        
        # Test basic functionality
        self.assertTrue(wallet.isInitialized())
        
        # Test wallet creation
        wallet_name = "test_wallet"
        success = wallet.createWallet(wallet_name)
        self.assertTrue(success)
        
        # Test wallet info retrieval
        wallet_info = wallet.getWalletInfo(wallet_name)
        self.assertIsNotNone(wallet_info)
        
        # Test balance retrieval
        balance = wallet.getBalance(wallet_name)
        self.assertIsNotNone(balance)
        
        # Test transaction history
        history = wallet.getTransactionHistory(wallet_name)
        self.assertIsNotNone(history)
        
        stats = wallet.getStats()
        self.assertIsNotNone(stats)
        self.assertTrue(wallet.isHealthy())
    
    def test_configuration_classes(self):
        """Test configuration classes."""
        # Test CoreConfig
        core_config = CoreConfig()
        core_config.name = "test_core"
        core_config.enableLogging = True
        core_config.logPath = "/tmp/test.log"
        core_config.logLevel = LogLevel.INFO
        
        self.assertEqual(core_config.name, "test_core")
        self.assertTrue(core_config.enableLogging)
        self.assertEqual(core_config.logPath, "/tmp/test.log")
        self.assertEqual(core_config.logLevel, LogLevel.INFO)
        
        # Test DatabaseConfig
        db_config = DatabaseConfig()
        db_config.name = "test_db"
        db_config.enableLogging = True
        db_config.maxConnections = 10
        db_config.connectionTimeout = 30
        
        self.assertEqual(db_config.name, "test_db")
        self.assertEqual(db_config.maxConnections, 10)
        self.assertEqual(db_config.connectionTimeout, 30)
        
        # Test BlockchainConfig
        bc_config = BlockchainConfig()
        bc_config.network = NetworkType.TESTNET
        bc_config.enableLogging = True
        
        self.assertEqual(bc_config.network, NetworkType.TESTNET)
    
    def test_enums(self):
        """Test enum values."""
        # Test LogLevel
        self.assertEqual(LogLevel.TRACE.value, 0)
        self.assertEqual(LogLevel.DEBUG.value, 1)
        self.assertEqual(LogLevel.INFO.value, 2)
        self.assertEqual(LogLevel.WARN.value, 3)
        self.assertEqual(LogLevel.ERROR.value, 4)
        self.assertEqual(LogLevel.CRITICAL.value, 5)
        
        # Test DatabaseType
        self.assertEqual(DatabaseType.SQLITE.value, 0)
        self.assertEqual(DatabaseType.POSTGRESQL.value, 1)
        self.assertEqual(DatabaseType.MYSQL.value, 2)
        self.assertEqual(DatabaseType.ROCKSDB.value, 3)
        self.assertEqual(DatabaseType.REDIS.value, 4)
        self.assertEqual(DatabaseType.MONGODB.value, 5)
        self.assertEqual(DatabaseType.MEMORY.value, 6)
        
        # Test NetworkType
        self.assertEqual(NetworkType.MAINNET.value, 0)
        self.assertEqual(NetworkType.TESTNET.value, 1)
        self.assertEqual(NetworkType.DEVNET.value, 2)
        
        # Test SecurityLevel
        self.assertEqual(SecurityLevel.LOW.value, 0)
        self.assertEqual(SecurityLevel.MEDIUM.value, 1)
        self.assertEqual(SecurityLevel.HIGH.value, 2)
        self.assertEqual(SecurityLevel.CRITICAL.value, 3)
    
    def test_convenience_functions(self):
        """Test convenience functions."""
        # Test initialize_sdk
        test_sdk = initialize_sdk({"name": "test_convenience"})
        self.assertIsNotNone(test_sdk)
        self.assertTrue(test_sdk.isInitialized())
        
        # Test get_blockchain_info
        info = get_blockchain_info(test_sdk)
        self.assertIsNotNone(info)
        self.assertIn("height", info)
        self.assertIn("best_block_hash", info)
        self.assertIn("is_healthy", info)
        
        # Test create_simple_wallet
        wallet_info = create_simple_wallet("test_convenience_wallet", test_sdk)
        self.assertIsNotNone(wallet_info)
        
        # Test create_asset
        asset_info = create_asset("Test Asset", "TEST", "test_owner", test_sdk)
        self.assertIsNotNone(asset_info)
        
        # Cleanup
        test_sdk.shutdown()
    
    def test_error_handling(self):
        """Test error handling."""
        # Test invalid operations
        with self.assertRaises(Exception):
            # Try to use SDK without initialization
            sdk = SDK.getInstance()
            core = sdk.getCoreManager()
            core.getStatus()  # Should fail if not initialized
    
    def test_performance(self):
        """Test basic performance characteristics."""
        import time
        
        # Test SDK initialization time
        start_time = time.time()
        test_sdk = SDK.getInstance()
        test_sdk.initialize({})
        init_time = time.time() - start_time
        
        print(f"SDK initialization time: {init_time:.3f} seconds")
        self.assertLess(init_time, 5.0)  # Should initialize in under 5 seconds
        
        # Test manager access time
        start_time = time.time()
        for _ in range(100):
            core = test_sdk.getCoreManager()
            blockchain = test_sdk.getBlockchainManager()
            wallet = test_sdk.getWalletManager()
        access_time = time.time() - start_time
        
        print(f"Manager access time (100 iterations): {access_time:.3f} seconds")
        self.assertLess(access_time, 1.0)  # Should access managers quickly
        
        test_sdk.shutdown()


def run_tests():
    """Run the test suite."""
    if not BINDINGS_AVAILABLE:
        print("Skipping tests: Satox bindings not available")
        return
    
    print("Running Satox SDK Python Bindings Test Suite")
    print("=" * 50)
    
    # Create test suite
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSatoxBindings)
    
    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    # Print summary
    print("\n" + "=" * 50)
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print(f"Skipped: {len(result.skipped)}")
    
    if result.failures:
        print("\nFailures:")
        for test, traceback in result.failures:
            print(f"  {test}: {traceback}")
    
    if result.errors:
        print("\nErrors:")
        for test, traceback in result.errors:
            print(f"  {test}: {traceback}")
    
    return result.wasSuccessful()


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1) 