"""
Game integration implementation for Satox.
"""

import json
from typing import Dict, Any, Optional, List
from dataclasses import dataclass
from datetime import datetime

@dataclass
class GameAsset:
    """Represents a game asset."""
    id: str
    type: str
    name: str
    description: str
    attributes: Dict[str, Any]
    owner: str
    created_at: datetime
    updated_at: datetime

class GameError(Exception):
    """Base class for game errors."""
    pass

class GameIntegration:
    """Integrates Satox with games."""
    
    def __init__(self):
        self._assets: Dict[str, GameAsset] = {}
        self._game_state: Dict[str, Any] = {}

    def register_game(self, game_id: str, config: Dict[str, Any]):
        """Register a game with Satox."""
        if game_id in self._game_state:
            raise GameError(f"Game '{game_id}' is already registered")
        
        self._game_state[game_id] = {
            'config': config,
            'assets': {},
            'players': {},
            'created_at': datetime.utcnow()
        }

    def unregister_game(self, game_id: str):
        """Unregister a game from Satox."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        del self._game_state[game_id]

    def create_game_asset(self, game_id: str, asset_data: Dict[str, Any]) -> GameAsset:
        """Create a game asset."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")

        now = datetime.utcnow()
        asset = GameAsset(
            id=asset_data['id'],
            type=asset_data['type'],
            name=asset_data['name'],
            description=asset_data.get('description', ''),
            attributes=asset_data.get('attributes', {}),
            owner=asset_data['owner'],
            created_at=now,
            updated_at=now
        )
        
        self._game_state[game_id]['assets'][asset.id] = asset
        return asset

    def get_game_asset(self, game_id: str, asset_id: str) -> Optional[GameAsset]:
        """Get a game asset."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        return self._game_state[game_id]['assets'].get(asset_id)

    def update_game_asset(self, game_id: str, asset_id: str, **kwargs) -> GameAsset:
        """Update a game asset."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        assets = self._game_state[game_id]['assets']
        if asset_id not in assets:
            raise GameError(f"Asset '{asset_id}' not found in game '{game_id}'")

        asset = assets[asset_id]
        for key, value in kwargs.items():
            if hasattr(asset, key):
                setattr(asset, key, value)
        
        asset.updated_at = datetime.utcnow()
        return asset

    def delete_game_asset(self, game_id: str, asset_id: str):
        """Delete a game asset."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        assets = self._game_state[game_id]['assets']
        if asset_id not in assets:
            raise GameError(f"Asset '{asset_id}' not found in game '{game_id}'")
        
        del assets[asset_id]

    def list_game_assets(self, game_id: str, owner: Optional[str] = None) -> List[GameAsset]:
        """List game assets, optionally filtered by owner."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        assets = self._game_state[game_id]['assets'].values()
        if owner:
            return [asset for asset in assets if asset.owner == owner]
        return list(assets)

    def transfer_game_asset(self, game_id: str, asset_id: str, from_owner: str, to_owner: str) -> GameAsset:
        """Transfer a game asset between owners."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        assets = self._game_state[game_id]['assets']
        if asset_id not in assets:
            raise GameError(f"Asset '{asset_id}' not found in game '{game_id}'")

        asset = assets[asset_id]
        if asset.owner != from_owner:
            raise GameError(f"Asset '{asset_id}' is not owned by '{from_owner}'")

        asset.owner = to_owner
        asset.updated_at = datetime.utcnow()
        return asset

    def get_game_state(self, game_id: str) -> Dict[str, Any]:
        """Get the current state of a game."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        return self._game_state[game_id]

    def update_game_state(self, game_id: str, state_update: Dict[str, Any]):
        """Update the state of a game."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        self._game_state[game_id].update(state_update)
        self._game_state[game_id]['updated_at'] = datetime.utcnow()

    def register_player(self, game_id: str, player_id: str, player_data: Dict[str, Any]):
        """Register a player in a game."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        self._game_state[game_id]['players'][player_id] = {
            **player_data,
            'registered_at': datetime.utcnow()
        }

    def unregister_player(self, game_id: str, player_id: str):
        """Unregister a player from a game."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        if player_id not in self._game_state[game_id]['players']:
            raise GameError(f"Player '{player_id}' is not registered in game '{game_id}'")
        
        del self._game_state[game_id]['players'][player_id]

    def get_player_data(self, game_id: str, player_id: str) -> Dict[str, Any]:
        """Get player data from a game."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        if player_id not in self._game_state[game_id]['players']:
            raise GameError(f"Player '{player_id}' is not registered in game '{game_id}'")
        
        return self._game_state[game_id]['players'][player_id]

    def update_player_data(self, game_id: str, player_id: str, player_update: Dict[str, Any]):
        """Update player data in a game."""
        if game_id not in self._game_state:
            raise GameError(f"Game '{game_id}' is not registered")
        
        if player_id not in self._game_state[game_id]['players']:
            raise GameError(f"Player '{player_id}' is not registered in game '{game_id}'")
        
        self._game_state[game_id]['players'][player_id].update(player_update)
        self._game_state[game_id]['players'][player_id]['updated_at'] = datetime.utcnow() 