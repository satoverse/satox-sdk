import pytest
from typing import AsyncGenerator
from satox.blockchain.blockchain_manager import BlockchainManager
from satox.tests.utils.mock_wallet import MockWallet

pytestmark = pytest.mark.asyncio

class TestBlockchainIntegration:
    """Integration tests for blockchain functionality."""

    async def test_blockchain_connection(
        self,
        blockchain_manager: AsyncGenerator[BlockchainManager, None],
        mock_wallet: MockWallet
    ):
        """Test blockchain connection and basic operations."""
        # Test network connection
        is_connected = await blockchain_manager.is_connected()
        assert is_connected, "Blockchain manager should be connected to the network"

        # Test account balance
        balance = await blockchain_manager.get_balance(mock_wallet.get_account("0x123"))
        assert balance == 1000, "Account balance should match the mock wallet setup"

        # Test block information
        latest_block = await blockchain_manager.get_latest_block()
        assert latest_block is not None, "Should be able to get latest block"
        assert "number" in latest_block, "Block should have a number"
        assert "hash" in latest_block, "Block should have a hash"

    async def test_transaction_flow(
        self,
        blockchain_manager: AsyncGenerator[BlockchainManager, None],
        mock_wallet: MockWallet
    ):
        """Test transaction creation and processing."""
        from_account = mock_wallet.get_account("0x123")
        to_account = mock_wallet.get_account("0x456")
        amount = 100

        # Create transaction
        tx_hash = await blockchain_manager.create_transaction(
            from_address=from_account,
            to_address=to_account,
            value=amount
        )
        assert tx_hash is not None, "Transaction should be created successfully"

        # Wait for transaction confirmation
        receipt = await blockchain_manager.wait_for_transaction(tx_hash)
        assert receipt is not None, "Transaction should be confirmed"
        assert receipt["status"] == 1, "Transaction should be successful"

        # Verify balances after transaction
        from_balance = await blockchain_manager.get_balance(from_account)
        to_balance = await blockchain_manager.get_balance(to_account)
        
        assert from_balance == 900, "Sender balance should be reduced by amount plus gas"
        assert to_balance == 100, "Receiver balance should be increased by amount"

    async def test_error_handling(
        self,
        blockchain_manager: AsyncGenerator[BlockchainManager, None],
        mock_wallet: MockWallet
    ):
        """Test error handling in blockchain operations."""
        # Test invalid address
        with pytest.raises(ValueError):
            await blockchain_manager.get_balance("invalid_address")

        # Test insufficient funds
        from_account = mock_wallet.get_account("0x456")  # Account with 0 balance
        to_account = mock_wallet.get_account("0x123")
        
        with pytest.raises(Exception) as exc_info:
            await blockchain_manager.create_transaction(
                from_address=from_account,
                to_address=to_account,
                value=100
            )
        assert "insufficient funds" in str(exc_info.value).lower() 