from typing import Dict, List, Optional, Any, Union
from datetime import datetime
import json
import logging
import hashlib
import os
from cryptography.fernet import Fernet
from ..core.satox_error import SatoxError

class WalletManager:
    """Manages blockchain wallets."""
    
    def __init__(self, wallet_dir: str = "wallets"):
        """Initialize the wallet manager.
        
        Args:
            wallet_dir: Directory to store wallets
        """
        self.wallet_dir = wallet_dir
        self.wallets = {}
        self.logger = logging.getLogger(__name__)
        
        # Create wallet directory if it doesn't exist
        if not os.path.exists(wallet_dir):
            os.makedirs(wallet_dir)
            
    def create_wallet(self, name: str, password: str) -> Dict[str, Any]:
        """Create a new wallet.
        
        Args:
            name: Wallet name
            password: Wallet password
            
        Returns:
            Wallet data
            
        Raises:
            SatoxError: If wallet creation fails
        """
        try:
            # Generate wallet key
            key = Fernet.generate_key()
            f = Fernet(key)
            
            # Create wallet data
            wallet = {
                'name': name,
                'created_at': datetime.utcnow().isoformat(),
                'address': self._generate_address(name),
                'balance': 0.0,
                'transactions': []
            }
            
            # Encrypt wallet data
            encrypted_data = f.encrypt(json.dumps(wallet).encode())
            
            # Save wallet
            wallet_path = os.path.join(self.wallet_dir, f"{name}.wallet")
            with open(wallet_path, 'wb') as f:
                f.write(encrypted_data)
                
            # Store wallet in memory
            self.wallets[name] = {
                'data': wallet,
                'key': key
            }
            
            self.logger.info(f"Created wallet {name}")
            return wallet
            
        except Exception as e:
            raise SatoxError(f"Failed to create wallet: {str(e)}")
            
    def load_wallet(self, name: str, password: str) -> Dict[str, Any]:
        """Load a wallet.
        
        Args:
            name: Wallet name
            password: Wallet password
            
        Returns:
            Wallet data
            
        Raises:
            SatoxError: If wallet not found or loading fails
        """
        try:
            wallet_path = os.path.join(self.wallet_dir, f"{name}.wallet")
            if not os.path.exists(wallet_path):
                raise SatoxError(f"Wallet {name} not found")
                
            # Load encrypted wallet data
            with open(wallet_path, 'rb') as f:
                encrypted_data = f.read()
                
            # Decrypt wallet data
            key = self._derive_key(password)
            f = Fernet(key)
            wallet_data = json.loads(f.decrypt(encrypted_data))
            
            # Store wallet in memory
            self.wallets[name] = {
                'data': wallet_data,
                'key': key
            }
            
            self.logger.info(f"Loaded wallet {name}")
            return wallet_data
            
        except Exception as e:
            raise SatoxError(f"Failed to load wallet: {str(e)}")
            
    def save_wallet(self, name: str) -> None:
        """Save a wallet.
        
        Args:
            name: Wallet name
            
        Raises:
            SatoxError: If wallet not found or saving fails
        """
        if name not in self.wallets:
            raise SatoxError(f"Wallet {name} not found")
            
        try:
            wallet = self.wallets[name]
            
            # Encrypt wallet data
            f = Fernet(wallet['key'])
            encrypted_data = f.encrypt(json.dumps(wallet['data']).encode())
            
            # Save wallet
            wallet_path = os.path.join(self.wallet_dir, f"{name}.wallet")
            with open(wallet_path, 'wb') as f:
                f.write(encrypted_data)
                
            self.logger.info(f"Saved wallet {name}")
            
        except Exception as e:
            raise SatoxError(f"Failed to save wallet: {str(e)}")
            
    def delete_wallet(self, name: str) -> None:
        """Delete a wallet.
        
        Args:
            name: Wallet name
            
        Raises:
            SatoxError: If wallet not found or deletion fails
        """
        try:
            wallet_path = os.path.join(self.wallet_dir, f"{name}.wallet")
            if not os.path.exists(wallet_path):
                raise SatoxError(f"Wallet {name} not found")
                
            # Delete wallet file
            os.remove(wallet_path)
            
            # Remove from memory
            if name in self.wallets:
                del self.wallets[name]
                
            self.logger.info(f"Deleted wallet {name}")
            
        except Exception as e:
            raise SatoxError(f"Failed to delete wallet: {str(e)}")
            
    def get_wallet(self, name: str) -> Dict[str, Any]:
        """Get wallet data.
        
        Args:
            name: Wallet name
            
        Returns:
            Wallet data
            
        Raises:
            SatoxError: If wallet not found
        """
        if name not in self.wallets:
            raise SatoxError(f"Wallet {name} not found")
            
        return self.wallets[name]['data']
        
    def list_wallets(self) -> List[str]:
        """List all wallets.
        
        Returns:
            List of wallet names
        """
        return list(self.wallets.keys())
        
    def update_balance(self, name: str, amount: float) -> None:
        """Update wallet balance.
        
        Args:
            name: Wallet name
            amount: Amount to add/subtract
            
        Raises:
            SatoxError: If wallet not found or update fails
        """
        if name not in self.wallets:
            raise SatoxError(f"Wallet {name} not found")
            
        try:
            wallet = self.wallets[name]['data']
            wallet['balance'] += amount
            wallet['updated_at'] = datetime.utcnow().isoformat()
            
            self.logger.info(f"Updated balance for wallet {name}")
            
        except Exception as e:
            raise SatoxError(f"Failed to update balance: {str(e)}")
            
    def add_transaction(self, name: str, transaction: Dict[str, Any]) -> None:
        """Add transaction to wallet.
        
        Args:
            name: Wallet name
            transaction: Transaction data
            
        Raises:
            SatoxError: If wallet not found or update fails
        """
        if name not in self.wallets:
            raise SatoxError(f"Wallet {name} not found")
            
        try:
            wallet = self.wallets[name]['data']
            wallet['transactions'].append(transaction)
            wallet['updated_at'] = datetime.utcnow().isoformat()
            
            self.logger.info(f"Added transaction to wallet {name}")
            
        except Exception as e:
            raise SatoxError(f"Failed to add transaction: {str(e)}")
            
    def _generate_address(self, name: str) -> str:
        """Generate a wallet address.
        
        Args:
            name: Wallet name
            
        Returns:
            Wallet address
        """
        data = f"{name}{datetime.utcnow().isoformat()}".encode()
        return hashlib.sha256(data).hexdigest()
        
    def _derive_key(self, password: str) -> bytes:
        """Derive encryption key from password.
        
        Args:
            password: Wallet password
            
        Returns:
            Encryption key
        """
        return hashlib.sha256(password.encode()).digest() 