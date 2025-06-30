"""
Performance benchmarks for Satox SDK.
"""

import asyncio
import time
import statistics
from typing import Dict, Any, List, Callable
import logging
from datetime import datetime
import json
import os

from satox.blockchain.blockchain_manager import BlockchainManager
from satox.blockchain.sync_manager import SyncManager
from satox.wallet.wallet import Wallet
from satox.security.quantum_crypto import QuantumCrypto
from satox.assets.asset_manager import AssetManager

class Benchmark:
    """Base class for benchmarks."""
    
    def __init__(self, name: str, iterations: int = 100):
        """Initialize benchmark.
        
        Args:
            name: Benchmark name
            iterations: Number of iterations
        """
        self.name = name
        self.iterations = iterations
        self.logger = logging.getLogger(__name__)
        self.results: List[float] = []
        self.start_time = None
        self.end_time = None

    async def setup(self):
        """Setup benchmark."""
        pass

    async def teardown(self):
        """Teardown benchmark."""
        pass

    async def run(self):
        """Run benchmark."""
        self.start_time = time.time()
        self.results = []
        
        await self.setup()
        
        for i in range(self.iterations):
            start = time.time()
            await self.iteration()
            end = time.time()
            self.results.append(end - start)
            
            if (i + 1) % 10 == 0:
                self.logger.info(f"Completed {i + 1}/{self.iterations} iterations")
        
        await self.teardown()
        
        self.end_time = time.time()
        return self.get_results()

    async def iteration(self):
        """Run single iteration."""
        raise NotImplementedError

    def get_results(self) -> Dict[str, Any]:
        """Get benchmark results.
        
        Returns:
            Dictionary of results
        """
        return {
            'name': self.name,
            'iterations': self.iterations,
            'total_time': self.end_time - self.start_time,
            'mean': statistics.mean(self.results),
            'median': statistics.median(self.results),
            'stddev': statistics.stdev(self.results) if len(self.results) > 1 else 0,
            'min': min(self.results),
            'max': max(self.results),
            'results': self.results
        }

class BlockchainBenchmark(Benchmark):
    """Blockchain operation benchmarks."""
    
    def __init__(self, blockchain_manager: BlockchainManager, **kwargs):
        """Initialize blockchain benchmark.
        
        Args:
            blockchain_manager: Blockchain manager instance
            **kwargs: Additional arguments
        """
        super().__init__("Blockchain Operations", **kwargs)
        self.blockchain_manager = blockchain_manager

    async def iteration(self):
        """Run blockchain operations."""
        # Get latest block
        await self.blockchain_manager.get_latest_block()
        
        # Get block range
        await self.blockchain_manager.get_blocks(0, 10)
        
        # Get transaction
        await self.blockchain_manager.get_transaction("test_tx")
        
        # Get balance
        await self.blockchain_manager.get_balance("test_address")

class SyncBenchmark(Benchmark):
    """Blockchain sync benchmarks."""
    
    def __init__(self, sync_manager: SyncManager, **kwargs):
        """Initialize sync benchmark.
        
        Args:
            sync_manager: Sync manager instance
            **kwargs: Additional arguments
        """
        super().__init__("Blockchain Sync", **kwargs)
        self.sync_manager = sync_manager

    async def iteration(self):
        """Run sync operations."""
        # Sync blocks
        await self.sync_manager.sync(force=True)
        
        # Get metrics
        self.sync_manager.get_metrics()

class WalletBenchmark(Benchmark):
    """Wallet operation benchmarks."""
    
    def __init__(self, wallet: Wallet, **kwargs):
        """Initialize wallet benchmark.
        
        Args:
            wallet: Wallet instance
            **kwargs: Additional arguments
        """
        super().__init__("Wallet Operations", **kwargs)
        self.wallet = wallet

    async def iteration(self):
        """Run wallet operations."""
        # Create transaction
        await self.wallet.create_transaction(
            "recipient",
            1.0,
            "memo"
        )
        
        # Sign transaction
        await self.wallet.sign_transaction("test_tx")
        
        # Get balance
        await self.wallet.get_balance()
        
        # Get transactions
        await self.wallet.get_transactions()

class SecurityBenchmark(Benchmark):
    """Security operation benchmarks."""
    
    def __init__(self, quantum_crypto: QuantumCrypto, **kwargs):
        """Initialize security benchmark.
        
        Args:
            quantum_crypto: Quantum crypto instance
            **kwargs: Additional arguments
        """
        super().__init__("Security Operations", **kwargs)
        self.quantum_crypto = quantum_crypto

    async def iteration(self):
        """Run security operations."""
        # Generate key pair
        await self.quantum_crypto.generate_key_pair()
        
        # Sign message
        await self.quantum_crypto.sign_message(
            "test_message",
            "test_private_key"
        )
        
        # Verify signature
        await self.quantum_crypto.verify_signature(
            "test_message",
            "test_signature",
            "test_public_key"
        )
        
        # Encrypt message
        await self.quantum_crypto.encrypt_message(
            "test_message",
            "test_public_key"
        )
        
        # Decrypt message
        await self.quantum_crypto.decrypt_message(
            "test_ciphertext",
            "test_private_key"
        )

class AssetBenchmark(Benchmark):
    """Asset operation benchmarks."""
    
    def __init__(self, asset_manager: AssetManager, **kwargs):
        """Initialize asset benchmark.
        
        Args:
            asset_manager: Asset manager instance
            **kwargs: Additional arguments
        """
        super().__init__("Asset Operations", **kwargs)
        self.asset_manager = asset_manager

    async def iteration(self):
        """Run asset operations."""
        # Create asset
        await self.asset_manager.create_asset(
            "test_asset",
            "Test Asset",
            "Test Description"
        )
        
        # Get asset
        await self.asset_manager.get_asset("test_asset")
        
        # Transfer asset
        await self.asset_manager.transfer_asset(
            "test_asset",
            "recipient",
            1.0
        )
        
        # Get asset balance
        await self.asset_manager.get_asset_balance(
            "test_asset",
            "test_address"
        )

class BenchmarkRunner:
    """Benchmark runner."""
    
    def __init__(self, output_dir: str = "benchmarks"):
        """Initialize benchmark runner.
        
        Args:
            output_dir: Output directory for results
        """
        self.output_dir = output_dir
        self.logger = logging.getLogger(__name__)
        self.benchmarks: List[Benchmark] = []
        
        # Create output directory
        os.makedirs(output_dir, exist_ok=True)

    def add_benchmark(self, benchmark: Benchmark):
        """Add benchmark.
        
        Args:
            benchmark: Benchmark instance
        """
        self.benchmarks.append(benchmark)

    async def run_all(self):
        """Run all benchmarks."""
        results = []
        
        for benchmark in self.benchmarks:
            self.logger.info(f"Running benchmark: {benchmark.name}")
            result = await benchmark.run()
            results.append(result)
            
            # Save results
            self._save_results(result)
        
        return results

    def _save_results(self, result: Dict[str, Any]):
        """Save benchmark results.
        
        Args:
            result: Benchmark results
        """
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"{self.output_dir}/{result['name']}_{timestamp}.json"
        
        with open(filename, 'w') as f:
            json.dump(result, f, indent=2)
        
        self.logger.info(f"Saved results to {filename}")

async def run_benchmarks():
    """Run all benchmarks."""
    # Initialize components
    blockchain_manager = BlockchainManager()
    sync_manager = SyncManager(blockchain_manager)
    wallet = Wallet()
    quantum_crypto = QuantumCrypto()
    asset_manager = AssetManager()
    
    # Create benchmark runner
    runner = BenchmarkRunner()
    
    # Add benchmarks
    runner.add_benchmark(BlockchainBenchmark(blockchain_manager))
    runner.add_benchmark(SyncBenchmark(sync_manager))
    runner.add_benchmark(WalletBenchmark(wallet))
    runner.add_benchmark(SecurityBenchmark(quantum_crypto))
    runner.add_benchmark(AssetBenchmark(asset_manager))
    
    # Run benchmarks
    results = await runner.run_all()
    
    # Print results
    for result in results:
        print(f"\nBenchmark: {result['name']}")
        print(f"Total time: {result['total_time']:.2f}s")
        print(f"Mean: {result['mean']*1000:.2f}ms")
        print(f"Median: {result['median']*1000:.2f}ms")
        print(f"StdDev: {result['stddev']*1000:.2f}ms")
        print(f"Min: {result['min']*1000:.2f}ms")
        print(f"Max: {result['max']*1000:.2f}ms")

if __name__ == "__main__":
    # Configure logging
    logging.basicConfig(level=logging.INFO)
    
    # Run benchmarks
    asyncio.run(run_benchmarks()) 