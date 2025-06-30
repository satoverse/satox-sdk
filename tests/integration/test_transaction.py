import pytest
from typing import AsyncGenerator
from satox.transaction.transaction_manager import TransactionManager
from satox.tests.utils.mock_wallet import MockWallet

pytestmark = pytest.mark.asyncio

class TestTransactionIntegration:
    """Integration tests for transaction functionality."""

    async def test_transaction_creation(
        self,
        transaction_manager: AsyncGenerator[TransactionManager, None],
        mock_wallet: MockWallet
    ):
        """Test transaction creation and validation."""
        from_account = mock_wallet.get_account("0x123")
        to_account = mock_wallet.get_account("0x456")
        
        # Create transaction
        tx = await transaction_manager.create_transaction(
            from_address=from_account,
            to_address=to_account,
            value=100,
            data="0x"
        )
        assert tx is not None, "Transaction should be created"
        assert tx["from"] == from_account, "Transaction should have correct sender"
        assert tx["to"] == to_account, "Transaction should have correct recipient"
        assert tx["value"] == 100, "Transaction should have correct value"

        # Validate transaction
        is_valid = await transaction_manager.validate_transaction(tx)
        assert is_valid, "Transaction should be valid"

    async def test_transaction_signing(
        self,
        transaction_manager: AsyncGenerator[TransactionManager, None],
        mock_wallet: MockWallet
    ):
        """Test transaction signing and verification."""
        from_account = mock_wallet.get_account("0x123")
        to_account = mock_wallet.get_account("0x456")
        
        # Create and sign transaction
        tx = await transaction_manager.create_transaction(
            from_address=from_account,
            to_address=to_account,
            value=100
        )
        signed_tx = await transaction_manager.sign_transaction(tx, from_account)
        
        assert signed_tx is not None, "Transaction should be signed"
        assert "signature" in signed_tx, "Signed transaction should have signature"
        
        # Verify signature
        is_verified = await transaction_manager.verify_transaction(signed_tx)
        assert is_verified, "Transaction signature should be verified"

    async def test_transaction_broadcasting(
        self,
        transaction_manager: AsyncGenerator[TransactionManager, None],
        mock_wallet: MockWallet
    ):
        """Test transaction broadcasting and confirmation."""
        from_account = mock_wallet.get_account("0x123")
        to_account = mock_wallet.get_account("0x456")
        
        # Create, sign and broadcast transaction
        tx = await transaction_manager.create_transaction(
            from_address=from_account,
            to_address=to_account,
            value=100
        )
        signed_tx = await transaction_manager.sign_transaction(tx, from_account)
        tx_hash = await transaction_manager.broadcast_transaction(signed_tx)
        
        assert tx_hash is not None, "Transaction should be broadcasted"
        
        # Wait for confirmation
        receipt = await transaction_manager.wait_for_confirmation(tx_hash)
        assert receipt is not None, "Transaction should be confirmed"
        assert receipt["status"] == 1, "Transaction should be successful"

    async def test_transaction_error_handling(
        self,
        transaction_manager: AsyncGenerator[TransactionManager, None],
        mock_wallet: MockWallet
    ):
        """Test transaction error handling."""
        from_account = mock_wallet.get_account("0x123")
        to_account = mock_wallet.get_account("0x456")
        
        # Test invalid amount
        with pytest.raises(ValueError):
            await transaction_manager.create_transaction(
                from_address=from_account,
                to_address=to_account,
                value=-100
            )
        
        # Test invalid address
        with pytest.raises(ValueError):
            await transaction_manager.create_transaction(
                from_address="invalid_address",
                to_address=to_account,
                value=100
            )
        
        # Test insufficient funds
        with pytest.raises(Exception) as exc_info:
            await transaction_manager.create_transaction(
                from_address=mock_wallet.get_account("0x456"),  # Account with 0 balance
                to_address=from_account,
                value=1000
            )
        assert "insufficient funds" in str(exc_info.value).lower()

    async def test_transaction_history(
        self,
        transaction_manager: AsyncGenerator[TransactionManager, None],
        mock_wallet: MockWallet
    ):
        """Test transaction history and querying."""
        account = mock_wallet.get_account("0x123")
        
        # Get transaction history
        history = await transaction_manager.get_transaction_history(account)
        assert history is not None, "Transaction history should be retrieved"
        assert isinstance(history, list), "History should be a list"
        
        # Get specific transaction
        if history:
            tx_hash = history[0]["hash"]
            tx_details = await transaction_manager.get_transaction(tx_hash)
            assert tx_details is not None, "Transaction details should be retrieved"
            assert tx_details["hash"] == tx_hash, "Transaction hash should match" 