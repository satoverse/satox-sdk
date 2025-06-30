"""Transaction Processor for Satox SDK.
Handles blockchain transaction processing operations.
"""

from typing import Optional, List, Dict, Union
import json
import hashlib
import time

class TransactionProcessor:
    """Processes blockchain transactions and manages transaction-related operations."""
    
    def __init__(self):
        """Initialize the transaction processor."""
        self._initialized = False
        self._last_error = ""
        self._pending_transactions = []
        self._processed_transactions = {}

    def initialize(self) -> bool:
        """Initialize the transaction processor."""
        try:
            self._initialized = True
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def shutdown(self) -> bool:
        """Shutdown the transaction processor."""
        try:
            self._initialized = False
            self._pending_transactions.clear()
            self._processed_transactions.clear()
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def create_transaction(self, sender: str, recipient: str, amount: Union[int, float], data: Optional[Dict] = None) -> Optional[Dict]:
        """Create a new transaction."""
        if not self._initialized:
            self._last_error = "Transaction processor not initialized"
            return None
        
        try:
            # Create transaction object
            transaction = {
                "sender": sender,
                "recipient": recipient,
                "amount": amount,
                "timestamp": int(time.time()),
                "data": data if data else {},
                "status": "pending"
            }
            
            # Calculate transaction hash
            tx_hash = self._calculate_hash(transaction)
            transaction["hash"] = tx_hash
            
            # Add to pending transactions
            self._pending_transactions.append(transaction)
            
            return transaction
        except Exception as e:
            self._last_error = str(e)
            return None

    def process_transaction(self, transaction: Dict) -> bool:
        """Process a single transaction."""
        if not self._initialized:
            self._last_error = "Transaction processor not initialized"
            return False
        
        try:
            # Validate transaction
            if not self._validate_transaction(transaction):
                return False
                
            # Process the transaction
            tx_hash = transaction["hash"]
            transaction["status"] = "processed"
            
            # Move to processed transactions
            self._processed_transactions[tx_hash] = transaction
            
            # If it was in pending, remove it
            self._pending_transactions = [tx for tx in self._pending_transactions 
                                        if tx["hash"] != tx_hash]
            
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def get_transaction(self, tx_hash: str) -> Optional[Dict]:
        """Get a transaction by its hash."""
        if not self._initialized:
            self._last_error = "Transaction processor not initialized"
            return None
        
        # Check processed transactions
        if tx_hash in self._processed_transactions:
            return self._processed_transactions[tx_hash]
        
        # Check pending transactions
        for tx in self._pending_transactions:
            if tx["hash"] == tx_hash:
                return tx
        
        self._last_error = f"Transaction with hash {tx_hash} not found"
        return None

    def get_pending_transactions(self) -> List[Dict]:
        """Get all pending transactions."""
        if not self._initialized:
            self._last_error = "Transaction processor not initialized"
            return []
        
        return self._pending_transactions

    def _validate_transaction(self, transaction: Dict) -> bool:
        """Validate a transaction."""
        required_fields = ["hash", "sender", "recipient", "amount", "timestamp"]
        for field in required_fields:
            if field not in transaction:
                self._last_error = f"Missing required field: {field}"
                return False
        
        # Validate hash
        calculated_hash = self._calculate_hash({
            k: v for k, v in transaction.items() if k != "hash"
        })
        
        if calculated_hash != transaction["hash"]:
            self._last_error = "Invalid transaction hash"
            return False
        
        return True

    def _calculate_hash(self, data: Dict) -> str:
        """Calculate hash for a transaction."""
        # Convert data to sorted string to ensure consistent hash
        data_str = str(sorted(data.items()))
        return hashlib.sha256(data_str.encode()).hexdigest()

    def get_last_error(self) -> str:
        """Get the last error message."""
        return self._last_error
        
    def clear_last_error(self) -> None:
        """Clear the last error message."""
        self._last_error = ""
