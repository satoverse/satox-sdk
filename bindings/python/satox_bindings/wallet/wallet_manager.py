from typing import Dict, Any, List, Optional
from ..core.error_handling import SatoxError
from ..core.satox_manager import SatoxManager

class WalletManager:
    def __init__(self, satox_manager: SatoxManager):
        """
        Initialize the WalletManager with a reference to the SatoxManager.
        
        Args:
            satox_manager: Instance of SatoxManager
        """
        self._satox_manager = satox_manager
        self._wallets: Dict[str, Dict[str, Any]] = {}

    def create_wallet(self, wallet_id: str, wallet_data: Dict[str, Any]) -> bool:
        """
        Create a new wallet.
        
        Args:
            wallet_id: Unique identifier for the wallet
            wallet_data: Dictionary containing wallet properties
            
        Returns:
            bool: True if wallet was created successfully
            
        Raises:
            SatoxError: If wallet creation fails
        """
        try:
            if wallet_id in self._wallets:
                raise SatoxError(f"Wallet with ID {wallet_id} already exists")
            
            self._wallets[wallet_id] = wallet_data
            return True
        except Exception as e:
            raise SatoxError(f"Failed to create wallet: {str(e)}")

    def get_wallet(self, wallet_id: str) -> Optional[Dict[str, Any]]:
        """
        Retrieve a wallet by its ID.
        
        Args:
            wallet_id: ID of the wallet to retrieve
            
        Returns:
            Optional[Dict[str, Any]]: Wallet data if found, None otherwise
        """
        return self._wallets.get(wallet_id)

    def update_wallet(self, wallet_id: str, wallet_data: Dict[str, Any]) -> bool:
        """
        Update an existing wallet's data.
        
        Args:
            wallet_id: ID of the wallet to update
            wallet_data: New wallet data
            
        Returns:
            bool: True if update was successful
            
        Raises:
            SatoxError: If wallet update fails
        """
        try:
            if wallet_id not in self._wallets:
                raise SatoxError(f"Wallet with ID {wallet_id} not found")
            
            self._wallets[wallet_id].update(wallet_data)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to update wallet: {str(e)}")

    def delete_wallet(self, wallet_id: str) -> bool:
        """
        Delete a wallet by its ID.
        
        Args:
            wallet_id: ID of the wallet to delete
            
        Returns:
            bool: True if deletion was successful
            
        Raises:
            SatoxError: If wallet deletion fails
        """
        try:
            if wallet_id not in self._wallets:
                raise SatoxError(f"Wallet with ID {wallet_id} not found")
            
            del self._wallets[wallet_id]
            return True
        except Exception as e:
            raise SatoxError(f"Failed to delete wallet: {str(e)}")

    def list_wallets(self) -> List[str]:
        """
        Get a list of all wallet IDs.
        
        Returns:
            List[str]: List of wallet IDs
        """
        return list(self._wallets.keys())

    def get_wallet_balance(self, wallet_id: str) -> Dict[str, float]:
        """
        Get the balance of a wallet.
        
        Args:
            wallet_id: ID of the wallet
            
        Returns:
            Dict[str, float]: Dictionary mapping asset types to balances
            
        Raises:
            SatoxError: If getting balance fails
        """
        try:
            wallet = self.get_wallet(wallet_id)
            if not wallet:
                raise SatoxError(f"Wallet with ID {wallet_id} not found")
            
            return wallet.get('balances', {})
        except Exception as e:
            raise SatoxError(f"Failed to get wallet balance: {str(e)}")

    def update_wallet_balance(self, wallet_id: str, asset_type: str, amount: float) -> bool:
        """
        Update the balance of a wallet for a specific asset type.
        
        Args:
            wallet_id: ID of the wallet
            asset_type: Type of asset
            amount: Amount to update by (positive for credit, negative for debit)
            
        Returns:
            bool: True if balance was updated successfully
            
        Raises:
            SatoxError: If balance update fails
        """
        try:
            wallet = self.get_wallet(wallet_id)
            if not wallet:
                raise SatoxError(f"Wallet with ID {wallet_id} not found")
            
            if 'balances' not in wallet:
                wallet['balances'] = {}
            
            current_balance = wallet['balances'].get(asset_type, 0.0)
            new_balance = current_balance + amount
            
            if new_balance < 0:
                raise SatoxError(f"Insufficient balance for {asset_type}")
            
            wallet['balances'][asset_type] = new_balance
            return True
        except Exception as e:
            raise SatoxError(f"Failed to update wallet balance: {str(e)}")

    def transfer_assets(self, from_wallet_id: str, to_wallet_id: str, asset_type: str, amount: float) -> bool:
        """
        Transfer assets between wallets.
        
        Args:
            from_wallet_id: ID of the source wallet
            to_wallet_id: ID of the destination wallet
            asset_type: Type of asset to transfer
            amount: Amount to transfer
            
        Returns:
            bool: True if transfer was successful
            
        Raises:
            SatoxError: If transfer fails
        """
        try:
            if amount <= 0:
                raise SatoxError("Transfer amount must be positive")
            
            # Debit from source wallet
            if not self.update_wallet_balance(from_wallet_id, asset_type, -amount):
                return False
            
            # Credit to destination wallet
            if not self.update_wallet_balance(to_wallet_id, asset_type, amount):
                # Rollback source wallet if destination update fails
                self.update_wallet_balance(from_wallet_id, asset_type, amount)
                raise SatoxError("Failed to credit destination wallet")
            
            return True
        except Exception as e:
            raise SatoxError(f"Failed to transfer assets: {str(e)}")

    def get_wallet_transactions(self, wallet_id: str) -> List[Dict[str, Any]]:
        """
        Get transaction history for a wallet.
        
        Args:
            wallet_id: ID of the wallet
            
        Returns:
            List[Dict[str, Any]]: List of transactions
            
        Raises:
            SatoxError: If getting transactions fails
        """
        try:
            wallet = self.get_wallet(wallet_id)
            if not wallet:
                raise SatoxError(f"Wallet with ID {wallet_id} not found")
            
            return wallet.get('transactions', [])
        except Exception as e:
            raise SatoxError(f"Failed to get wallet transactions: {str(e)}")

    def add_transaction(self, wallet_id: str, transaction: Dict[str, Any]) -> bool:
        """
        Add a transaction to a wallet's history.
        
        Args:
            wallet_id: ID of the wallet
            transaction: Transaction data
            
        Returns:
            bool: True if transaction was added successfully
            
        Raises:
            SatoxError: If adding transaction fails
        """
        try:
            wallet = self.get_wallet(wallet_id)
            if not wallet:
                raise SatoxError(f"Wallet with ID {wallet_id} not found")
            
            if 'transactions' not in wallet:
                wallet['transactions'] = []
            
            wallet['transactions'].append(transaction)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to add transaction: {str(e)}") 