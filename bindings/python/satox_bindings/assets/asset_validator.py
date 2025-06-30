from typing import Dict, Any, List, Optional
from ..core.error_handling import SatoxError

class AssetValidator:
    def __init__(self):
        """Initialize the AssetValidator."""
        self._required_fields = {'name', 'type', 'owner'}
        self._valid_types = {'token', 'nft', 'currency', 'collectible'}

    def validate_asset_data(self, asset_data: Dict[str, Any]) -> bool:
        """
        Validate asset data structure and content.
        
        Args:
            asset_data: Dictionary containing asset properties
            
        Returns:
            bool: True if asset data is valid
            
        Raises:
            SatoxError: If validation fails
        """
        try:
            # Check required fields
            missing_fields = self._required_fields - set(asset_data.keys())
            if missing_fields:
                raise SatoxError(f"Missing required fields: {missing_fields}")

            # Validate asset type
            if asset_data['type'] not in self._valid_types:
                raise SatoxError(f"Invalid asset type: {asset_data['type']}")

            # Validate name
            if not isinstance(asset_data['name'], str) or not asset_data['name'].strip():
                raise SatoxError("Asset name must be a non-empty string")

            # Validate owner
            if not isinstance(asset_data['owner'], str) or not asset_data['owner'].strip():
                raise SatoxError("Asset owner must be a non-empty string")

            return True
        except Exception as e:
            raise SatoxError(f"Asset validation failed: {str(e)}")

    def validate_asset_update(self, current_data: Dict[str, Any], update_data: Dict[str, Any]) -> bool:
        """
        Validate asset update data.
        
        Args:
            current_data: Current asset data
            update_data: New asset data to apply
            
        Returns:
            bool: True if update data is valid
            
        Raises:
            SatoxError: If validation fails
        """
        try:
            # Create a copy of current data with updates
            updated_data = current_data.copy()
            updated_data.update(update_data)
            
            # Validate the updated data
            return self.validate_asset_data(updated_data)
        except Exception as e:
            raise SatoxError(f"Asset update validation failed: {str(e)}")

    def validate_asset_transfer(self, asset_data: Dict[str, Any], new_owner: str) -> bool:
        """
        Validate asset transfer operation.
        
        Args:
            asset_data: Current asset data
            new_owner: New owner's identifier
            
        Returns:
            bool: True if transfer is valid
            
        Raises:
            SatoxError: If validation fails
        """
        try:
            if not isinstance(new_owner, str) or not new_owner.strip():
                raise SatoxError("New owner must be a non-empty string")

            if new_owner == asset_data['owner']:
                raise SatoxError("New owner must be different from current owner")

            return True
        except Exception as e:
            raise SatoxError(f"Asset transfer validation failed: {str(e)}")

    def validate_asset_batch(self, assets: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """
        Validate a batch of assets.
        
        Args:
            assets: List of asset data dictionaries
            
        Returns:
            List[Dict[str, Any]]: List of valid assets
            
        Raises:
            SatoxError: If validation fails
        """
        valid_assets = []
        errors = []

        for i, asset in enumerate(assets):
            try:
                if self.validate_asset_data(asset):
                    valid_assets.append(asset)
            except SatoxError as e:
                errors.append(f"Asset {i}: {str(e)}")

        if errors:
            raise SatoxError(f"Batch validation failed:\n" + "\n".join(errors))

        return valid_assets 