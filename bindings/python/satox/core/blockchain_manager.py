"""
Blockchain Manager Python Bindings
"""

from typing import Dict, List, Optional, Any, Tuple
import satox_cpp
from satox_bindings.core.thread_safe_manager import ThreadSafeManager, ThreadState

class BlockchainManager(ThreadSafeManager):
    """Python wrapper for the C++ BlockchainManager class with full thread safety."""
    
    def __init__(self):
        """Initialize the Blockchain Manager."""
        super().__init__("BlockchainManager")
        self._manager = satox_cpp.BlockchainManager()
        self._block_cache = {}
        self._transaction_cache = {}
        self._network_cache = {}
        self._mempool_cache = {}
    
    def initialize(self) -> bool:
        """Initialize the blockchain manager.
        
        Returns:
            bool: True if initialization was successful, False otherwise.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_lock():
                if self._state == ThreadState.INITIALIZED:
                    return True
                
                try:
                    self._set_state(ThreadState.INITIALIZING)
                    success = self._manager.initialize()
                    if success:
                        self._set_state(ThreadState.INITIALIZED)
                        self._clear_error()
                        self.emit_event("initialized")
                        return True
                    else:
                        self._set_error("Failed to initialize blockchain manager")
                        return False
                except Exception as e:
                    self._set_error(f"Initialization error: {str(e)}")
                    return False
        finally:
            self._end_operation(operation_id)
    
    def shutdown(self) -> bool:
        """Shutdown the blockchain manager.
        
        Returns:
            bool: True if shutdown was successful, False otherwise.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_lock():
                if self._state == ThreadState.SHUTDOWN:
                    return True
                
                try:
                    self._set_state(ThreadState.SHUTTING_DOWN)
                    success = self._manager.shutdown()
                    if success:
                        self._set_state(ThreadState.SHUTDOWN)
                        self._clear_error()
                        self.emit_event("shutdown")
                        # Clear all caches
                        self._clear_all_caches()
                        return True
                    else:
                        self._set_error("Failed to shutdown blockchain manager")
                        return False
                except Exception as e:
                    self._set_error(f"Shutdown error: {str(e)}")
                    return False
        finally:
            self._end_operation(operation_id)
    
    def get_current_height(self) -> int:
        """Get the current blockchain height.
        
        Returns:
            int: Current block height
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                if not self._ensure_initialized():
                    raise RuntimeError(self.get_last_error())
                
                try:
                    height = self._manager.getCurrentHeight()
                    self.emit_event("height_queried", height)
                    return height
                except Exception as e:
                    self._set_error(f"Failed to get current height: {str(e)}")
                    raise RuntimeError(self.get_last_error())
        finally:
            self._end_operation(operation_id)
    
    def get_balance(self, address: str) -> float:
        """Get the balance for an address.
        
        Args:
            address (str): The address to get balance for
            
        Returns:
            float: The balance
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the address is invalid.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                if not self._ensure_initialized():
                    raise RuntimeError(self.get_last_error())
                
                if not address:
                    raise ValueError("Invalid address")
                
                # Check cache first
                cache_key = f"balance_{address}"
                cached_balance = self.get_cache(cache_key)
                if cached_balance is not None:
                    return cached_balance
                
                try:
                    balance = self._manager.getBalance(address)
                    # Cache balance for 30 seconds
                    self.set_cache(cache_key, balance, ttl=30.0)
                    self.emit_event("balance_queried", address, balance)
                    return balance
                except Exception as e:
                    self._set_error(f"Failed to get balance: {str(e)}")
                    raise RuntimeError(self.get_last_error())
        finally:
            self._end_operation(operation_id)
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get blockchain statistics.
        
        Returns:
            Dict[str, Any]: Blockchain statistics
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                if not self._ensure_initialized():
                    raise RuntimeError(self.get_last_error())
                
                # Check cache first
                cache_key = "blockchain_statistics"
                cached_stats = self.get_cache(cache_key)
                if cached_stats is not None:
                    return cached_stats
                
                try:
                    stats = self._manager.getStatistics()
                    # Cache statistics for 60 seconds
                    self.set_cache(cache_key, stats, ttl=60.0)
                    self.emit_event("statistics_queried", stats)
                    return stats
                except Exception as e:
                    self._set_error(f"Failed to get statistics: {str(e)}")
                    raise RuntimeError(self.get_last_error())
        finally:
            self._end_operation(operation_id)
    
    def get_block_info(self, block_hash: str) -> Dict[str, Any]:
        """Get information about a specific block.
        
        Args:
            block_hash (str): The block hash
            
        Returns:
            Dict[str, Any]: Block information
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the block hash is invalid.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                if not self._ensure_initialized():
                    raise RuntimeError(self.get_last_error())
                
                if not block_hash:
                    raise ValueError("Invalid block hash")
                
                # Check cache first
                cache_key = f"block_{block_hash}"
                cached_info = self.get_cache(cache_key)
                if cached_info is not None:
                    return cached_info
                
                try:
                    block_info = self._manager.getBlockInfo(block_hash)
                    # Cache the result for 5 minutes
                    self.set_cache(cache_key, block_info, ttl=300.0)
                    self.emit_event("block_info_queried", block_hash, block_info)
                    return block_info
                except Exception as e:
                    self._set_error(f"Failed to get block info: {str(e)}")
                    raise RuntimeError(self.get_last_error())
        finally:
            self._end_operation(operation_id)
    
    def get_transaction_info(self, txid: str) -> Dict[str, Any]:
        """Get information about a specific transaction.
        
        Args:
            txid (str): The transaction ID
            
        Returns:
            Dict[str, Any]: Transaction information
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the transaction ID is invalid.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                if not self._ensure_initialized():
                    raise RuntimeError(self.get_last_error())
                
                if not txid:
                    raise ValueError("Invalid transaction ID")
                
                # Check cache first
                cache_key = f"tx_{txid}"
                cached_info = self.get_cache(cache_key)
                if cached_info is not None:
                    return cached_info
                
                try:
                    tx_info = self._manager.getTransactionInfo(txid)
                    # Cache the result for 2 minutes
                    self.set_cache(cache_key, tx_info, ttl=120.0)
                    self.emit_event("transaction_info_queried", txid, tx_info)
                    return tx_info
                except Exception as e:
                    self._set_error(f"Failed to get transaction info: {str(e)}")
                    raise RuntimeError(self.get_last_error())
        finally:
            self._end_operation(operation_id)
    
    def get_network_info(self) -> Dict[str, Any]:
        """Get network information.
        
        Returns:
            Dict[str, Any]: Network information
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                if not self._ensure_initialized():
                    raise RuntimeError(self.get_last_error())
                
                # Check cache first
                cache_key = "network_info"
                cached_info = self.get_cache(cache_key)
                if cached_info is not None:
                    return cached_info
                
                try:
                    network_info = self._manager.getNetworkInfo()
                    # Cache for 30 seconds
                    self.set_cache(cache_key, network_info, ttl=30.0)
                    self.emit_event("network_info_queried", network_info)
                    return network_info
                except Exception as e:
                    self._set_error(f"Failed to get network info: {str(e)}")
                    raise RuntimeError(self.get_last_error())
        finally:
            self._end_operation(operation_id)
    
    def validate_address(self, address: str) -> bool:
        """Validate a Satoxcoin address.
        
        Args:
            address (str): The address to validate
            
        Returns:
            bool: True if valid, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                if not self._ensure_initialized():
                    raise RuntimeError(self.get_last_error())
                
                if not address:
                    return False
                
                try:
                    is_valid = self._manager.validateAddress(address)
                    self.emit_event("address_validated", address, is_valid)
                    return is_valid
                except Exception as e:
                    self._set_error(f"Failed to validate address: {str(e)}")
                    raise RuntimeError(self.get_last_error())
        finally:
            self._end_operation(operation_id)
    
    def get_mempool_info(self) -> Dict[str, Any]:
        """Get mempool information.
        
        Returns:
            Dict[str, Any]: Mempool information
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                if not self._ensure_initialized():
                    raise RuntimeError(self.get_last_error())
                
                # Check cache first
                cache_key = "mempool_info"
                cached_info = self.get_cache(cache_key)
                if cached_info is not None:
                    return cached_info
                
                try:
                    mempool_info = self._manager.getMempoolInfo()
                    # Cache for 10 seconds (mempool changes frequently)
                    self.set_cache(cache_key, mempool_info, ttl=10.0)
                    self.emit_event("mempool_info_queried", mempool_info)
                    return mempool_info
                except Exception as e:
                    self._set_error(f"Failed to get mempool info: {str(e)}")
                    raise RuntimeError(self.get_last_error())
        finally:
            self._end_operation(operation_id)
    
    def clear_cache(self) -> None:
        """Clear all caches in a thread-safe manner."""
        operation_id = self._begin_operation()
        try:
            with self._with_lock():
                super().clear_cache()
                self._block_cache.clear()
                self._transaction_cache.clear()
                self._network_cache.clear()
                self._mempool_cache.clear()
                self.emit_event("cache_cleared")
        finally:
            self._end_operation(operation_id)
    
    def get_cache_stats(self) -> Dict[str, Any]:
        """Get cache statistics.
        
        Returns:
            Dict[str, Any]: Cache statistics
        """
        operation_id = self._begin_operation()
        try:
            with self._with_read_lock():
                base_stats = super().get_cache_stats()
                additional_stats = {
                    "block_cache_size": len(self._block_cache),
                    "transaction_cache_size": len(self._transaction_cache),
                    "network_cache_size": len(self._network_cache),
                    "mempool_cache_size": len(self._mempool_cache)
                }
                base_stats.update(additional_stats)
                return base_stats
        finally:
            self._end_operation(operation_id)
    
    def _clear_all_caches(self) -> None:
        """Clear all internal caches."""
        self._block_cache.clear()
        self._transaction_cache.clear()
        self._network_cache.clear()
        self._mempool_cache.clear()
    
    def _dispose_impl(self) -> None:
        """Implementation-specific disposal logic."""
        try:
            if hasattr(self, '_manager'):
                self._manager.shutdown()
        except Exception as e:
            self._set_error(f"Disposal error: {str(e)}")
        finally:
            self._clear_all_caches() 