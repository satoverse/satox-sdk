"""
Wallet module for managing Satoxcoin wallets.
"""

from typing import Dict, Optional, List, Any, Tuple
import json
import os
from pathlib import Path
import hashlib
import base58
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization
from datetime import datetime

from satox.security.quantum_crypto import QuantumCrypto
from satox.core.exceptions import SatoxError

class Wallet:
    """A class for managing Satoxcoin wallets."""
    
    def __init__(self, name: str, password: str, security_manager: Optional[Any] = None):
        """
        Initialize a new wallet.
        
        Args:
            name: Wallet name
            password: Wallet password
            security_manager: Optional security manager instance
        """
        self.name = name
        self.password = password
        self.security_manager = security_manager or QuantumCrypto()
        self._address: Optional[str] = None
        self._private_key: Optional[bytes] = None
        self._public_key: Optional[bytes] = None
        self._balance: float = 0.0
        self._transactions: List[Dict[str, Any]] = []
        self._created_at = datetime.now()
        self._last_accessed = datetime.now()
        
        # Generate or load keys
        self._initialize_keys()
    
    @classmethod
    def from_private_key(cls, private_key: str, name: str, password: str) -> 'Wallet':
        """
        Create a wallet from an existing private key.
        
        Args:
            private_key: Private key in hex format
            name: Wallet name
            password: Wallet password
            
        Returns:
            Wallet: New wallet instance
        """
        wallet = cls(name, password)
        wallet._private_key = bytes.fromhex(private_key)
        wallet._generate_public_key()
        wallet._generate_address()
        return wallet
    
    def _initialize_keys(self) -> None:
        """Initialize wallet keys."""
        # Generate quantum-resistant key pair
        self._private_key, self._public_key = self.security_manager.generate_keypair()
        self._generate_address()
    
    def _generate_public_key(self) -> None:
        """Generate public key from private key."""
        if not self._private_key:
            raise SatoxError("Private key not initialized")
        
        # Generate public key using quantum-resistant algorithm
        self._public_key = self.security_manager.get_public_key(self._private_key)
    
    def _generate_address(self) -> None:
        """Generate wallet address from public key."""
        if not self._public_key:
            raise SatoxError("Public key not initialized")
        
        # Generate address using quantum-resistant hash
        address_hash = self.security_manager.hash_public_key(self._public_key)
        self._address = base58.b58encode(address_hash).decode()
    
    def get_address(self) -> str:
        """Get wallet address."""
        if not self._address:
            raise SatoxError("Address not generated")
        return self._address
    
    def get_balance(self) -> float:
        """Get wallet balance."""
        return self._balance
    
    def get_transactions(self) -> List[Dict[str, Any]]:
        """Get wallet transaction history."""
        return self._transactions
    
    def sign_transaction(self, transaction: Dict[str, Any]) -> Dict[str, Any]:
        """
        Sign a transaction.
        
        Args:
            transaction: Transaction data
            
        Returns:
            Dict: Signed transaction
        """
        if not self._private_key:
            raise SatoxError("Private key not initialized")
        
        # Create transaction hash
        tx_hash = self.security_manager.hash_transaction(transaction)
        
        # Sign transaction using quantum-resistant signature
        signature = self.security_manager.sign_transaction(tx_hash, self._private_key)
        
        # Add signature to transaction
        signed_tx = transaction.copy()
        signed_tx['signature'] = signature.hex()
        signed_tx['public_key'] = self._public_key.hex()
        
        return signed_tx
    
    def verify_transaction(self, transaction: Dict[str, Any]) -> bool:
        """
        Verify a transaction signature.
        
        Args:
            transaction: Transaction data with signature
            
        Returns:
            bool: True if signature is valid
        """
        if 'signature' not in transaction or 'public_key' not in transaction:
            return False
        
        # Create transaction hash
        tx_hash = self.security_manager.hash_transaction(transaction)
        
        # Verify signature
        return self.security_manager.verify_signature(
            tx_hash,
            bytes.fromhex(transaction['signature']),
            bytes.fromhex(transaction['public_key'])
        )
    
    def encrypt(self) -> Dict[str, bytes]:
        """
        Encrypt wallet data.
        
        Returns:
            Dict: Encrypted wallet data
        """
        wallet_data = {
            'name': self.name,
            'address': self._address,
            'private_key': self._private_key.hex() if self._private_key else None,
            'public_key': self._public_key.hex() if self._public_key else None,
            'balance': self._balance,
            'transactions': self._transactions,
            'created_at': self._created_at.isoformat(),
            'last_accessed': self._last_accessed.isoformat()
        }
        
        return self.security_manager.encrypt_wallet(
            json.dumps(wallet_data).encode(),
            self.password
        )
    
    @classmethod
    def decrypt(cls, encrypted_data: Dict[str, bytes], password: str) -> 'Wallet':
        """
        Decrypt wallet data.
        
        Args:
            encrypted_data: Encrypted wallet data
            password: Wallet password
            
        Returns:
            Wallet: Decrypted wallet instance
        """
        # Decrypt wallet data
        decrypted_data = json.loads(
            QuantumCrypto().decrypt_wallet(encrypted_data, password).decode()
        )
        
        # Create wallet instance
        wallet = cls(decrypted_data['name'], password)
        wallet._address = decrypted_data['address']
        wallet._private_key = bytes.fromhex(decrypted_data['private_key']) if decrypted_data['private_key'] else None
        wallet._public_key = bytes.fromhex(decrypted_data['public_key']) if decrypted_data['public_key'] else None
        wallet._balance = decrypted_data['balance']
        wallet._transactions = decrypted_data['transactions']
        wallet._created_at = datetime.fromisoformat(decrypted_data['created_at'])
        wallet._last_accessed = datetime.fromisoformat(decrypted_data['last_accessed'])
        
        return wallet
    
    def backup(self, backup_path: str) -> None:
        """
        Backup wallet to file.
        
        Args:
            backup_path: Path to backup file
        """
        encrypted_data = self.encrypt()
        
        with open(backup_path, 'wb') as f:
            json.dump({
                'encrypted_data': {
                    k: base58.b58encode(v).decode() for k, v in encrypted_data.items()
                }
            }, f)
    
    @classmethod
    def restore(cls, backup_path: str, password: str) -> 'Wallet':
        """
        Restore wallet from backup.
        
        Args:
            backup_path: Path to backup file
            password: Wallet password
            
        Returns:
            Wallet: Restored wallet instance
        """
        with open(backup_path, 'r') as f:
            backup_data = json.load(f)
        
        encrypted_data = {
            k: base58.b58decode(v) for k, v in backup_data['encrypted_data'].items()
        }
        
        return cls.decrypt(encrypted_data, password)
    
    def update_balance(self, balance: float) -> None:
        """
        Update wallet balance.
        
        Args:
            balance: New balance
        """
        self._balance = balance
        self._last_accessed = datetime.now()
    
    def add_transaction(self, transaction: Dict[str, Any]) -> None:
        """
        Add transaction to history.
        
        Args:
            transaction: Transaction data
        """
        self._transactions.append(transaction)
        self._last_accessed = datetime.now() 