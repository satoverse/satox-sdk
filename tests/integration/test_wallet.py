import pytest
from typing import AsyncGenerator
from satox.wallet.wallet_manager import WalletManager
from satox.tests.utils.mock_wallet import MockWallet

pytestmark = pytest.mark.asyncio

class TestWalletIntegration:
    """Integration tests for wallet functionality."""

    async def test_wallet_creation(
        self,
        wallet_manager: AsyncGenerator[WalletManager, None]
    ):
        """Test wallet creation and basic operations."""
        # Create new wallet
        wallet = await wallet_manager.create_wallet()
        assert wallet is not None, "Wallet should be created successfully"
        assert wallet.address is not None, "Wallet should have an address"
        assert wallet.private_key is not None, "Wallet should have a private key"

        # Test wallet backup
        backup = await wallet_manager.backup_wallet(wallet)
        assert backup is not None, "Wallet backup should be created"
        assert "address" in backup, "Backup should contain address"
        assert "private_key" in backup, "Backup should contain private key"

        # Test wallet restoration
        restored_wallet = await wallet_manager.restore_wallet(backup)
        assert restored_wallet.address == wallet.address, "Restored wallet should have same address"

    async def test_wallet_operations(
        self,
        wallet_manager: AsyncGenerator[WalletManager, None],
        mock_wallet: MockWallet
    ):
        """Test wallet operations and transactions."""
        # Test account creation
        account = await wallet_manager.create_account("test_account")
        assert account is not None, "Account should be created"
        assert account.name == "test_account", "Account should have correct name"

        # Test balance checking
        balance = await wallet_manager.get_balance(account.address)
        assert balance >= 0, "Balance should be non-negative"

        # Test transaction signing
        tx_data = {
            "to": mock_wallet.get_account("0x456"),
            "value": 100,
            "gas": 21000,
            "nonce": 0
        }
        signed_tx = await wallet_manager.sign_transaction(account, tx_data)
        assert signed_tx is not None, "Transaction should be signed"
        assert "signature" in signed_tx, "Signed transaction should have signature"

    async def test_wallet_security(
        self,
        wallet_manager: AsyncGenerator[WalletManager, None]
    ):
        """Test wallet security features."""
        # Test password protection
        wallet = await wallet_manager.create_wallet()
        password = "test_password"
        
        # Encrypt wallet
        encrypted_wallet = await wallet_manager.encrypt_wallet(wallet, password)
        assert encrypted_wallet.is_encrypted, "Wallet should be encrypted"
        
        # Test decryption with correct password
        decrypted_wallet = await wallet_manager.decrypt_wallet(encrypted_wallet, password)
        assert decrypted_wallet.address == wallet.address, "Decrypted wallet should match original"
        
        # Test decryption with wrong password
        with pytest.raises(Exception) as exc_info:
            await wallet_manager.decrypt_wallet(encrypted_wallet, "wrong_password")
        assert "invalid password" in str(exc_info.value).lower()

    async def test_wallet_import_export(
        self,
        wallet_manager: AsyncGenerator[WalletManager, None]
    ):
        """Test wallet import and export functionality."""
        # Create test wallet
        wallet = await wallet_manager.create_wallet()
        
        # Export wallet
        exported_data = await wallet_manager.export_wallet(wallet)
        assert exported_data is not None, "Wallet should be exported"
        assert "address" in exported_data, "Export should contain address"
        
        # Import wallet
        imported_wallet = await wallet_manager.import_wallet(exported_data)
        assert imported_wallet.address == wallet.address, "Imported wallet should match original"
        
        # Test invalid import data
        with pytest.raises(ValueError):
            await wallet_manager.import_wallet({"invalid": "data"}) 