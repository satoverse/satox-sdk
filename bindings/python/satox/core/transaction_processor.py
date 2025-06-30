"""
Transaction Processor Python Bindings
"""

from typing import Dict, List, Optional, Any
import satox_cpp

class TransactionProcessor:
    """Python wrapper for the C++ TransactionProcessor class."""
    
    def __init__(self):
        """Initialize the Transaction Processor."""
        self._processor = satox_cpp.TransactionProcessor()
        self._initialized = False
    
    def initialize(self) -> bool:
        """Initialize the transaction processor.
        
        Returns:
            bool: True if initialization was successful, False otherwise.
        """
        if not self._initialized:
            self._initialized = self._processor.initialize()
        return self._initialized
    
    def shutdown(self) -> bool:
        """Shutdown the transaction processor.
        
        Returns:
            bool: True if shutdown was successful, False otherwise.
        """
        if self._initialized:
            self._initialized = not self._processor.shutdown()
        return not self._initialized
    
    def process_transaction(self, transaction_data: bytes) -> bool:
        """Process a new transaction.
        
        Args:
            transaction_data (bytes): The transaction data to process
            
        Returns:
            bool: True if processing was successful, False otherwise
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        if not transaction_data:
            raise ValueError("Invalid transaction data")
        
        return self._processor.process_transaction(transaction_data)
    
    def validate_transaction(self, transaction_data: bytes) -> bool:
        """Validate a transaction.
        
        Args:
            transaction_data (bytes): The transaction data to validate
            
        Returns:
            bool: True if the transaction is valid, False otherwise
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        if not transaction_data:
            raise ValueError("Invalid transaction data")
        
        return self._processor.validate_transaction(transaction_data)
    
    def get_transaction_info(self, transaction_hash: bytes) -> Optional[Dict[str, Any]]:
        """Get information about a transaction.
        
        Args:
            transaction_hash (bytes): The hash of the transaction
            
        Returns:
            Optional[Dict[str, Any]]: Transaction information, or None if not found
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        if not transaction_hash:
            raise ValueError("Invalid transaction hash")
        
        return self._processor.get_transaction_info(transaction_hash)
    
    def get_transaction_status(self, transaction_hash: bytes) -> str:
        """Get the status of a transaction.
        
        Args:
            transaction_hash (bytes): The hash of the transaction
            
        Returns:
            str: The transaction status
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        if not transaction_hash:
            raise ValueError("Invalid transaction hash")
        
        return self._processor.get_transaction_status(transaction_hash)
    
    def get_transaction_receipt(self, transaction_hash: bytes) -> Optional[Dict[str, Any]]:
        """Get the receipt of a transaction.
        
        Args:
            transaction_hash (bytes): The hash of the transaction
            
        Returns:
            Optional[Dict[str, Any]]: Transaction receipt, or None if not found
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        if not transaction_hash:
            raise ValueError("Invalid transaction hash")
        
        return self._processor.get_transaction_receipt(transaction_hash)
    
    def get_transaction_logs(self, transaction_hash: bytes) -> List[Dict[str, Any]]:
        """Get the logs of a transaction.
        
        Args:
            transaction_hash (bytes): The hash of the transaction
            
        Returns:
            List[Dict[str, Any]]: List of transaction logs
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        if not transaction_hash:
            raise ValueError("Invalid transaction hash")
        
        return self._processor.get_transaction_logs(transaction_hash)
    
    def get_pending_transactions(self) -> List[Dict[str, Any]]:
        """Get all pending transactions.
        
        Returns:
            List[Dict[str, Any]]: List of pending transaction information
            
        Raises:
            RuntimeError: If the processor is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        return self._processor.get_pending_transactions()
    
    def get_transaction_count(self, address: bytes) -> int:
        """Get the number of transactions for an address.
        
        Args:
            address (bytes): The address to check
            
        Returns:
            int: The number of transactions
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        if not address:
            raise ValueError("Invalid address")
        
        return self._processor.get_transaction_count(address)
    
    def get_transactions_by_address(self, address: bytes, start: int = 0, count: int = 10) -> List[Dict[str, Any]]:
        """Get transactions for an address.
        
        Args:
            address (bytes): The address to check
            start (int): The starting index
            count (int): The number of transactions to return
            
        Returns:
            List[Dict[str, Any]]: List of transaction information
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        if not address:
            raise ValueError("Invalid address")
        
        if start < 0 or count < 0:
            raise ValueError("Invalid pagination parameters")
        
        return self._processor.get_transactions_by_address(address, start, count)
    
    def is_initialized(self) -> bool:
        """Check if the transaction processor is initialized.
        
        Returns:
            bool: True if initialized, False otherwise.
        """
        return self._initialized
    
    def get_version(self) -> str:
        """Get the version of the transaction processor.
        
        Returns:
            str: The version string
        """
        return self._processor.get_version()
    
    def get_network_type(self) -> str:
        """Get the network type.
        
        Returns:
            str: The network type
            
        Raises:
            RuntimeError: If the processor is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        return self._processor.get_network_type()
    
    def get_total_transaction_count(self) -> int:
        """Get the total number of processed transactions.
        
        Returns:
            int: The number of transactions
            
        Raises:
            RuntimeError: If the processor is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Transaction Processor not initialized")
        
        return self._processor.get_total_transaction_count() 