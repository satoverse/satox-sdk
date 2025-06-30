import os
os.environ["SATOX_API_KEY"] = "test_api_key"
os.environ["SATOX_SECRET_KEY"] = "test_secret_key"
os.environ["SATOX_NETWORK_ID"] = "mainnet"

import pytest
from satox.core.satox_manager import SatoxManager, SatoxConfig
from satox.blockchain.blockchain_manager import BlockchainManager, NetworkConfig
from satox.transaction.transaction_manager import TransactionManager, TransactionConfig
from satox.tests.utils.mock_wallet import MockWallet
from satox.security.security_manager import SecurityManager

MAINNET_RPC_PORT = 18767
MAINNET_P2P_PORT = 8767

PERF_TEST_ITERATIONS = 100
PERF_TEST_CONCURRENT_REQUESTS = 10

class PerformanceMetrics:
    def __init__(self, iterations, concurrent_requests):
        self.iterations = iterations
        self.concurrent_requests = concurrent_requests

@pytest.fixture
def satox_config():
    return SatoxConfig(
        network_id="mainnet",
        api_endpoint=f"https://localhost:{MAINNET_RPC_PORT}",
        api_key="test_api_key",
        debug_mode=True,
        max_retries=3,
        timeout=30,
        custom_config={}
    )

@pytest.fixture
def network_config():
    return NetworkConfig(
        network_id="mainnet",
        rpc_url=f"http://localhost:{MAINNET_RPC_PORT}",
        chain_id=9007,
        gas_limit=21000,
        gas_price=20000000000,
        timeout=30,
        max_retries=3
    )

@pytest.fixture
def transaction_config():
    return TransactionConfig(
        gas_limit=21000,
        gas_price=20000000000,
        nonce=0,
        value=0,
        data="0x",
        timeout=30,
        max_retries=3
    )

@pytest.fixture
def satox_manager(satox_config):
    manager = SatoxManager()
    manager.initialize(satox_config)
    return manager

@pytest.fixture
def blockchain_manager(network_config):
    manager = BlockchainManager()
    manager.initialize(network_config)
    return manager

@pytest.fixture
def transaction_manager(transaction_config):
    manager = TransactionManager()
    manager.initialize(transaction_config)
    return manager

@pytest.fixture
def mock_wallet():
    wallet = MockWallet()
    wallet.create_account("0x123", 1000)
    wallet.create_account("0x456", 0)
    return wallet

@pytest.fixture
def sdk(satox_config):
    manager = SatoxManager()
    manager.initialize(satox_config)
    return manager

@pytest.fixture
def security_manager():
    manager = SecurityManager()
    return manager 