from typing import Dict, List, Optional, Any
from ..core.satox_manager import SatoxManager
from ..core.error_handling import SatoxError

class AssetManager:
    def __init__(self, satox_manager: SatoxManager):
        """Initialize the AssetManager with a reference to the SatoxManager."""
        self._satox_manager = satox_manager
        self._assets: Dict[str, Any] = {}

    def create_asset(self, asset_id: str, asset_data: Dict[str, Any]) -> bool:
        """
        Create a new asset with the given ID and data.
        
        Args:
            asset_id: Unique identifier for the asset
            asset_data: Dictionary containing asset properties
            
        Returns:
            bool: True if asset was created successfully
            
        Raises:
            SatoxError: If asset creation fails
        """
        try:
            if asset_id in self._assets:
                raise SatoxError(f"Asset with ID {asset_id} already exists")
            
            self._assets[asset_id] = asset_data
            return True
        except Exception as e:
            raise SatoxError(f"Failed to create asset: {str(e)}")

    def get_asset(self, asset_id: str) -> Optional[Dict[str, Any]]:
        """
        Retrieve an asset by its ID.
        
        Args:
            asset_id: ID of the asset to retrieve
            
        Returns:
            Optional[Dict[str, Any]]: Asset data if found, None otherwise
        """
        return self._assets.get(asset_id)

    def update_asset(self, asset_id: str, asset_data: Dict[str, Any]) -> bool:
        """
        Update an existing asset's data.
        
        Args:
            asset_id: ID of the asset to update
            asset_data: New asset data
            
        Returns:
            bool: True if update was successful
            
        Raises:
            SatoxError: If asset update fails
        """
        try:
            if asset_id not in self._assets:
                raise SatoxError(f"Asset with ID {asset_id} not found")
            
            self._assets[asset_id].update(asset_data)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to update asset: {str(e)}")

    def delete_asset(self, asset_id: str) -> bool:
        """
        Delete an asset by its ID.
        
        Args:
            asset_id: ID of the asset to delete
            
        Returns:
            bool: True if deletion was successful
            
        Raises:
            SatoxError: If asset deletion fails
        """
        try:
            if asset_id not in self._assets:
                raise SatoxError(f"Asset with ID {asset_id} not found")
            
            del self._assets[asset_id]
            return True
        except Exception as e:
            raise SatoxError(f"Failed to delete asset: {str(e)}")

    def list_assets(self) -> List[str]:
        """
        Get a list of all asset IDs.
        
        Returns:
            List[str]: List of asset IDs
        """
        return list(self._assets.keys())

    def search_assets(self, query: Dict[str, Any]) -> List[str]:
        """
        Search for assets matching the given query criteria.
        
        Args:
            query: Dictionary of search criteria
            
        Returns:
            List[str]: List of matching asset IDs
        """
        matching_assets = []
        for asset_id, asset_data in self._assets.items():
            if all(asset_data.get(k) == v for k, v in query.items()):
                matching_assets.append(asset_id)
        return matching_assets 