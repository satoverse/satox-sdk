import pytest
import asyncio
from typing import AsyncGenerator
from satox.core.satox_manager import SatoxManager, SatoxConfig
from satox.blockchain.blockchain_manager import BlockchainManager, NetworkConfig
from satox.transaction.transaction_manager import TransactionManager, TransactionConfig
from satox.tests.utils.mock_wallet import MockWallet

# Test network configuration
TEST_NETWORK_ID = "testnet"
TEST_RPC_PORT = 18767
TEST_P2P_PORT = 8767

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
        network_id=TEST_NETWORK_ID,
        api_endpoint=f"http://localhost:{TEST_RPC_PORT}",
        debug_mode=True,
        max_retries=3,
        timeout=30,
        custom_config={
            "test_mode": True,
            "mock_network": True
        }
    )

@pytest.fixture(scope="session")
async def network_config() -> NetworkConfig:
    """Create a test configuration for BlockchainManager."""
    return NetworkConfig(
        network_id=TEST_NETWORK_ID,
        rpc_url=f"http://localhost:{TEST_RPC_PORT}",
        chain_id=9007,
        gas_limit=21000,
        gas_price=20000000000,
        timeout=30,
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
        timeout=30,
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
    await wallet.create_account("0x123", 1000)
    await wallet.create_account("0x456", 0)
    return wallet 