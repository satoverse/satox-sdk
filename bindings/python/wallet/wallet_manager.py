"""
Wallet manager for handling multiple Satoxcoin wallets.
"""

from typing import Dict, List, Optional, Any
from pathlib import Path
import json
import os

from satox.wallet.wallet import Wallet

class WalletManager:
    """A class for managing multiple Satoxcoin wallets."""
    
    def __init__(self, wallet_dir: Optional[str] = None):
        """
        Initialize the wallet manager.
        
        Args:
            wallet_dir (str, optional): Directory to store wallet files. Defaults to ~/.satox/wallets.
        """
        self.wallet_dir = Path(wallet_dir or os.path.expanduser("~/.satox/wallets"))
        self.wallet_dir.mkdir(parents=True, exist_ok=True)
        self.wallets: Dict[str, Wallet] = {}
        self._load_wallets()
    
    def _load_wallets(self) -> None:
        """Load all wallets from the wallet directory."""
        for wallet_file in self.wallet_dir.glob("*.json"):
            try:
                with open(wallet_file, 'r') as f:
                    wallet_data = json.load(f)
                    wallet = Wallet.from_private_key(wallet_data['private_key'])
                    self.wallets[wallet.address] = wallet
            except Exception as e:
                print(f"Error loading wallet {wallet_file}: {e}")
    
    def create_wallet(self) -> Wallet:
        """
        Create a new wallet.
        
        Returns:
            Wallet: The newly created wallet.
        """
        wallet = Wallet()
        self._save_wallet(wallet)
        self.wallets[wallet.address] = wallet
        return wallet
    
    def import_wallet(self, private_key: str) -> Wallet:
        """
        Import a wallet from a private key.
        
        Args:
            private_key (str): The private key to import.
            
        Returns:
            Wallet: The imported wallet.
        """
        wallet = Wallet.from_private_key(private_key)
        self._save_wallet(wallet)
        self.wallets[wallet.address] = wallet
        return wallet
    
    def import_wallet_from_mnemonic(self, mnemonic: str) -> Wallet:
        """
        Import a wallet from a mnemonic phrase.
        
        Args:
            mnemonic (str): The mnemonic phrase.
            
        Returns:
            Wallet: The imported wallet.
        """
        wallet = Wallet.from_mnemonic(mnemonic)
        self._save_wallet(wallet)
        self.wallets[wallet.address] = wallet
        return wallet
    
    def _save_wallet(self, wallet: Wallet) -> None:
        """
        Save a wallet to disk.
        
        Args:
            wallet (Wallet): The wallet to save.
        """
        wallet_file = self.wallet_dir / f"{wallet.address}.json"
        wallet_data = {
            'address': wallet.address,
            'private_key': wallet.private_key,
            'public_key': wallet.public_key
        }
        with open(wallet_file, 'w') as f:
            json.dump(wallet_data, f, indent=2)
    
    def get_wallet(self, address: str) -> Optional[Wallet]:
        """
        Get a wallet by address.
        
        Args:
            address (str): The wallet address.
            
        Returns:
            Optional[Wallet]: The wallet if found, None otherwise.
        """
        return self.wallets.get(address)
    
    def list_wallets(self) -> List[Dict[str, Any]]:
        """
        List all wallets.
        
        Returns:
            List[Dict[str, Any]]: List of wallet data.
        """
        return [wallet.to_dict() for wallet in self.wallets.values()]
    
    def delete_wallet(self, address: str) -> bool:
        """
        Delete a wallet.
        
        Args:
            address (str): The wallet address.
            
        Returns:
            bool: True if the wallet was deleted, False otherwise.
        """
        if address in self.wallets:
            wallet_file = self.wallet_dir / f"{address}.json"
            try:
                wallet_file.unlink()
                del self.wallets[address]
                return True
            except Exception as e:
                print(f"Error deleting wallet {address}: {e}")
        return False 