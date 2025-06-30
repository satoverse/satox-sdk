"""
Performance tests for Satox assets.
"""

import time
import pytest
from typing import List, Dict, Any
from satox.asset import AssetManager, Asset

@pytest.fixture
def asset_manager():
    """Create an asset manager for testing."""
    return AssetManager()

@pytest.fixture
def sample_assets(asset_manager: AssetManager) -> List[Asset]:
    """Create sample assets for testing."""
    assets = []
    for i in range(1000):
        asset = asset_manager.create_asset(
            name=f"ASSET_{i}",
            type="GAME_ITEM",
            amount=1000.0,
            owner=f"OWNER_{i % 10}",
            metadata={"rarity": "common", "level": i % 100}
        )
        assets.append(asset)
    return assets

def test_asset_creation_performance(asset_manager: AssetManager):
    """Test asset creation performance."""
    start_time = time.time()
    
    for i in range(1000):
        asset_manager.create_asset(
            name=f"PERF_ASSET_{i}",
            type="GAME_ITEM",
            amount=1000.0,
            owner=f"OWNER_{i % 10}",
            metadata={"rarity": "common", "level": i % 100}
        )
    
    end_time = time.time()
    creation_time = end_time - start_time
    
    # Assert that creation time is reasonable (less than 1 second for 1000 assets)
    assert creation_time < 1.0, f"Asset creation took too long: {creation_time:.2f} seconds"

def test_asset_query_performance(asset_manager: AssetManager, sample_assets: List[Asset]):
    """Test asset query performance."""
    start_time = time.time()
    
    # Query all assets
    all_assets = asset_manager.list_assets()
    
    # Query assets by owner
    for i in range(10):
        owner_assets = asset_manager.list_assets(owner=f"OWNER_{i}")
    
    end_time = time.time()
    query_time = end_time - start_time
    
    # Assert that query time is reasonable (less than 0.1 second for all queries)
    assert query_time < 0.1, f"Asset query took too long: {query_time:.2f} seconds"

def test_asset_transfer_performance(asset_manager: AssetManager, sample_assets: List[Asset]):
    """Test asset transfer performance."""
    start_time = time.time()
    
    # Perform transfers
    for i in range(100):
        asset_manager.transfer_asset(
            name=f"ASSET_{i}",
            from_owner=f"OWNER_{i % 10}",
            to_owner=f"OWNER_{(i + 1) % 10}",
            amount=100.0
        )
    
    end_time = time.time()
    transfer_time = end_time - start_time
    
    # Assert that transfer time is reasonable (less than 0.5 second for 100 transfers)
    assert transfer_time < 0.5, f"Asset transfer took too long: {transfer_time:.2f} seconds"

def test_asset_balance_performance(asset_manager: AssetManager, sample_assets: List[Asset]):
    """Test asset balance query performance."""
    start_time = time.time()
    
    # Query balances
    for i in range(100):
        for j in range(10):
            balance = asset_manager.get_asset_balance(
                name=f"ASSET_{i}",
                owner=f"OWNER_{j}"
            )
    
    end_time = time.time()
    balance_time = end_time - start_time
    
    # Assert that balance query time is reasonable (less than 0.2 second for 1000 queries)
    assert balance_time < 0.2, f"Balance query took too long: {balance_time:.2f} seconds"

def test_asset_metadata_performance(asset_manager: AssetManager, sample_assets: List[Asset]):
    """Test asset metadata operations performance."""
    start_time = time.time()
    
    # Store metadata
    for i in range(100):
        ipfs_hash = asset_manager.store_asset_metadata(
            name=f"ASSET_{i}",
            metadata={"updated": True, "timestamp": time.time()}
        )
    
    # Retrieve metadata
    for i in range(100):
        metadata = asset_manager.get_asset_metadata(f"ipfs://dummy_hash")
    
    end_time = time.time()
    metadata_time = end_time - start_time
    
    # Assert that metadata operations time is reasonable (less than 0.3 second for 200 operations)
    assert metadata_time < 0.3, f"Metadata operations took too long: {metadata_time:.2f} seconds"

def test_concurrent_operations_performance(asset_manager: AssetManager, sample_assets: List[Asset]):
    """Test performance of concurrent operations."""
    start_time = time.time()
    
    # Perform a mix of operations
    for i in range(100):
        # Create new asset
        asset_manager.create_asset(
            name=f"CONC_ASSET_{i}",
            type="GAME_ITEM",
            amount=1000.0,
            owner=f"OWNER_{i % 10}",
            metadata={"rarity": "common", "level": i % 100}
        )
        
        # Transfer existing asset
        asset_manager.transfer_asset(
            name=f"ASSET_{i}",
            from_owner=f"OWNER_{i % 10}",
            to_owner=f"OWNER_{(i + 1) % 10}",
            amount=100.0
        )
        
        # Query balance
        balance = asset_manager.get_asset_balance(
            name=f"ASSET_{i}",
            owner=f"OWNER_{(i + 1) % 10}"
        )
        
        # Update metadata
        ipfs_hash = asset_manager.store_asset_metadata(
            name=f"ASSET_{i}",
            metadata={"updated": True, "timestamp": time.time()}
        )
    
    end_time = time.time()
    concurrent_time = end_time - start_time
    
    # Assert that concurrent operations time is reasonable (less than 1 second for 400 operations)
    assert concurrent_time < 1.0, f"Concurrent operations took too long: {concurrent_time:.2f} seconds" 