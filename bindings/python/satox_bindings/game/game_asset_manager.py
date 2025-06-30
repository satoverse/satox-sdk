from typing import Dict, Any, Optional, List, Set
from datetime import datetime
from ..core.error_handling import SatoxError

class GameAssetManager:
    def __init__(self):
        """Initialize the GameAssetManager."""
        self._assets: Dict[str, Dict[str, Any]] = {}
        self._game_assets: Dict[str, Set[str]] = {}
        self._asset_games: Dict[str, Set[str]] = {}
        self._asset_types: Dict[str, Set[str]] = {}

    def create_asset(self, asset_id: str, asset_data: Dict[str, Any], asset_type: str) -> Dict[str, Any]:
        """
        Create a new game asset.
        
        Args:
            asset_id: Unique identifier for the asset
            asset_data: Asset data
            asset_type: Type of the asset
            
        Returns:
            Dict[str, Any]: Created asset data
            
        Raises:
            SatoxError: If asset already exists
        """
        if asset_id in self._assets:
            raise SatoxError(f"Asset already exists with ID {asset_id}")

        asset = {
            'id': asset_id,
            'type': asset_type,
            'data': asset_data,
            'created_at': datetime.now().isoformat(),
            'updated_at': datetime.now().isoformat(),
            'games': set()
        }
        
        self._assets[asset_id] = asset
        self._asset_games[asset_id] = set()
        
        if asset_type not in self._asset_types:
            self._asset_types[asset_type] = set()
        self._asset_types[asset_type].add(asset_id)
        
        return asset

    def get_asset(self, asset_id: str) -> Optional[Dict[str, Any]]:
        """
        Get an asset by ID.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            Optional[Dict[str, Any]]: Asset data if found
        """
        return self._assets.get(asset_id)

    def update_asset(self, asset_id: str, asset_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Update an asset's data.
        
        Args:
            asset_id: ID of the asset
            asset_data: New asset data
            
        Returns:
            Dict[str, Any]: Updated asset data
            
        Raises:
            SatoxError: If asset not found
        """
        if asset_id not in self._assets:
            raise SatoxError(f"Asset not found with ID {asset_id}")

        asset = self._assets[asset_id]
        asset['data'].update(asset_data)
        asset['updated_at'] = datetime.now().isoformat()
        
        return asset

    def delete_asset(self, asset_id: str) -> bool:
        """
        Delete an asset.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            bool: True if deletion was successful
            
        Raises:
            SatoxError: If asset not found
        """
        if asset_id not in self._assets:
            raise SatoxError(f"Asset not found with ID {asset_id}")

        # Remove asset from all games
        for game_id in self._asset_games[asset_id]:
            self._game_assets[game_id].remove(asset_id)
            if not self._game_assets[game_id]:
                del self._game_assets[game_id]

        # Remove asset from its type
        asset_type = self._assets[asset_id]['type']
        self._asset_types[asset_type].remove(asset_id)
        if not self._asset_types[asset_type]:
            del self._asset_types[asset_type]

        del self._assets[asset_id]
        del self._asset_games[asset_id]
        return True

    def list_assets(self) -> List[str]:
        """
        Get a list of all asset IDs.
        
        Returns:
            List[str]: List of asset IDs
        """
        return list(self._assets.keys())

    def add_asset_to_game(self, asset_id: str, game_id: str) -> bool:
        """
        Add an asset to a game.
        
        Args:
            asset_id: ID of the asset
            game_id: ID of the game
            
        Returns:
            bool: True if asset was added successfully
            
        Raises:
            SatoxError: If asset not found
        """
        if asset_id not in self._assets:
            raise SatoxError(f"Asset not found with ID {asset_id}")

        if game_id not in self._game_assets:
            self._game_assets[game_id] = set()

        self._game_assets[game_id].add(asset_id)
        self._asset_games[asset_id].add(game_id)
        return True

    def remove_asset_from_game(self, asset_id: str, game_id: str) -> bool:
        """
        Remove an asset from a game.
        
        Args:
            asset_id: ID of the asset
            game_id: ID of the game
            
        Returns:
            bool: True if asset was removed successfully
            
        Raises:
            SatoxError: If asset or game not found
        """
        if asset_id not in self._assets:
            raise SatoxError(f"Asset not found with ID {asset_id}")
        if game_id not in self._game_assets:
            raise SatoxError(f"Game not found with ID {game_id}")

        self._game_assets[game_id].remove(asset_id)
        self._asset_games[asset_id].remove(game_id)

        if not self._game_assets[game_id]:
            del self._game_assets[game_id]
        return True

    def get_game_assets(self, game_id: str) -> List[str]:
        """
        Get a list of assets in a game.
        
        Args:
            game_id: ID of the game
            
        Returns:
            List[str]: List of asset IDs
            
        Raises:
            SatoxError: If game not found
        """
        if game_id not in self._game_assets:
            raise SatoxError(f"Game not found with ID {game_id}")

        return list(self._game_assets[game_id])

    def get_asset_games(self, asset_id: str) -> List[str]:
        """
        Get a list of games an asset is in.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            List[str]: List of game IDs
            
        Raises:
            SatoxError: If asset not found
        """
        if asset_id not in self._assets:
            raise SatoxError(f"Asset not found with ID {asset_id}")

        return list(self._asset_games[asset_id])

    def get_assets_by_type(self, asset_type: str) -> List[str]:
        """
        Get a list of assets of a specific type.
        
        Args:
            asset_type: Type of assets to get
            
        Returns:
            List[str]: List of asset IDs
        """
        return list(self._asset_types.get(asset_type, set()))

    def get_asset_types(self) -> List[str]:
        """
        Get a list of all asset types.
        
        Returns:
            List[str]: List of asset types
        """
        return list(self._asset_types.keys())

    def search_assets(self, criteria: Dict[str, Any]) -> List[str]:
        """
        Search for assets matching criteria.
        
        Args:
            criteria: Search criteria
            
        Returns:
            List[str]: List of matching asset IDs
        """
        matching_assets = []
        for asset_id, asset in self._assets.items():
            matches = True
            for key, value in criteria.items():
                if key not in asset['data'] or asset['data'][key] != value:
                    matches = False
                    break
            if matches:
                matching_assets.append(asset_id)
        return matching_assets

    def get_asset_stats(self, asset_id: str) -> Dict[str, Any]:
        """
        Get asset statistics.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            Dict[str, Any]: Asset statistics
            
        Raises:
            SatoxError: If asset not found
        """
        if asset_id not in self._assets:
            raise SatoxError(f"Asset not found with ID {asset_id}")

        asset = self._assets[asset_id]
        return {
            'id': asset_id,
            'type': asset['type'],
            'games_count': len(self._asset_games[asset_id]),
            'created_at': asset['created_at'],
            'updated_at': asset['updated_at'],
            'data': asset['data']
        }

    def get_active_assets(self) -> List[str]:
        """
        Get a list of assets that are currently in at least one game.
        
        Returns:
            List[str]: List of active asset IDs
        """
        return [asset_id for asset_id, games in self._asset_games.items() if games]

    def get_inactive_assets(self) -> List[str]:
        """
        Get a list of assets that are not in any games.
        
        Returns:
            List[str]: List of inactive asset IDs
        """
        return [asset_id for asset_id, games in self._asset_games.items() if not games] 