"""
Unit tests for AssetManager Python bindings
"""

import pytest
from satox.assets import AssetManager, Asset, AssetType, AssetStatus
from satox.types import SatoxError

@pytest.fixture
def asset_manager():
    """Create an AssetManager instance for testing."""
    manager = AssetManager()
    assert manager.initialize()
    yield manager
    manager.shutdown()

@pytest.fixture
def test_asset():
    """Create a test asset."""
    return Asset(
        id="test_asset_1",
        name="Test Asset",
        type=AssetType.TOKEN,
        owner="owner_address",
        amount=1000,
        created_at=1234567890,
        status=AssetStatus.ACTIVE,
        metadata={"description": "Test asset for unit testing"}
    )

def test_initialization():
    """Test AssetManager initialization."""
    manager = AssetManager()
    assert not manager._initialized
    assert manager.initialize()
    assert manager._initialized
    assert manager.shutdown()
    assert not manager._initialized

def test_asset_creation(asset_manager, test_asset):
    """Test asset creation and retrieval."""
    # Test asset creation
    assert asset_manager.create_asset(test_asset)
    
    # Test asset retrieval
    retrieved_asset = asset_manager.get_asset(test_asset.id)
    assert retrieved_asset.id == test_asset.id
    assert retrieved_asset.name == test_asset.name
    assert retrieved_asset.type == test_asset.type
    assert retrieved_asset.amount == test_asset.amount
    assert retrieved_asset.status == test_asset.status

def test_asset_update(asset_manager, test_asset):
    """Test asset update functionality."""
    # Create asset first
    asset_manager.create_asset(test_asset)
    
    # Update asset
    test_asset.amount = 2000
    test_asset.metadata["description"] = "Updated test asset"
    assert asset_manager.update_asset(test_asset)
    
    # Verify update
    updated_asset = asset_manager.get_asset(test_asset.id)
    assert updated_asset.amount == 2000
    assert updated_asset.metadata["description"] == "Updated test asset"

def test_asset_deletion(asset_manager, test_asset):
    """Test asset deletion."""
    # Create asset first
    asset_manager.create_asset(test_asset)
    
    # Delete asset
    assert asset_manager.delete_asset(test_asset.id)
    
    # Verify deletion
    with pytest.raises(SatoxError) as exc_info:
        asset_manager.get_asset(test_asset.id)
    assert exc_info.value.code == "ASSET_NOT_FOUND"

def test_asset_transfer(asset_manager, test_asset):
    """Test asset transfer functionality."""
    # Create asset first
    asset_manager.create_asset(test_asset)
    
    # Transfer asset
    new_owner = "new_owner_address"
    assert asset_manager.transfer_asset(test_asset.id, new_owner)
    
    # Verify transfer
    transferred_asset = asset_manager.get_asset(test_asset.id)
    assert transferred_asset.owner == new_owner

def test_asset_listing(asset_manager, test_asset):
    """Test asset listing functionality."""
    # Create multiple assets
    for i in range(3):
        asset = Asset(
            id=f"test_asset_{i}",
            name=f"Test Asset {i}",
            type=AssetType.TOKEN,
            owner="owner_address",
            amount=1000,
            created_at=1234567890,
            status=AssetStatus.ACTIVE,
            metadata={"description": f"Test asset {i}"}
        )
        asset_manager.create_asset(asset)
    
    # List all assets
    assets = asset_manager.list_assets()
    assert len(assets) == 3
    
    # List assets by owner
    owner_assets = asset_manager.list_assets_by_owner("owner_address")
    assert len(owner_assets) == 3
    
    # List assets by type
    token_assets = asset_manager.list_assets_by_type(AssetType.TOKEN)
    assert len(token_assets) == 3

def test_asset_validation(asset_manager):
    """Test asset validation."""
    # Test valid asset
    valid_asset = Asset(
        id="valid_asset",
        name="Valid Asset",
        type=AssetType.TOKEN,
        owner="owner_address",
        amount=1000,
        created_at=1234567890,
        status=AssetStatus.ACTIVE,
        metadata={"description": "Valid asset"}
    )
    assert asset_manager.validate_asset(valid_asset)
    
    # Test invalid asset
    invalid_asset = Asset(
        id="invalid_asset",
        name="",  # Invalid name
        type=AssetType.TOKEN,
        owner="",  # Invalid owner
        amount=-1000,  # Invalid amount
        created_at=1234567890,
        status=AssetStatus.ACTIVE,
        metadata={}
    )
    assert not asset_manager.validate_asset(invalid_asset)

def test_uninitialized_operations():
    """Test operations when not initialized."""
    manager = AssetManager()
    with pytest.raises(SatoxError) as exc_info:
        manager.create_asset(None)
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        manager.get_asset("test_id")
    assert exc_info.value.code == "NOT_INITIALIZED"

    with pytest.raises(SatoxError) as exc_info:
        manager.update_asset(None)
    assert exc_info.value.code == "NOT_INITIALIZED"

def test_invalid_inputs(asset_manager):
    """Test invalid input handling."""
    # Test with None inputs
    with pytest.raises(SatoxError) as exc_info:
        asset_manager.create_asset(None)
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        asset_manager.get_asset(None)
    assert exc_info.value.code == "INVALID_INPUT"

    with pytest.raises(SatoxError) as exc_info:
        asset_manager.update_asset(None)
    assert exc_info.value.code == "INVALID_INPUT"

def test_double_initialization():
    """Test double initialization handling."""
    manager = AssetManager()
    assert manager.initialize()
    assert manager.initialize()  # Should not raise an error
    assert manager.shutdown()

def test_double_shutdown():
    """Test double shutdown handling."""
    manager = AssetManager()
    assert manager.initialize()
    assert manager.shutdown()
    assert manager.shutdown()  # Should not raise an error 