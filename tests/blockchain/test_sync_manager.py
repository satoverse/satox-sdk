"""
Tests for the blockchain sync manager.
"""

import pytest
import asyncio
from unittest.mock import Mock, patch, AsyncMock
from datetime import datetime, timedelta

from satox.blockchain.sync_manager import SyncManager

@pytest.fixture
def blockchain_manager():
    """Create a mock blockchain manager."""
    manager = Mock()
    manager.get_current_height = AsyncMock(return_value=0)
    manager.get_latest_height = AsyncMock(return_value=1000)
    manager.get_block = AsyncMock(return_value={
        'height': 0,
        'transactions': [
            {'id': 'tx1', 'data': 'test1'},
            {'id': 'tx2', 'data': 'test2'}
        ]
    })
    return manager

@pytest.fixture
def sync_manager(blockchain_manager):
    """Create a sync manager instance."""
    return SyncManager(blockchain_manager, {
        'batch_size': 10,
        'sync_interval': 0.1,
        'max_retries': 3,
        'retry_delay': 0.1,
        'max_parallel_requests': 5,
        'cache_ttl': 1
    })

@pytest.mark.asyncio
async def test_sync_manager_initialization(sync_manager):
    """Test sync manager initialization."""
    assert sync_manager._batch_size == 10
    assert sync_manager._sync_interval == 0.1
    assert sync_manager._max_retries == 3
    assert sync_manager._retry_delay == 0.1
    assert sync_manager._max_parallel_requests == 5
    assert sync_manager._cache_ttl == 1
    assert not sync_manager._is_syncing
    assert sync_manager._current_height == 0
    assert sync_manager._target_height == 0
    assert sync_manager._sync_progress == 0

@pytest.mark.asyncio
async def test_start_stop(sync_manager):
    """Test starting and stopping sync manager."""
    await sync_manager.start()
    assert sync_manager._sync_task is not None
    assert sync_manager._cache_cleanup_task is not None
    
    await sync_manager.stop()
    assert sync_manager._sync_task is None
    assert sync_manager._cache_cleanup_task is None

@pytest.mark.asyncio
async def test_sync(sync_manager):
    """Test blockchain synchronization."""
    # Mock event handler
    event_handler = Mock()
    sync_manager.register_event_handler('sync_started', event_handler)
    sync_manager.register_event_handler('sync_progress', event_handler)
    sync_manager.register_event_handler('sync_completed', event_handler)
    
    # Start sync
    await sync_manager.start()
    
    # Wait for sync to complete
    await asyncio.sleep(0.5)
    
    # Check event calls
    assert event_handler.call_count >= 3
    
    # Check metrics
    metrics = sync_manager.get_metrics()
    assert metrics['sync_count'] > 0
    assert metrics['total_blocks_synced'] > 0
    assert metrics['sync_duration'] > 0
    
    # Stop sync
    await sync_manager.stop()

@pytest.mark.asyncio
async def test_sync_batch(sync_manager):
    """Test batch synchronization."""
    # Mock block data
    blocks = {
        i: {
            'height': i,
            'transactions': [
                {'id': f'tx{i}_1', 'data': f'test{i}_1'},
                {'id': f'tx{i}_2', 'data': f'test{i}_2'}
            ]
        }
        for i in range(10)
    }
    sync_manager.blockchain_manager.get_block = AsyncMock(side_effect=lambda h: blocks[h])
    
    # Sync batch
    await sync_manager._sync_batch(0, 9)
    
    # Check cache
    assert len(sync_manager._block_cache) == 10
    assert len(sync_manager._tx_cache) == 20

@pytest.mark.asyncio
async def test_get_block(sync_manager):
    """Test getting block with caching."""
    # Get block
    block = await sync_manager._get_block(0)
    
    # Check block
    assert block['height'] == 0
    assert len(block['transactions']) == 2
    
    # Check cache
    assert 0 in sync_manager._block_cache
    assert 'tx1' in sync_manager._tx_cache
    assert 'tx2' in sync_manager._tx_cache
    
    # Get cached block
    cached_block = await sync_manager._get_block(0)
    assert cached_block == block
    assert sync_manager._metrics['cache_hits'] > 0

@pytest.mark.asyncio
async def test_cache_cleanup(sync_manager):
    """Test cache cleanup."""
    # Add blocks to cache
    for i in range(5):
        await sync_manager._get_block(i)
    
    # Wait for cache cleanup
    await asyncio.sleep(1.1)
    
    # Check cache
    assert len(sync_manager._block_cache) == 0
    assert len(sync_manager._tx_cache) == 0

@pytest.mark.asyncio
async def test_sync_errors(sync_manager):
    """Test sync error handling."""
    # Mock error
    sync_manager.blockchain_manager.get_block = AsyncMock(side_effect=Exception("Test error"))
    
    # Start sync
    await sync_manager.start()
    
    # Wait for sync
    await asyncio.sleep(0.5)
    
    # Check metrics
    metrics = sync_manager.get_metrics()
    assert metrics['sync_errors'] > 0
    
    # Stop sync
    await sync_manager.stop()

@pytest.mark.asyncio
async def test_event_handlers(sync_manager):
    """Test event handlers."""
    # Mock event handlers
    handlers = {
        'sync_started': Mock(),
        'sync_progress': Mock(),
        'sync_completed': Mock(),
        'sync_error': Mock(),
        'block_added': Mock(),
        'block_removed': Mock()
    }
    
    # Register handlers
    for event, handler in handlers.items():
        sync_manager.register_event_handler(event, handler)
    
    # Start sync
    await sync_manager.start()
    
    # Wait for sync
    await asyncio.sleep(0.5)
    
    # Check handler calls
    assert handlers['sync_started'].call_count > 0
    assert handlers['sync_progress'].call_count > 0
    assert handlers['sync_completed'].call_count > 0
    assert handlers['block_added'].call_count > 0
    
    # Stop sync
    await sync_manager.stop()

@pytest.mark.asyncio
async def test_parallel_requests(sync_manager):
    """Test parallel block requests."""
    # Mock block data
    blocks = {
        i: {
            'height': i,
            'transactions': [
                {'id': f'tx{i}_1', 'data': f'test{i}_1'},
                {'id': f'tx{i}_2', 'data': f'test{i}_2'}
            ]
        }
        for i in range(20)
    }
    sync_manager.blockchain_manager.get_block = AsyncMock(side_effect=lambda h: blocks[h])
    
    # Sync batch
    await sync_manager._sync_batch(0, 19)
    
    # Check parallel requests
    assert sync_manager.blockchain_manager.get_block.call_count == 20
    
    # Check cache
    assert len(sync_manager._block_cache) == 20
    assert len(sync_manager._tx_cache) == 40

@pytest.mark.asyncio
async def test_force_sync(sync_manager):
    """Test force sync."""
    # Start sync
    await sync_manager.start()
    
    # Wait for sync to start
    await asyncio.sleep(0.1)
    
    # Force sync
    await sync_manager.sync(force=True)
    
    # Check metrics
    metrics = sync_manager.get_metrics()
    assert metrics['sync_count'] > 0
    
    # Stop sync
    await sync_manager.stop() 