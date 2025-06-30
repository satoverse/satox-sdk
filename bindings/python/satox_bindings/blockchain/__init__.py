from .transaction_manager import TransactionManager
from .wallet_manager import WalletManager
from .smart_contract_manager import SmartContractManager

__all__ = ['TransactionManager', 'WalletManager', 'SmartContractManager']

"""
Satox SDK Blockchain Module
"""

from typing import Dict, List, Optional, Any
from datetime import datetime
import json

class Block:
    """Represents a blockchain block"""
    
    def __init__(self, 
                 block_hash: str,
                 previous_hash: str,
                 timestamp: datetime,
                 transactions: List[Dict[str, Any]],
                 nonce: int):
        self.block_hash = block_hash
        self.previous_hash = previous_hash
        self.timestamp = timestamp
        self.transactions = transactions
        self.nonce = nonce
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert block to dictionary"""
        return {
            'block_hash': self.block_hash,
            'previous_hash': self.previous_hash,
            'timestamp': self.timestamp.isoformat(),
            'transactions': self.transactions,
            'nonce': self.nonce
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Block':
        """Create block from dictionary"""
        return cls(
            block_hash=data['block_hash'],
            previous_hash=data['previous_hash'],
            timestamp=datetime.fromisoformat(data['timestamp']),
            transactions=data['transactions'],
            nonce=data['nonce']
        )

class BlockchainManager:
    """Manages blockchain operations"""
    
    def __init__(self):
        self._blocks: List[Block] = []
        self._last_error = ""
    
    def add_block(self, block: Block) -> bool:
        """
        Add a new block to the chain
        
        Args:
            block: Block to add
            
        Returns:
            bool: True if block added successfully, False otherwise
        """
        try:
            # Validate block
            if not self._validate_block(block):
                return False
                
            self._blocks.append(block)
            return True
            
        except Exception as e:
            self._last_error = f"Failed to add block: {str(e)}"
            return False
    
    def get_block(self, block_hash: str) -> Optional[Block]:
        """
        Get block by hash
        
        Args:
            block_hash: Block hash
            
        Returns:
            Optional[Block]: Block if found, None otherwise
        """
        for block in self._blocks:
            if block.block_hash == block_hash:
                return block
        return None
    
    def get_latest_block(self) -> Optional[Block]:
        """
        Get the latest block
        
        Returns:
            Optional[Block]: Latest block if chain not empty, None otherwise
        """
        return self._blocks[-1] if self._blocks else None
    
    def get_block_height(self) -> int:
        """
        Get current block height
        
        Returns:
            int: Current block height
        """
        return len(self._blocks)
    
    def _validate_block(self, block: Block) -> bool:
        """
        Validate a block
        
        Args:
            block: Block to validate
            
        Returns:
            bool: True if block is valid, False otherwise
        """
        # Check if block hash is valid
        if not block.block_hash:
            self._last_error = "Invalid block hash"
            return False
            
        # Check if previous hash matches
        if self._blocks and block.previous_hash != self._blocks[-1].block_hash:
            self._last_error = "Previous hash mismatch"
            return False
            
        # Check if transactions are valid
        if not block.transactions:
            self._last_error = "Block must contain transactions"
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
blockchain_manager = BlockchainManager() 