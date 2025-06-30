import pytest
import asyncio
import time
from typing import AsyncGenerator, Dict, Any
from satox.core.satox_manager import SatoxManager, SatoxConfig
from satox.blockchain.blockchain_manager import BlockchainManager, NetworkConfig
from satox.transaction.transaction_manager import TransactionManager, TransactionConfig
from satox.tests.utils.mock_wallet import MockWallet

# Performance test configuration
PERF_TEST_ITERATIONS = 100
PERF_TEST_TIMEOUT = 30  # seconds
PERF_TEST_CONCURRENT_REQUESTS = 10

class PerformanceMetrics:
    """Helper class to collect and analyze performance metrics."""
    
    def __init__(self):
        self.start_time = 0
        self.end_time = 0
        self.operations = []
        self.errors = []
    
    def start(self):
        """Start timing an operation."""
        self.start_time = time.time()
    
    def end(self, operation_name: str):
        """End timing an operation and record metrics."""
        self.end_time = time.time()
        duration = self.end_time - self.start_time
        self.operations.append({
            "name": operation_name,
            "duration": duration,
            "timestamp": self.end_time
        })
    
    def record_error(self, operation_name: str, error: Exception):
        """Record an error during an operation."""
        self.errors.append({
            "name": operation_name,
            "error": str(error),
            "timestamp": time.time()
        })
    
    def get_summary(self) -> Dict[str, Any]:
        """Get a summary of performance metrics."""
        if not self.operations:
            return {"error": "No operations recorded"}
        
        durations = [op["duration"] for op in self.operations]
        return {
            "total_operations": len(self.operations),
            "total_errors": len(self.errors),
            "min_duration": min(durations),
            "max_duration": max(durations),
            "avg_duration": sum(durations) / len(durations),
            "total_duration": sum(durations)
        }

@pytest.fixture(scope="session")
def event_loop():
    """Create an instance of the default event loop for the test session."""
    loop = asyncio.get_event_loop_policy().new_event_loop()
    yield loop
    loop.close()

@pytest.fixture(scope="session")
async def satox_config() -> SatoxConfig:
    """Create a test configuration for SatoxManager."""
    return SatoxConfig(
        network_id="testnet",
        api_endpoint="http://localhost:18767",
        debug_mode=True,
        max_retries=3,
        timeout=PERF_TEST_TIMEOUT,
        custom_config={
            "test_mode": True,
            "mock_network": True,
            "performance_testing": True
        }
    )

@pytest.fixture(scope="session")
async def network_config() -> NetworkConfig:
    """Create a test configuration for BlockchainManager."""
    return NetworkConfig(
        network_id="testnet",
        rpc_url="http://localhost:18767",
        chain_id=9007,
        gas_limit=21000,
        gas_price=20000000000,
        timeout=PERF_TEST_TIMEOUT,
        max_retries=3
    )

@pytest.fixture(scope="session")
async def transaction_config() -> TransactionConfig:
    """Create a test configuration for TransactionManager."""
    return TransactionConfig(
        gas_limit=21000,
        gas_price=20000000000,
        nonce=0,
        value=0,
        data="0x",
        timeout=PERF_TEST_TIMEOUT,
        max_retries=3
    )

@pytest.fixture(scope="session")
async def satox_manager(satox_config: SatoxConfig) -> AsyncGenerator[SatoxManager, None]:
    """Create and initialize a SatoxManager instance for testing."""
    manager = SatoxManager()
    await manager.initialize(satox_config)
    yield manager
    await manager.cleanup()

@pytest.fixture(scope="session")
async def blockchain_manager(network_config: NetworkConfig) -> AsyncGenerator[BlockchainManager, None]:
    """Create and initialize a BlockchainManager instance for testing."""
    manager = BlockchainManager()
    await manager.initialize(network_config)
    yield manager
    await manager.cleanup()

@pytest.fixture(scope="session")
async def transaction_manager(transaction_config: TransactionConfig) -> AsyncGenerator[TransactionManager, None]:
    """Create and initialize a TransactionManager instance for testing."""
    manager = TransactionManager()
    await manager.initialize(transaction_config)
    yield manager
    await manager.cleanup()

@pytest.fixture(scope="session")
async def mock_wallet() -> MockWallet:
    """Create a mock wallet with test accounts."""
    wallet = MockWallet()
    await wallet.create_account("0x123", 1000000)  # Large balance for performance testing
    await wallet.create_account("0x456", 0)
    return wallet

@pytest.fixture
def metrics() -> PerformanceMetrics:
    """Create a new PerformanceMetrics instance for each test."""
    return PerformanceMetrics() 