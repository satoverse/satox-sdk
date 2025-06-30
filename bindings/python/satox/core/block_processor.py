"""
Block Processor Python Bindings
"""

from typing import Dict, List, Optional, Any
import satox_cpp

class BlockProcessor:
    """Python wrapper for the C++ BlockProcessor class."""
    
    def __init__(self):
        """Initialize the Block Processor."""
        self._processor = satox_cpp.BlockProcessor()
        self._initialized = False
    
    def initialize(self) -> bool:
        """Initialize the block processor.
        
        Returns:
            bool: True if initialization was successful, False otherwise.
        """
        if not self._initialized:
            self._initialized = self._processor.initialize()
        return self._initialized
    
    def shutdown(self) -> bool:
        """Shutdown the block processor.
        
        Returns:
            bool: True if shutdown was successful, False otherwise.
        """
        if self._initialized:
            self._initialized = not self._processor.shutdown()
        return not self._initialized
    
    def process_block(self, block_data: bytes) -> bool:
        """Process a new block.
        
        Args:
            block_data (bytes): The block data to process
            
        Returns:
            bool: True if processing was successful, False otherwise
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        if not block_data:
            raise ValueError("Invalid block data")
        
        return self._processor.process_block(block_data)
    
    def validate_block(self, block_data: bytes) -> bool:
        """Validate a block.
        
        Args:
            block_data (bytes): The block data to validate
            
        Returns:
            bool: True if the block is valid, False otherwise
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        if not block_data:
            raise ValueError("Invalid block data")
        
        return self._processor.validate_block(block_data)
    
    def get_block_info(self, block_hash: bytes) -> Optional[Dict[str, Any]]:
        """Get information about a block.
        
        Args:
            block_hash (bytes): The hash of the block
            
        Returns:
            Optional[Dict[str, Any]]: Block information, or None if not found
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        if not block_hash:
            raise ValueError("Invalid block hash")
        
        return self._processor.get_block_info(block_hash)
    
    def get_latest_block(self) -> Optional[Dict[str, Any]]:
        """Get information about the latest block.
        
        Returns:
            Optional[Dict[str, Any]]: Latest block information, or None if no blocks
            
        Raises:
            RuntimeError: If the processor is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        return self._processor.get_latest_block()
    
    def get_block_by_height(self, height: int) -> Optional[Dict[str, Any]]:
        """Get information about a block by its height.
        
        Args:
            height (int): The height of the block
            
        Returns:
            Optional[Dict[str, Any]]: Block information, or None if not found
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        if height < 0:
            raise ValueError("Invalid block height")
        
        return self._processor.get_block_by_height(height)
    
    def get_block_transactions(self, block_hash: bytes) -> List[Dict[str, Any]]:
        """Get all transactions in a block.
        
        Args:
            block_hash (bytes): The hash of the block
            
        Returns:
            List[Dict[str, Any]]: List of transaction information
            
        Raises:
            RuntimeError: If the processor is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        if not block_hash:
            raise ValueError("Invalid block hash")
        
        return self._processor.get_block_transactions(block_hash)
    
    def is_initialized(self) -> bool:
        """Check if the block processor is initialized.
        
        Returns:
            bool: True if initialized, False otherwise.
        """
        return self._initialized
    
    def get_version(self) -> str:
        """Get the version of the block processor.
        
        Returns:
            str: The version string
        """
        return self._processor.get_version()
    
    def get_chain_id(self) -> str:
        """Get the chain ID.
        
        Returns:
            str: The chain ID
            
        Raises:
            RuntimeError: If the processor is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        return self._processor.get_chain_id()
    
    def get_network_type(self) -> str:
        """Get the network type.
        
        Returns:
            str: The network type
            
        Raises:
            RuntimeError: If the processor is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        return self._processor.get_network_type()
    
    def get_block_count(self) -> int:
        """Get the total number of processed blocks.
        
        Returns:
            int: The number of blocks
            
        Raises:
            RuntimeError: If the processor is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        return self._processor.get_block_count()
    
    def get_transaction_count(self) -> int:
        """Get the total number of processed transactions.
        
        Returns:
            int: The number of transactions
            
        Raises:
            RuntimeError: If the processor is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Block Processor not initialized")
        
        return self._processor.get_transaction_count() 