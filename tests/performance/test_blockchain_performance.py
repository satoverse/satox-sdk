import pytest
import asyncio
from typing import AsyncGenerator, List
from satox.blockchain.blockchain_manager import BlockchainManager
from satox.tests.utils.mock_wallet import MockWallet
from tests.conftest import PerformanceMetrics, PERF_TEST_ITERATIONS, PERF_TEST_CONCURRENT_REQUESTS, sdk

pytestmark = pytest.mark.asyncio

class TestBlockchainPerformance:
    """Performance tests for blockchain operations."""

    async def test_block_retrieval_performance(
        self,
        blockchain_manager: AsyncGenerator[BlockchainManager, None],
        metrics: PerformanceMetrics
    ):
        """Test performance of block retrieval operations."""
        for i in range(PERF_TEST_ITERATIONS):
            try:
                metrics.start()
                block = await blockchain_manager.get_latest_block()
                metrics.end(f"get_latest_block_{i}")
                assert block is not None
            except Exception as e:
                metrics.record_error(f"get_latest_block_{i}", e)
        
        summary = metrics.get_summary()
        assert summary["total_operations"] == PERF_TEST_ITERATIONS
        assert summary["total_errors"] == 0
        assert summary["avg_duration"] < 1.0  # Should complete within 1 second

    async def test_concurrent_transaction_performance(
        self,
        blockchain_manager: AsyncGenerator[BlockchainManager, None],
        mock_wallet: MockWallet,
        metrics: PerformanceMetrics
    ):
        """Test performance of concurrent transaction operations."""
        from_account = mock_wallet.get_account("0x123")
        to_account = mock_wallet.get_account("0x456")
        
        async def create_transaction():
            try:
                metrics.start()
                tx = await blockchain_manager.create_transaction(
                    from_address=from_account,
                    to_address=to_account,
                    value=100
                )
                metrics.end("create_transaction")
                return tx
            except Exception as e:
                metrics.record_error("create_transaction", e)
                return None
        
        # Create concurrent transactions
        tasks = [create_transaction() for _ in range(PERF_TEST_CONCURRENT_REQUESTS)]
        results = await asyncio.gather(*tasks)
        
        summary = metrics.get_summary()
        assert summary["total_operations"] == PERF_TEST_CONCURRENT_REQUESTS
        assert summary["total_errors"] == 0
        assert summary["avg_duration"] < 2.0  # Should complete within 2 seconds

    async def test_balance_check_performance(
        self,
        blockchain_manager: AsyncGenerator[BlockchainManager, None],
        mock_wallet: MockWallet,
        metrics: PerformanceMetrics
    ):
        """Test performance of balance checking operations."""
        account = mock_wallet.get_account("0x123")
        
        async def check_balance():
            try:
                metrics.start()
                balance = await blockchain_manager.get_balance(account)
                metrics.end("get_balance")
                return balance
            except Exception as e:
                metrics.record_error("get_balance", e)
                return None
        
        # Perform concurrent balance checks
        tasks = [check_balance() for _ in range(PERF_TEST_ITERATIONS)]
        results = await asyncio.gather(*tasks)
        
        summary = metrics.get_summary()
        assert summary["total_operations"] == PERF_TEST_ITERATIONS
        assert summary["total_errors"] == 0
        assert summary["avg_duration"] < 0.5  # Should complete within 0.5 seconds

    async def test_block_range_performance(
        self,
        blockchain_manager: AsyncGenerator[BlockchainManager, None],
        metrics: PerformanceMetrics
    ):
        """Test performance of block range retrieval."""
        for i in range(10):  # Test with smaller range
            try:
                metrics.start()
                blocks = await blockchain_manager.get_blocks_range(i, i + 10)
                metrics.end(f"get_blocks_range_{i}")
                assert len(blocks) <= 10
            except Exception as e:
                metrics.record_error(f"get_blocks_range_{i}", e)
        
        summary = metrics.get_summary()
        assert summary["total_operations"] == 10
        assert summary["total_errors"] == 0
        assert summary["avg_duration"] < 2.0  # Should complete within 2 seconds

    async def test_transaction_history_performance(
        self,
        blockchain_manager: AsyncGenerator[BlockchainManager, None],
        mock_wallet: MockWallet,
        metrics: PerformanceMetrics
    ):
        """Test performance of transaction history retrieval."""
        account = mock_wallet.get_account("0x123")
        
        for i in range(PERF_TEST_ITERATIONS):
            try:
                metrics.start()
                history = await blockchain_manager.get_transaction_history(account)
                metrics.end(f"get_transaction_history_{i}")
                assert isinstance(history, list)
            except Exception as e:
                metrics.record_error(f"get_transaction_history_{i}", e)
        
        summary = metrics.get_summary()
        assert summary["total_operations"] == PERF_TEST_ITERATIONS
        assert summary["total_errors"] == 0
        assert summary["avg_duration"] < 1.0  # Should complete within 1 second 