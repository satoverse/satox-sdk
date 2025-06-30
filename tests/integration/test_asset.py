import pytest
from typing import AsyncGenerator
from satox.asset.asset_manager import AssetManager
from satox.tests.utils.mock_wallet import MockWallet

pytestmark = pytest.mark.asyncio

class TestAssetIntegration:
    """Integration tests for asset functionality."""

    async def test_asset_creation(
        self,
        asset_manager: AsyncGenerator[AssetManager, None],
        mock_wallet: MockWallet
    ):
        """Test asset creation and basic operations."""
        creator = mock_wallet.get_account("0x123")
        
        # Create new asset
        asset_data = {
            "name": "Test Asset",
            "symbol": "TEST",
            "decimals": 18,
            "total_supply": 1000000
        }
        
        asset = await asset_manager.create_asset(creator, asset_data)
        assert asset is not None, "Asset should be created"
        assert asset["name"] == asset_data["name"], "Asset should have correct name"
        assert asset["symbol"] == asset_data["symbol"], "Asset should have correct symbol"
        assert asset["total_supply"] == asset_data["total_supply"], "Asset should have correct supply"

    async def test_asset_transfer(
        self,
        asset_manager: AsyncGenerator[AssetManager, None],
        mock_wallet: MockWallet
    ):
        """Test asset transfer operations."""
        creator = mock_wallet.get_account("0x123")
        recipient = mock_wallet.get_account("0x456")
        
        # Create asset
        asset = await asset_manager.create_asset(creator, {
            "name": "Test Asset",
            "symbol": "TEST",
            "decimals": 18,
            "total_supply": 1000000
        })
        
        # Transfer asset
        amount = 1000
        transfer_tx = await asset_manager.transfer_asset(
            asset["address"],
            creator,
            recipient,
            amount
        )
        
        assert transfer_tx is not None, "Transfer should be successful"
        
        # Check balances
        creator_balance = await asset_manager.get_balance(asset["address"], creator)
        recipient_balance = await asset_manager.get_balance(asset["address"], recipient)
        
        assert creator_balance == 999000, "Creator balance should be reduced"
        assert recipient_balance == 1000, "Recipient balance should be increased"

    async def test_asset_metadata(
        self,
        asset_manager: AsyncGenerator[AssetManager, None],
        mock_wallet: MockWallet
    ):
        """Test asset metadata operations."""
        creator = mock_wallet.get_account("0x123")
        
        # Create asset with metadata
        metadata = {
            "description": "Test asset description",
            "image": "https://example.com/image.png",
            "properties": {
                "category": "test",
                "version": "1.0"
            }
        }
        
        asset = await asset_manager.create_asset(creator, {
            "name": "Test Asset",
            "symbol": "TEST",
            "decimals": 18,
            "total_supply": 1000000,
            "metadata": metadata
        })
        
        # Get metadata
        retrieved_metadata = await asset_manager.get_metadata(asset["address"])
        assert retrieved_metadata is not None, "Metadata should be retrieved"
        assert retrieved_metadata["description"] == metadata["description"]
        assert retrieved_metadata["image"] == metadata["image"]
        assert retrieved_metadata["properties"] == metadata["properties"]

    async def test_asset_permissions(
        self,
        asset_manager: AsyncGenerator[AssetManager, None],
        mock_wallet: MockWallet
    ):
        """Test asset permission management."""
        creator = mock_wallet.get_account("0x123")
        operator = mock_wallet.get_account("0x456")
        
        # Create asset
        asset = await asset_manager.create_asset(creator, {
            "name": "Test Asset",
            "symbol": "TEST",
            "decimals": 18,
            "total_supply": 1000000
        })
        
        # Grant operator permission
        await asset_manager.grant_operator_permission(asset["address"], creator, operator)
        
        # Verify permission
        has_permission = await asset_manager.check_operator_permission(
            asset["address"],
            operator
        )
        assert has_permission, "Operator should have permission"
        
        # Revoke permission
        await asset_manager.revoke_operator_permission(asset["address"], creator, operator)
        
        # Verify permission revoked
        has_permission = await asset_manager.check_operator_permission(
            asset["address"],
            operator
        )
        assert not has_permission, "Operator should not have permission"

    async def test_asset_error_handling(
        self,
        asset_manager: AsyncGenerator[AssetManager, None],
        mock_wallet: MockWallet
    ):
        """Test asset error handling."""
        creator = mock_wallet.get_account("0x123")
        recipient = mock_wallet.get_account("0x456")
        
        # Create asset
        asset = await asset_manager.create_asset(creator, {
            "name": "Test Asset",
            "symbol": "TEST",
            "decimals": 18,
            "total_supply": 1000000
        })
        
        # Test invalid transfer amount
        with pytest.raises(ValueError):
            await asset_manager.transfer_asset(
                asset["address"],
                creator,
                recipient,
                -1000
            )
        
        # Test insufficient balance
        with pytest.raises(Exception) as exc_info:
            await asset_manager.transfer_asset(
                asset["address"],
                recipient,  # Account with 0 balance
                creator,
                1000
            )
        assert "insufficient balance" in str(exc_info.value).lower()
        
        # Test invalid asset address
        with pytest.raises(ValueError):
            await asset_manager.get_balance("invalid_address", creator) 