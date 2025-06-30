"""
Tests for Satox game integration.
"""

import pytest
from datetime import datetime
from typing import Dict, Any
from satox.game import GameIntegration, GameAsset, GameError

@pytest.fixture
def game_integration():
    """Create a game integration instance for testing."""
    return GameIntegration()

@pytest.fixture
def sample_game_config() -> Dict[str, Any]:
    """Create a sample game configuration."""
    return {
        'name': 'Test Game',
        'version': '1.0.0',
        'max_players': 100,
        'asset_types': ['WEAPON', 'ARMOR', 'ITEM']
    }

@pytest.fixture
def sample_asset_data() -> Dict[str, Any]:
    """Create sample asset data."""
    return {
        'id': 'ASSET_1',
        'type': 'WEAPON',
        'name': 'Test Weapon',
        'description': 'A test weapon',
        'attributes': {
            'damage': 10,
            'durability': 100
        },
        'owner': 'PLAYER_1'
    }

def test_register_game(game_integration: GameIntegration, sample_game_config: Dict[str, Any]):
    """Test game registration."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_state = game_integration.get_game_state('GAME_1')
    
    assert game_state['config'] == sample_game_config
    assert 'assets' in game_state
    assert 'players' in game_state
    assert 'created_at' in game_state

def test_register_duplicate_game(game_integration: GameIntegration, sample_game_config: Dict[str, Any]):
    """Test registering a duplicate game."""
    game_integration.register_game('GAME_1', sample_game_config)
    
    with pytest.raises(GameError) as exc_info:
        game_integration.register_game('GAME_1', sample_game_config)
    assert "already registered" in str(exc_info.value)

def test_unregister_game(game_integration: GameIntegration, sample_game_config: Dict[str, Any]):
    """Test game unregistration."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_integration.unregister_game('GAME_1')
    
    with pytest.raises(GameError) as exc_info:
        game_integration.get_game_state('GAME_1')
    assert "not registered" in str(exc_info.value)

def test_create_game_asset(game_integration: GameIntegration, sample_game_config: Dict[str, Any], sample_asset_data: Dict[str, Any]):
    """Test game asset creation."""
    game_integration.register_game('GAME_1', sample_game_config)
    asset = game_integration.create_game_asset('GAME_1', sample_asset_data)
    
    assert asset.id == sample_asset_data['id']
    assert asset.type == sample_asset_data['type']
    assert asset.name == sample_asset_data['name']
    assert asset.description == sample_asset_data['description']
    assert asset.attributes == sample_asset_data['attributes']
    assert asset.owner == sample_asset_data['owner']
    assert isinstance(asset.created_at, datetime)
    assert isinstance(asset.updated_at, datetime)

def test_get_game_asset(game_integration: GameIntegration, sample_game_config: Dict[str, Any], sample_asset_data: Dict[str, Any]):
    """Test getting a game asset."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_integration.create_game_asset('GAME_1', sample_asset_data)
    
    asset = game_integration.get_game_asset('GAME_1', sample_asset_data['id'])
    assert asset is not None
    assert asset.id == sample_asset_data['id']

def test_update_game_asset(game_integration: GameIntegration, sample_game_config: Dict[str, Any], sample_asset_data: Dict[str, Any]):
    """Test updating a game asset."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_integration.create_game_asset('GAME_1', sample_asset_data)
    
    updated_asset = game_integration.update_game_asset(
        'GAME_1',
        sample_asset_data['id'],
        name='Updated Weapon',
        description='An updated test weapon'
    )
    
    assert updated_asset.name == 'Updated Weapon'
    assert updated_asset.description == 'An updated test weapon'
    assert updated_asset.attributes == sample_asset_data['attributes']

def test_delete_game_asset(game_integration: GameIntegration, sample_game_config: Dict[str, Any], sample_asset_data: Dict[str, Any]):
    """Test deleting a game asset."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_integration.create_game_asset('GAME_1', sample_asset_data)
    
    game_integration.delete_game_asset('GAME_1', sample_asset_data['id'])
    asset = game_integration.get_game_asset('GAME_1', sample_asset_data['id'])
    assert asset is None

def test_list_game_assets(game_integration: GameIntegration, sample_game_config: Dict[str, Any], sample_asset_data: Dict[str, Any]):
    """Test listing game assets."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_integration.create_game_asset('GAME_1', sample_asset_data)
    
    # Create another asset
    another_asset_data = sample_asset_data.copy()
    another_asset_data['id'] = 'ASSET_2'
    another_asset_data['owner'] = 'PLAYER_2'
    game_integration.create_game_asset('GAME_1', another_asset_data)
    
    # Test listing all assets
    all_assets = game_integration.list_game_assets('GAME_1')
    assert len(all_assets) == 2
    
    # Test listing assets by owner
    player1_assets = game_integration.list_game_assets('GAME_1', owner='PLAYER_1')
    assert len(player1_assets) == 1
    assert player1_assets[0].owner == 'PLAYER_1'

def test_transfer_game_asset(game_integration: GameIntegration, sample_game_config: Dict[str, Any], sample_asset_data: Dict[str, Any]):
    """Test transferring a game asset."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_integration.create_game_asset('GAME_1', sample_asset_data)
    
    transferred_asset = game_integration.transfer_game_asset(
        'GAME_1',
        sample_asset_data['id'],
        from_owner='PLAYER_1',
        to_owner='PLAYER_2'
    )
    
    assert transferred_asset.owner == 'PLAYER_2'
    assert isinstance(transferred_asset.updated_at, datetime)

def test_transfer_nonexistent_asset(game_integration: GameIntegration, sample_game_config: Dict[str, Any]):
    """Test transferring a nonexistent asset."""
    game_integration.register_game('GAME_1', sample_game_config)
    
    with pytest.raises(GameError) as exc_info:
        game_integration.transfer_game_asset(
            'GAME_1',
            'NONEXISTENT_ASSET',
            from_owner='PLAYER_1',
            to_owner='PLAYER_2'
        )
    assert "not found" in str(exc_info.value)

def test_register_player(game_integration: GameIntegration, sample_game_config: Dict[str, Any]):
    """Test player registration."""
    game_integration.register_game('GAME_1', sample_game_config)
    
    player_data = {
        'name': 'Test Player',
        'level': 1,
        'experience': 0
    }
    game_integration.register_player('GAME_1', 'PLAYER_1', player_data)
    
    retrieved_data = game_integration.get_player_data('GAME_1', 'PLAYER_1')
    assert retrieved_data['name'] == player_data['name']
    assert retrieved_data['level'] == player_data['level']
    assert retrieved_data['experience'] == player_data['experience']
    assert 'registered_at' in retrieved_data

def test_unregister_player(game_integration: GameIntegration, sample_game_config: Dict[str, Any]):
    """Test player unregistration."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_integration.register_player('GAME_1', 'PLAYER_1', {'name': 'Test Player'})
    
    game_integration.unregister_player('GAME_1', 'PLAYER_1')
    
    with pytest.raises(GameError) as exc_info:
        game_integration.get_player_data('GAME_1', 'PLAYER_1')
    assert "not registered" in str(exc_info.value)

def test_update_player_data(game_integration: GameIntegration, sample_game_config: Dict[str, Any]):
    """Test updating player data."""
    game_integration.register_game('GAME_1', sample_game_config)
    game_integration.register_player('GAME_1', 'PLAYER_1', {'name': 'Test Player', 'level': 1})
    
    game_integration.update_player_data('GAME_1', 'PLAYER_1', {'level': 2, 'experience': 100})
    
    player_data = game_integration.get_player_data('GAME_1', 'PLAYER_1')
    assert player_data['level'] == 2
    assert player_data['experience'] == 100
    assert 'updated_at' in player_data

def test_update_game_state(game_integration: GameIntegration, sample_game_config: Dict[str, Any]):
    """Test updating game state."""
    game_integration.register_game('GAME_1', sample_game_config)
    
    state_update = {
        'status': 'active',
        'current_players': 10,
        'last_update': datetime.utcnow().isoformat()
    }
    game_integration.update_game_state('GAME_1', state_update)
    
    game_state = game_integration.get_game_state('GAME_1')
    assert game_state['status'] == state_update['status']
    assert game_state['current_players'] == state_update['current_players']
    assert game_state['last_update'] == state_update['last_update']
    assert 'updated_at' in game_state 