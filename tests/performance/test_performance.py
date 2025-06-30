"""
Performance test suite for Satox SDK.
"""

import pytest
import asyncio
import time
import statistics
from typing import Dict, List, Any
from datetime import datetime

from satox.wallet.wallet import Wallet
from satox.security.quantum_crypto import QuantumCrypto
from satox.blockchain.blockchain_manager import BlockchainManager
from satox.assets.asset_manager import AssetManager
from satox.core.exceptions import SatoxError

# Test fixtures
@pytest.fixture
async def blockchain_manager():
    """Create blockchain manager instance."""
    manager = BlockchainManager()
    await manager.connect()
    yield manager
    await manager.disconnect()

@pytest.fixture
def security_manager():
    """Create security manager instance."""
    return QuantumCrypto()

@pytest.fixture
async def asset_manager(blockchain_manager):
    """Create asset manager instance."""
    return AssetManager(blockchain_manager)

@pytest.fixture
def wallet(security_manager):
    """Create wallet instance."""
    return Wallet(security_manager)

# Performance metrics
class PerformanceMetrics:
    """Performance metrics collection and analysis."""
    
    def __init__(self):
        """Initialize metrics."""
        self.metrics: Dict[str, List[float]] = {}
    
    def add_metric(self, name: str, value: float):
        """Add a metric value."""
        if name not in self.metrics:
            self.metrics[name] = []
        self.metrics[name].append(value)
    
    def get_statistics(self, name: str) -> Dict[str, float]:
        """Get statistics for a metric."""
        if name not in self.metrics:
            return {}
        
        values = self.metrics[name]
        return {
            'min': min(values),
            'max': max(values),
            'mean': statistics.mean(values),
            'median': statistics.median(values),
            'stdev': statistics.stdev(values) if len(values) > 1 else 0
        }

# Performance tests
class TestWalletPerformance:
    """Test wallet performance."""
    
    @pytest.mark.asyncio
    async def test_wallet_creation_performance(self, wallet):
        """Test wallet creation performance."""
        metrics = PerformanceMetrics()
        
        # Test wallet creation
        for _ in range(100):
            start_time = time.time()
            new_wallet = Wallet()
            end_time = time.time()
            metrics.add_metric('wallet_creation', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('wallet_creation')
        assert stats['mean'] < 100  # Should take less than 100ms
    
    @pytest.mark.asyncio
    async def test_wallet_encryption_performance(self, wallet):
        """Test wallet encryption performance."""
        metrics = PerformanceMetrics()
        
        # Test data
        test_data = {
            'address': wallet.get_address(),
            'balance': 100.0,
            'transactions': []
        }
        
        # Test encryption
        for _ in range(100):
            start_time = time.time()
            encrypted = wallet.encrypt(test_data)
            end_time = time.time()
            metrics.add_metric('wallet_encryption', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('wallet_encryption')
        assert stats['mean'] < 50  # Should take less than 50ms
    
    @pytest.mark.asyncio
    async def test_transaction_signing_performance(self, wallet):
        """Test transaction signing performance."""
        metrics = PerformanceMetrics()
        
        # Test transaction
        transaction = {
            'from': wallet.get_address(),
            'to': '0x1234567890abcdef',
            'amount': 10.0,
            'timestamp': datetime.utcnow().isoformat()
        }
        
        # Test signing
        for _ in range(100):
            start_time = time.time()
            signed_tx = wallet.sign_transaction(transaction)
            end_time = time.time()
            metrics.add_metric('transaction_signing', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('transaction_signing')
        assert stats['mean'] < 30  # Should take less than 30ms

class TestSecurityManagerPerformance:
    """Test security manager performance."""
    
    def test_key_generation_performance(self, security_manager):
        """Test key generation performance."""
        metrics = PerformanceMetrics()
        
        # Test key generation
        for _ in range(100):
            start_time = time.time()
            public_key, secret_key = security_manager.generate_keypair()
            end_time = time.time()
            metrics.add_metric('key_generation', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('key_generation')
        assert stats['mean'] < 20  # Should take less than 20ms
    
    def test_message_encryption_performance(self, security_manager):
        """Test message encryption performance."""
        metrics = PerformanceMetrics()
        
        # Generate keys
        public_key, secret_key = security_manager.generate_keypair()
        
        # Test message
        message = b"Test message" * 100  # Larger message for better testing
        
        # Test encryption
        for _ in range(100):
            start_time = time.time()
            encrypted = security_manager.encrypt_message(message, public_key)
            end_time = time.time()
            metrics.add_metric('message_encryption', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('message_encryption')
        assert stats['mean'] < 40  # Should take less than 40ms

class TestBlockchainManagerPerformance:
    """Test blockchain manager performance."""
    
    @pytest.mark.asyncio
    async def test_block_retrieval_performance(self, blockchain_manager):
        """Test block retrieval performance."""
        metrics = PerformanceMetrics()
        
        # Get latest block
        latest_block = await blockchain_manager.get_latest_block()
        block_number = latest_block['number']
        
        # Test block retrieval
        for _ in range(100):
            start_time = time.time()
            block = await blockchain_manager.get_block(block_number)
            end_time = time.time()
            metrics.add_metric('block_retrieval', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('block_retrieval')
        assert stats['mean'] < 200  # Should take less than 200ms
    
    @pytest.mark.asyncio
    async def test_transaction_broadcast_performance(self, blockchain_manager, wallet):
        """Test transaction broadcast performance."""
        metrics = PerformanceMetrics()
        
        # Test transaction
        transaction = {
            'from': wallet.get_address(),
            'to': '0x1234567890abcdef',
            'amount': 10.0,
            'timestamp': datetime.utcnow().isoformat()
        }
        
        # Sign transaction
        signed_tx = wallet.sign_transaction(transaction)
        
        # Test broadcasting
        for _ in range(100):
            start_time = time.time()
            tx_hash = await blockchain_manager.broadcast_transaction(signed_tx)
            end_time = time.time()
            metrics.add_metric('transaction_broadcast', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('transaction_broadcast')
        assert stats['mean'] < 300  # Should take less than 300ms

class TestAssetManagerPerformance:
    """Test asset manager performance."""
    
    @pytest.mark.asyncio
    async def test_asset_creation_performance(self, asset_manager, wallet):
        """Test asset creation performance."""
        metrics = PerformanceMetrics()
        
        # Test asset creation
        for _ in range(100):
            start_time = time.time()
            asset_id = await asset_manager.create_asset(
                name="Test Asset",
                symbol="TEST",
                asset_type="token",
                total_supply=1000000,
                owner=wallet.get_address()
            )
            end_time = time.time()
            metrics.add_metric('asset_creation', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('asset_creation')
        assert stats['mean'] < 250  # Should take less than 250ms
    
    @pytest.mark.asyncio
    async def test_asset_transfer_performance(self, asset_manager, wallet):
        """Test asset transfer performance."""
        metrics = PerformanceMetrics()
        
        # Create test asset
        asset_id = await asset_manager.create_asset(
            name="Test Asset",
            symbol="TEST",
            asset_type="token",
            total_supply=1000000,
            owner=wallet.get_address()
        )
        
        # Test transfer
        for _ in range(100):
            start_time = time.time()
            tx_hash = await asset_manager.transfer_asset(
                asset_id=asset_id,
                from_address=wallet.get_address(),
                to_address="0x1234567890abcdef",
                amount=100
            )
            end_time = time.time()
            metrics.add_metric('asset_transfer', (end_time - start_time) * 1000)
        
        stats = metrics.get_statistics('asset_transfer')
        assert stats['mean'] < 200  # Should take less than 200ms

# Concurrent performance tests
class TestConcurrentPerformance:
    """Test concurrent performance."""
    
    @pytest.mark.asyncio
    async def test_concurrent_wallet_operations(self, wallet):
        """Test concurrent wallet operations."""
        metrics = PerformanceMetrics()
        
        async def wallet_operation():
            # Create transaction
            transaction = {
                'from': wallet.get_address(),
                'to': '0x1234567890abcdef',
                'amount': 10.0,
                'timestamp': datetime.utcnow().isoformat()
            }
            
            # Sign transaction
            start_time = time.time()
            signed_tx = wallet.sign_transaction(transaction)
            end_time = time.time()
            metrics.add_metric('concurrent_signing', (end_time - start_time) * 1000)
        
        # Run concurrent operations
        tasks = [wallet_operation() for _ in range(100)]
        await asyncio.gather(*tasks)
        
        stats = metrics.get_statistics('concurrent_signing')
        assert stats['mean'] < 40  # Should take less than 40ms
    
    @pytest.mark.asyncio
    async def test_concurrent_blockchain_operations(self, blockchain_manager):
        """Test concurrent blockchain operations."""
        metrics = PerformanceMetrics()
        
        async def blockchain_operation():
            # Get latest block
            start_time = time.time()
            latest_block = await blockchain_manager.get_latest_block()
            end_time = time.time()
            metrics.add_metric('concurrent_block_retrieval', (end_time - start_time) * 1000)
        
        # Run concurrent operations
        tasks = [blockchain_operation() for _ in range(100)]
        await asyncio.gather(*tasks)
        
        stats = metrics.get_statistics('concurrent_block_retrieval')
        assert stats['mean'] < 250  # Should take less than 250ms

# Resource usage tests
class TestResourceUsage:
    """Test resource usage."""
    
    @pytest.mark.asyncio
    async def test_memory_usage(self, wallet, blockchain_manager):
        """Test memory usage."""
        import psutil
        import os
        
        process = psutil.Process(os.getpid())
        initial_memory = process.memory_info().rss
        
        # Perform operations
        for _ in range(1000):
            transaction = {
                'from': wallet.get_address(),
                'to': '0x1234567890abcdef',
                'amount': 10.0,
                'timestamp': datetime.utcnow().isoformat()
            }
            signed_tx = wallet.sign_transaction(transaction)
            await blockchain_manager.broadcast_transaction(signed_tx)
        
        final_memory = process.memory_info().rss
        memory_increase = (final_memory - initial_memory) / 1024 / 1024  # MB
        
        assert memory_increase < 100  # Should use less than 100MB additional memory
    
    @pytest.mark.asyncio
    async def test_cpu_usage(self, wallet, blockchain_manager):
        """Test CPU usage."""
        import psutil
        import os
        
        process = psutil.Process(os.getpid())
        
        # Perform operations
        for _ in range(1000):
            transaction = {
                'from': wallet.get_address(),
                'to': '0x1234567890abcdef',
                'amount': 10.0,
                'timestamp': datetime.utcnow().isoformat()
            }
            signed_tx = wallet.sign_transaction(transaction)
            await blockchain_manager.broadcast_transaction(signed_tx)
        
        cpu_percent = process.cpu_percent(interval=1)
        assert cpu_percent < 80  # Should use less than 80% CPU

if __name__ == '__main__':
    pytest.main(['-v', 'test_performance.py']) 