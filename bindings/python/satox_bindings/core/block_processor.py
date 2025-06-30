"""Block Processor for Satox SDK.
Handles blockchain block processing operations.
"""

from typing import Optional, List, Dict
import json

class BlockProcessor:
    """Processes blockchain blocks and manages block-related operations."""
    
    def __init__(self):
        """Initialize the block processor."""
        self._initialized = False
        self._last_error = ""
        self._current_block = None
        self._block_cache = {}

    def initialize(self) -> bool:
        """Initialize the block processor."""
        try:
            self._initialized = True
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def shutdown(self) -> bool:
        """Shutdown the block processor."""
        try:
            self._initialized = False
            self._current_block = None
            self._block_cache.clear()
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def process_block(self, block_data: Dict) -> bool:
        """Process a single block."""
        if not self._initialized:
            self._last_error = "Block processor not initialized"
            return False
        
        try:
            # Process the block
            block_hash = block_data.get("hash")
            if not block_hash:
                self._last_error = "Invalid block data: missing hash"
                return False
            
            self._current_block = block_data
            self._block_cache[block_hash] = block_data
            
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def get_block(self, block_hash: str) -> Optional[Dict]:
        """Get a block by its hash."""
        if not self._initialized:
            self._last_error = "Block processor not initialized"
            return None
        
        if block_hash not in self._block_cache:
            self._last_error = f"Block with hash {block_hash} not found"
            return None
        
        return self._block_cache[block_hash]

    def get_current_block(self) -> Optional[Dict]:
        """Get the current block."""
        if not self._initialized:
            self._last_error = "Block processor not initialized"
            return None
        
        return self._current_block

    def get_last_error(self) -> str:
        """Get the last error message."""
        return self._last_error
        
    def clear_last_error(self) -> None:
        """Clear the last error message."""
        self._last_error = ""
