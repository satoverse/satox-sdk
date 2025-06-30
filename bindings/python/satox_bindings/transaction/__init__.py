"""
Satox SDK Transaction Module
"""

from typing import Dict, List, Optional, Any
from datetime import datetime
import json
import hashlib

class Transaction:
    """Represents a blockchain transaction"""
    
    def __init__(self,
                 sender: str,
                 recipient: str,
                 amount: float,
                 timestamp: datetime,
                 transaction_hash: Optional[str] = None,
                 signature: Optional[str] = None):
        self.sender = sender
        self.recipient = recipient
        self.amount = amount
        self.timestamp = timestamp
        self.transaction_hash = transaction_hash or self._calculate_hash()
        self.signature = signature
    
    def _calculate_hash(self) -> str:
        """Calculate transaction hash"""
        data = f"{self.sender}{self.recipient}{self.amount}{self.timestamp.isoformat()}"
        return hashlib.sha256(data.encode()).hexdigest()
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert transaction to dictionary"""
        return {
            'sender': self.sender,
            'recipient': self.recipient,
            'amount': self.amount,
            'timestamp': self.timestamp.isoformat(),
            'transaction_hash': self.transaction_hash,
            'signature': self.signature
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Transaction':
        """Create transaction from dictionary"""
        return cls(
            sender=data['sender'],
            recipient=data['recipient'],
            amount=data['amount'],
            timestamp=datetime.fromisoformat(data['timestamp']),
            transaction_hash=data.get('transaction_hash'),
            signature=data.get('signature')
        )

class TransactionManager:
    """Manages transaction operations"""
    
    def __init__(self):
        self._transactions: List[Transaction] = []
        self._last_error = ""
    
    def create_transaction(self,
                          sender: str,
                          recipient: str,
                          amount: float) -> Optional[Transaction]:
        """
        Create a new transaction
        
        Args:
            sender: Sender address
            recipient: Recipient address
            amount: Transaction amount
            
        Returns:
            Optional[Transaction]: Created transaction if successful, None otherwise
        """
        try:
            # Validate transaction
            if not self._validate_transaction(sender, recipient, amount):
                return None
                
            transaction = Transaction(
                sender=sender,
                recipient=recipient,
                amount=amount,
                timestamp=datetime.utcnow()
            )
            
            self._transactions.append(transaction)
            return transaction
            
        except Exception as e:
            self._last_error = f"Failed to create transaction: {str(e)}"
            return None
    
    def get_transaction(self, transaction_hash: str) -> Optional[Transaction]:
        """
        Get transaction by hash
        
        Args:
            transaction_hash: Transaction hash
            
        Returns:
            Optional[Transaction]: Transaction if found, None otherwise
        """
        for transaction in self._transactions:
            if transaction.transaction_hash == transaction_hash:
                return transaction
        return None
    
    def get_pending_transactions(self) -> List[Transaction]:
        """
        Get all pending transactions
        
        Returns:
            List[Transaction]: List of pending transactions
        """
        return [t for t in self._transactions if not t.signature]
    
    def _validate_transaction(self,
                            sender: str,
                            recipient: str,
                            amount: float) -> bool:
        """
        Validate transaction parameters
        
        Args:
            sender: Sender address
            recipient: Recipient address
            amount: Transaction amount
            
        Returns:
            bool: True if parameters are valid, False otherwise
        """
        if not sender or not recipient:
            self._last_error = "Invalid sender or recipient address"
            return False
            
        if amount <= 0:
            self._last_error = "Amount must be greater than 0"
            return False
            
        return True
    
    def get_last_error(self) -> str:
        """
        Get the last error message
        
        Returns:
            str: Last error message
        """
        return self._last_error
    
    def clear_last_error(self) -> None:
        """Clear the last error message"""
        self._last_error = ""

# Export the manager instance
transaction_manager = TransactionManager() 