import json
import os
from typing import Dict, Any, List, Optional
from ..core.error_handling import SatoxError

class AssetStorage:
    def __init__(self, storage_path: str):
        """
        Initialize the AssetStorage with a storage path.
        
        Args:
            storage_path: Path to the storage directory
        """
        self._storage_path = storage_path
        self._ensure_storage_directory()

    def _ensure_storage_directory(self):
        """Ensure the storage directory exists."""
        try:
            os.makedirs(self._storage_path, exist_ok=True)
        except Exception as e:
            raise SatoxError(f"Failed to create storage directory: {str(e)}")

    def _get_asset_path(self, asset_id: str) -> str:
        """Get the full path for an asset file."""
        return os.path.join(self._storage_path, f"{asset_id}.json")

    def save_asset(self, asset_id: str, asset_data: Dict[str, Any]) -> bool:
        """
        Save asset data to storage.
        
        Args:
            asset_id: Unique identifier for the asset
            asset_data: Dictionary containing asset properties
            
        Returns:
            bool: True if save was successful
            
        Raises:
            SatoxError: If save operation fails
        """
        try:
            asset_path = self._get_asset_path(asset_id)
            with open(asset_path, 'w') as f:
                json.dump(asset_data, f, indent=2)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to save asset: {str(e)}")

    def load_asset(self, asset_id: str) -> Optional[Dict[str, Any]]:
        """
        Load asset data from storage.
        
        Args:
            asset_id: ID of the asset to load
            
        Returns:
            Optional[Dict[str, Any]]: Asset data if found, None otherwise
            
        Raises:
            SatoxError: If load operation fails
        """
        try:
            asset_path = self._get_asset_path(asset_id)
            if not os.path.exists(asset_path):
                return None

            with open(asset_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            raise SatoxError(f"Failed to load asset: {str(e)}")

    def delete_asset(self, asset_id: str) -> bool:
        """
        Delete asset data from storage.
        
        Args:
            asset_id: ID of the asset to delete
            
        Returns:
            bool: True if deletion was successful
            
        Raises:
            SatoxError: If deletion fails
        """
        try:
            asset_path = self._get_asset_path(asset_id)
            if os.path.exists(asset_path):
                os.remove(asset_path)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to delete asset: {str(e)}")

    def list_assets(self) -> List[str]:
        """
        Get a list of all stored asset IDs.
        
        Returns:
            List[str]: List of asset IDs
            
        Raises:
            SatoxError: If listing fails
        """
        try:
            assets = []
            for filename in os.listdir(self._storage_path):
                if filename.endswith('.json'):
                    assets.append(filename[:-5])  # Remove .json extension
            return assets
        except Exception as e:
            raise SatoxError(f"Failed to list assets: {str(e)}")

    def backup_assets(self, backup_path: str) -> bool:
        """
        Create a backup of all assets.
        
        Args:
            backup_path: Path to store the backup
            
        Returns:
            bool: True if backup was successful
            
        Raises:
            SatoxError: If backup fails
        """
        try:
            os.makedirs(backup_path, exist_ok=True)
            for asset_id in self.list_assets():
                asset_data = self.load_asset(asset_id)
                if asset_data:
                    backup_file = os.path.join(backup_path, f"{asset_id}.json")
                    with open(backup_file, 'w') as f:
                        json.dump(asset_data, f, indent=2)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to backup assets: {str(e)}")

    def restore_assets(self, backup_path: str) -> bool:
        """
        Restore assets from a backup.
        
        Args:
            backup_path: Path containing the backup files
            
        Returns:
            bool: True if restore was successful
            
        Raises:
            SatoxError: If restore fails
        """
        try:
            if not os.path.exists(backup_path):
                raise SatoxError(f"Backup path does not exist: {backup_path}")

            for filename in os.listdir(backup_path):
                if filename.endswith('.json'):
                    asset_id = filename[:-5]
                    backup_file = os.path.join(backup_path, filename)
                    with open(backup_file, 'r') as f:
                        asset_data = json.load(f)
                    self.save_asset(asset_id, asset_data)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to restore assets: {str(e)}") 