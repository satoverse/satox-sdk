from typing import Dict, Any, List, Optional
from datetime import datetime
import json
import os
from ..core.error_handling import SatoxError

class AssetHistory:
    def __init__(self, history_path: str):
        """
        Initialize the AssetHistory with a history storage path.
        
        Args:
            history_path: Path to store asset history
        """
        self._history_path = history_path
        self._ensure_history_directory()

    def _ensure_history_directory(self):
        """Ensure the history directory exists."""
        try:
            os.makedirs(self._history_path, exist_ok=True)
        except Exception as e:
            raise SatoxError(f"Failed to create history directory: {str(e)}")

    def _get_history_path(self, asset_id: str) -> str:
        """Get the full path for an asset's history file."""
        return os.path.join(self._history_path, f"{asset_id}_history.json")

    def add_history_entry(self, asset_id: str, action: str, data: Dict[str, Any]) -> bool:
        """
        Add a new history entry for an asset.
        
        Args:
            asset_id: ID of the asset
            action: Type of action performed
            data: Additional data about the action
            
        Returns:
            bool: True if entry was added successfully
            
        Raises:
            SatoxError: If adding entry fails
        """
        try:
            history_path = self._get_history_path(asset_id)
            history = self._load_history(asset_id)
            
            entry = {
                'timestamp': datetime.utcnow().isoformat(),
                'action': action,
                'data': data
            }
            
            history.append(entry)
            
            with open(history_path, 'w') as f:
                json.dump(history, f, indent=2)
            
            return True
        except Exception as e:
            raise SatoxError(f"Failed to add history entry: {str(e)}")

    def _load_history(self, asset_id: str) -> List[Dict[str, Any]]:
        """
        Load history entries for an asset.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            List[Dict[str, Any]]: List of history entries
        """
        history_path = self._get_history_path(asset_id)
        if os.path.exists(history_path):
            with open(history_path, 'r') as f:
                return json.load(f)
        return []

    def get_history(self, asset_id: str, action: Optional[str] = None) -> List[Dict[str, Any]]:
        """
        Get history entries for an asset, optionally filtered by action type.
        
        Args:
            asset_id: ID of the asset
            action: Optional action type to filter by
            
        Returns:
            List[Dict[str, Any]]: List of history entries
            
        Raises:
            SatoxError: If retrieving history fails
        """
        try:
            history = self._load_history(asset_id)
            if action:
                return [entry for entry in history if entry['action'] == action]
            return history
        except Exception as e:
            raise SatoxError(f"Failed to get history: {str(e)}")

    def get_latest_entry(self, asset_id: str, action: Optional[str] = None) -> Optional[Dict[str, Any]]:
        """
        Get the most recent history entry for an asset.
        
        Args:
            asset_id: ID of the asset
            action: Optional action type to filter by
            
        Returns:
            Optional[Dict[str, Any]]: Latest history entry if found
            
        Raises:
            SatoxError: If retrieving entry fails
        """
        try:
            history = self.get_history(asset_id, action)
            if history:
                return max(history, key=lambda x: x['timestamp'])
            return None
        except Exception as e:
            raise SatoxError(f"Failed to get latest entry: {str(e)}")

    def clear_history(self, asset_id: str) -> bool:
        """
        Clear all history entries for an asset.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            bool: True if history was cleared successfully
            
        Raises:
            SatoxError: If clearing history fails
        """
        try:
            history_path = self._get_history_path(asset_id)
            if os.path.exists(history_path):
                os.remove(history_path)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to clear history: {str(e)}")

    def export_history(self, asset_id: str, export_path: str) -> bool:
        """
        Export asset history to a file.
        
        Args:
            asset_id: ID of the asset
            export_path: Path to export the history to
            
        Returns:
            bool: True if export was successful
            
        Raises:
            SatoxError: If export fails
        """
        try:
            history = self._load_history(asset_id)
            with open(export_path, 'w') as f:
                json.dump(history, f, indent=2)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to export history: {str(e)}")

    def import_history(self, asset_id: str, import_path: str) -> bool:
        """
        Import asset history from a file.
        
        Args:
            asset_id: ID of the asset
            import_path: Path to import the history from
            
        Returns:
            bool: True if import was successful
            
        Raises:
            SatoxError: If import fails
        """
        try:
            if not os.path.exists(import_path):
                raise SatoxError(f"Import file does not exist: {import_path}")

            with open(import_path, 'r') as f:
                history = json.load(f)

            history_path = self._get_history_path(asset_id)
            with open(history_path, 'w') as f:
                json.dump(history, f, indent=2)
            
            return True
        except Exception as e:
            raise SatoxError(f"Failed to import history: {str(e)}") 