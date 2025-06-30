"""
Blockchain synchronization manager for Satox SDK.
"""

import asyncio
import logging
from typing import Dict, Any, Optional, List, Set, Callable
from datetime import datetime, timedelta
import time

from satox.core.error_handler import ErrorHandler, handle_errors, retry_on_error

error_handler = ErrorHandler()

class SyncManager:
    """Manages blockchain synchronization."""
    
    def __init__(self, blockchain_manager, config: Optional[Dict[str, Any]] = None):
        """Initialize sync manager.
        
        Args:
            blockchain_manager: Blockchain manager instance
            config: Optional configuration dictionary
        """
        self.blockchain_manager = blockchain_manager
        self.config = config or {}
        self.logger = logging.getLogger(__name__)
        self.error_handler = ErrorHandler()
        
        # Sync configuration
        self._batch_size = self.config.get('batch_size', 100)
        self._sync_interval = self.config.get('sync_interval', 1)
        self._max_retries = self.config.get('max_retries', 3)
        self._retry_delay = self.config.get('retry_delay', 1)
        self._max_parallel_requests = self.config.get('max_parallel_requests', 10)
        self._cache_ttl = self.config.get('cache_ttl', 300)  # 5 minutes
        
        # Sync state
        self._sync_lock = asyncio.Lock()
        self._last_sync_time = None
        self._current_height = 0
        self._target_height = 0
        self._sync_progress = 0
        self._is_syncing = False
        self._sync_errors = 0
        self._max_sync_errors = self.config.get('max_sync_errors', 10)
        
        # Caching
        self._block_cache: Dict[int, Dict[str, Any]] = {}
        self._tx_cache: Dict[str, Dict[str, Any]] = {}
        self._cache_timestamps: Dict[str, float] = {}
        
        # Event handlers
        self._event_handlers: Dict[str, List[Callable]] = {
            'sync_started': [],
            'sync_progress': [],
            'sync_completed': [],
            'sync_error': [],
            'block_added': [],
            'block_removed': []
        }
        
        # Metrics
        self._metrics = {
            'sync_count': 0,
            'total_blocks_synced': 0,
            'sync_duration': 0,
            'sync_errors': 0,
            'cache_hits': 0,
            'cache_misses': 0
        }
        
        # Background tasks
        self._sync_task = None
        self._cache_cleanup_task = None

    async def start(self):
        """Start sync manager."""
        if self._sync_task is None:
            self._sync_task = asyncio.create_task(self._sync_loop())
            self._cache_cleanup_task = asyncio.create_task(self._cache_cleanup_loop())
            self.logger.info("Sync manager started")

    async def stop(self):
        """Stop sync manager."""
        if self._sync_task:
            self._sync_task.cancel()
            self._sync_task = None
        if self._cache_cleanup_task:
            self._cache_cleanup_task.cancel()
            self._cache_cleanup_task = None
        self.logger.info("Sync manager stopped")

    @handle_errors(error_handler)
    async def sync(self, force: bool = False):
        """Synchronize blockchain.
        
        Args:
            force: Force sync even if already syncing
        """
        async with self._sync_lock:
            if self._is_syncing and not force:
                return
            
            try:
                self._is_syncing = True
                self._sync_errors = 0
                start_time = time.time()
                
                # Get current height
                self._current_height = await self.blockchain_manager.get_current_height()
                self._target_height = await self.blockchain_manager.get_latest_height()
                
                # Notify sync started
                self._notify_event('sync_started', {
                    'current_height': self._current_height,
                    'target_height': self._target_height
                })
                
                # Sync blocks in batches
                while self._current_height < self._target_height:
                    batch_end = min(
                        self._current_height + self._batch_size,
                        self._target_height
                    )
                    
                    # Sync batch
                    await self._sync_batch(self._current_height, batch_end)
                    
                    # Update progress
                    self._current_height = batch_end
                    self._sync_progress = (self._current_height / self._target_height) * 100
                    
                    # Notify progress
                    self._notify_event('sync_progress', {
                        'current_height': self._current_height,
                        'target_height': self._target_height,
                        'progress': self._sync_progress
                    })
                    
                    # Check for errors
                    if self._sync_errors >= self._max_sync_errors:
                        raise Exception("Too many sync errors")
                
                # Update metrics
                self._metrics['sync_count'] += 1
                self._metrics['total_blocks_synced'] += (self._target_height - self._current_height)
                self._metrics['sync_duration'] = time.time() - start_time
                
                # Notify sync completed
                self._notify_event('sync_completed', {
                    'current_height': self._current_height,
                    'target_height': self._target_height,
                    'duration': self._metrics['sync_duration']
                })
                
            except Exception as e:
                self._sync_errors += 1
                self._metrics['sync_errors'] += 1
                self._notify_event('sync_error', {
                    'error': str(e),
                    'current_height': self._current_height,
                    'target_height': self._target_height
                })
                raise
            finally:
                self._is_syncing = False
                self._last_sync_time = datetime.now()

    @retry_on_error(error_handler)
    async def _sync_batch(self, start_height: int, end_height: int):
        """Synchronize a batch of blocks.
        
        Args:
            start_height: Start block height
            end_height: End block height
        """
        # Get blocks in parallel
        tasks = []
        for height in range(start_height, end_height + 1):
            if height not in self._block_cache:
                tasks.append(self._get_block(height))
            if len(tasks) >= self._max_parallel_requests:
                await asyncio.gather(*tasks)
                tasks = []
        
        if tasks:
            await asyncio.gather(*tasks)

    @retry_on_error(error_handler)
    async def _get_block(self, height: int) -> Dict[str, Any]:
        """Get block and cache it.
        
        Args:
            height: Block height
            
        Returns:
            Block data
        """
        # Check cache
        if height in self._block_cache:
            cache_key = f"block_{height}"
            if time.time() - self._cache_timestamps.get(cache_key, 0) < self._cache_ttl:
                self._metrics['cache_hits'] += 1
                return self._block_cache[height]
        
        # Get block from blockchain
        block = await self.blockchain_manager.get_block(height)
        
        # Cache block
        self._block_cache[height] = block
        self._cache_timestamps[f"block_{height}"] = time.time()
        self._metrics['cache_misses'] += 1
        
        # Cache transactions
        for tx in block.get('transactions', []):
            tx_id = tx.get('id')
            if tx_id:
                self._tx_cache[tx_id] = tx
                self._cache_timestamps[f"tx_{tx_id}"] = time.time()
        
        # Notify block added
        self._notify_event('block_added', {
            'height': height,
            'block': block
        })
        
        return block

    async def _sync_loop(self):
        """Background sync loop."""
        while True:
            try:
                if not self._is_syncing:
                    await self.sync()
                await asyncio.sleep(self._sync_interval)
            except asyncio.CancelledError:
                break
            except Exception as e:
                self.logger.error(f"Error in sync loop: {str(e)}")
                await asyncio.sleep(self._retry_delay)

    async def _cache_cleanup_loop(self):
        """Background cache cleanup loop."""
        while True:
            try:
                current_time = time.time()
                # Clean block cache
                for height in list(self._block_cache.keys()):
                    cache_key = f"block_{height}"
                    if current_time - self._cache_timestamps.get(cache_key, 0) > self._cache_ttl:
                        del self._block_cache[height]
                        del self._cache_timestamps[cache_key]
                
                # Clean transaction cache
                for tx_id in list(self._tx_cache.keys()):
                    cache_key = f"tx_{tx_id}"
                    if current_time - self._cache_timestamps.get(cache_key, 0) > self._cache_ttl:
                        del self._tx_cache[tx_id]
                        del self._cache_timestamps[cache_key]
                
                await asyncio.sleep(self._cache_ttl)
            except asyncio.CancelledError:
                break
            except Exception as e:
                self.logger.error(f"Error in cache cleanup loop: {str(e)}")
                await asyncio.sleep(self._retry_delay)

    def register_event_handler(self, event: str, handler: Callable):
        """Register event handler.
        
        Args:
            event: Event name
            handler: Event handler function
        """
        if event in self._event_handlers:
            self._event_handlers[event].append(handler)

    def unregister_event_handler(self, event: str, handler: Callable):
        """Unregister event handler.
        
        Args:
            event: Event name
            handler: Event handler function
        """
        if event in self._event_handlers and handler in self._event_handlers[event]:
            self._event_handlers[event].remove(handler)

    def _notify_event(self, event: str, data: Dict[str, Any]):
        """Notify event handlers.
        
        Args:
            event: Event name
            data: Event data
        """
        if event in self._event_handlers:
            for handler in self._event_handlers[event]:
                try:
                    handler(data)
                except Exception as e:
                    self.logger.error(f"Error in event handler: {str(e)}")

    def get_metrics(self) -> Dict[str, Any]:
        """Get sync metrics.
        
        Returns:
            Dictionary of metrics
        """
        return {
            **self._metrics,
            'is_syncing': self._is_syncing,
            'current_height': self._current_height,
            'target_height': self._target_height,
            'sync_progress': self._sync_progress,
            'last_sync_time': self._last_sync_time,
            'cache_size': {
                'blocks': len(self._block_cache),
                'transactions': len(self._tx_cache)
            }
        } 