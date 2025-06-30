from typing import Dict, Any, Optional
from ..core.error_handling import SatoxError
from .asset_validator import AssetValidator

class AssetFactory:
    def __init__(self, validator: AssetValidator):
        """
        Initialize the AssetFactory with an AssetValidator.
        
        Args:
            validator: AssetValidator instance for validating created assets
        """
        self._validator = validator
        self._asset_templates = {
            'token': {
                'name': '',
                'type': 'token',
                'owner': '',
                'supply': 0,
                'decimals': 18,
                'transferable': True
            },
            'nft': {
                'name': '',
                'type': 'nft',
                'owner': '',
                'unique_id': '',
                'metadata': {},
                'transferable': True
            },
            'currency': {
                'name': '',
                'type': 'currency',
                'owner': '',
                'supply': 0,
                'decimals': 8,
                'transferable': True,
                'exchange_rate': 1.0
            },
            'collectible': {
                'name': '',
                'type': 'collectible',
                'owner': '',
                'edition': 1,
                'total_editions': 1,
                'metadata': {},
                'transferable': True
            }
        }

    def create_token(self, name: str, owner: str, supply: int, decimals: int = 18) -> Dict[str, Any]:
        """
        Create a new token asset.
        
        Args:
            name: Token name
            owner: Owner's identifier
            supply: Total token supply
            decimals: Number of decimal places
            
        Returns:
            Dict[str, Any]: Token asset data
            
        Raises:
            SatoxError: If creation fails
        """
        try:
            token_data = self._asset_templates['token'].copy()
            token_data.update({
                'name': name,
                'owner': owner,
                'supply': supply,
                'decimals': decimals
            })
            
            if self._validator.validate_asset_data(token_data):
                return token_data
            raise SatoxError("Token validation failed")
        except Exception as e:
            raise SatoxError(f"Failed to create token: {str(e)}")

    def create_nft(self, name: str, owner: str, unique_id: str, metadata: Dict[str, Any]) -> Dict[str, Any]:
        """
        Create a new NFT asset.
        
        Args:
            name: NFT name
            owner: Owner's identifier
            unique_id: Unique identifier for the NFT
            metadata: Additional NFT metadata
            
        Returns:
            Dict[str, Any]: NFT asset data
            
        Raises:
            SatoxError: If creation fails
        """
        try:
            nft_data = self._asset_templates['nft'].copy()
            nft_data.update({
                'name': name,
                'owner': owner,
                'unique_id': unique_id,
                'metadata': metadata
            })
            
            if self._validator.validate_asset_data(nft_data):
                return nft_data
            raise SatoxError("NFT validation failed")
        except Exception as e:
            raise SatoxError(f"Failed to create NFT: {str(e)}")

    def create_currency(self, name: str, owner: str, supply: int, exchange_rate: float = 1.0) -> Dict[str, Any]:
        """
        Create a new currency asset.
        
        Args:
            name: Currency name
            owner: Owner's identifier
            supply: Total currency supply
            exchange_rate: Exchange rate relative to base currency
            
        Returns:
            Dict[str, Any]: Currency asset data
            
        Raises:
            SatoxError: If creation fails
        """
        try:
            currency_data = self._asset_templates['currency'].copy()
            currency_data.update({
                'name': name,
                'owner': owner,
                'supply': supply,
                'exchange_rate': exchange_rate
            })
            
            if self._validator.validate_asset_data(currency_data):
                return currency_data
            raise SatoxError("Currency validation failed")
        except Exception as e:
            raise SatoxError(f"Failed to create currency: {str(e)}")

    def create_collectible(self, name: str, owner: str, edition: int, total_editions: int, metadata: Dict[str, Any]) -> Dict[str, Any]:
        """
        Create a new collectible asset.
        
        Args:
            name: Collectible name
            owner: Owner's identifier
            edition: Edition number
            total_editions: Total number of editions
            metadata: Additional collectible metadata
            
        Returns:
            Dict[str, Any]: Collectible asset data
            
        Raises:
            SatoxError: If creation fails
        """
        try:
            collectible_data = self._asset_templates['collectible'].copy()
            collectible_data.update({
                'name': name,
                'owner': owner,
                'edition': edition,
                'total_editions': total_editions,
                'metadata': metadata
            })
            
            if self._validator.validate_asset_data(collectible_data):
                return collectible_data
            raise SatoxError("Collectible validation failed")
        except Exception as e:
            raise SatoxError(f"Failed to create collectible: {str(e)}")

    def create_custom_asset(self, asset_type: str, **kwargs) -> Dict[str, Any]:
        """
        Create a custom asset with specified type and properties.
        
        Args:
            asset_type: Type of asset to create
            **kwargs: Additional asset properties
            
        Returns:
            Dict[str, Any]: Custom asset data
            
        Raises:
            SatoxError: If creation fails
        """
        try:
            if asset_type not in self._asset_templates:
                raise SatoxError(f"Unknown asset type: {asset_type}")

            asset_data = self._asset_templates[asset_type].copy()
            asset_data.update(kwargs)
            
            if self._validator.validate_asset_data(asset_data):
                return asset_data
            raise SatoxError("Custom asset validation failed")
        except Exception as e:
            raise SatoxError(f"Failed to create custom asset: {str(e)}") 