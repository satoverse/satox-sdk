"""
Satox SDK Asset Management Module
"""

from typing import Dict, List, Optional, Any
from datetime import datetime
import json
import hashlib
from enum import Enum

class AssetType(Enum):
    """Types of assets"""
    TOKEN = "token"
    CURRENCY = "currency"
    UTILITY = "utility"
    GOVERNANCE = "governance"

class Asset:
    """Represents a blockchain asset"""
    
    def __init__(self,
                 asset_id: str,
                 name: str,
                 symbol: str,
                 asset_type: AssetType,
                 total_supply: float,
                 decimals: int,
                 owner: str,
                 metadata: Optional[Dict[str, Any]] = None,
                 created_at: Optional[datetime] = None):
        self.asset_id = asset_id
        self.name = name
        self.symbol = symbol
        self.asset_type = asset_type
        self.total_supply = total_supply
        self.decimals = decimals
        self.owner = owner
        self.metadata = metadata or {}
        self.created_at = created_at or datetime.utcnow()
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert asset to dictionary"""
        return {
            'asset_id': self.asset_id,
            'name': self.name,
            'symbol': self.symbol,
            'asset_type': self.asset_type.value,
            'total_supply': self.total_supply,
            'decimals': self.decimals,
            'owner': self.owner,
            'metadata': self.metadata,
            'created_at': self.created_at.isoformat()
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Asset':
        """Create asset from dictionary"""
        return cls(
            asset_id=data['asset_id'],
            name=data['name'],
            symbol=data['symbol'],
            asset_type=AssetType(data['asset_type']),
            total_supply=data['total_supply'],
            decimals=data['decimals'],
            owner=data['owner'],
            metadata=data.get('metadata'),
            created_at=datetime.fromisoformat(data['created_at'])
        )

class AssetManager:
    """Manages asset operations"""
    
    def __init__(self):
        self._assets: Dict[str, Asset] = {}
        self._last_error = ""
    
    def create_asset(self,
                    name: str,
                    symbol: str,
                    asset_type: AssetType,
                    total_supply: float,
                    decimals: int,
                    owner: str,
                    metadata: Optional[Dict[str, Any]] = None) -> Optional[Asset]:
        """
        Create a new asset
        
        Args:
            name: Asset name
            symbol: Asset symbol
            asset_type: Type of asset
            total_supply: Total supply
            decimals: Number of decimal places
            owner: Owner address
            metadata: Additional metadata
            
        Returns:
            Optional[Asset]: Created asset if successful, None otherwise
        """
        try:
            # Validate asset parameters
            if not self._validate_asset_parameters(name, symbol, total_supply, decimals, owner):
                return None
            
            # Generate unique asset ID
            asset_id = self._generate_asset_id(name, symbol, owner)
            
            # Create asset
            asset = Asset(
                asset_id=asset_id,
                name=name,
                symbol=symbol,
                asset_type=asset_type,
                total_supply=total_supply,
                decimals=decimals,
                owner=owner,
                metadata=metadata
            )
            
            self._assets[asset_id] = asset
            return asset
            
        except Exception as e:
            self._last_error = f"Failed to create asset: {str(e)}"
            return None
    
    def get_asset(self, asset_id: str) -> Optional[Asset]:
        """
        Get asset by ID
        
        Args:
            asset_id: Asset ID
            
        Returns:
            Optional[Asset]: Asset if found, None otherwise
        """
        return self._assets.get(asset_id)
    
    def get_assets_by_owner(self, owner: str) -> List[Asset]:
        """
        Get all assets owned by an address
        
        Args:
            owner: Owner address
            
        Returns:
            List[Asset]: List of owned assets
        """
        return [asset for asset in self._assets.values() if asset.owner == owner]
    
    def update_asset(self,
                    asset_id: str,
                    metadata: Dict[str, Any]) -> bool:
        """
        Update asset metadata
        
        Args:
            asset_id: Asset ID
            metadata: New metadata
            
        Returns:
            bool: True if update successful, False otherwise
        """
        try:
            asset = self.get_asset(asset_id)
            if not asset:
                self._last_error = f"Asset not found: {asset_id}"
                return False
            
            asset.metadata.update(metadata)
            return True
            
        except Exception as e:
            self._last_error = f"Failed to update asset: {str(e)}"
            return False
    
    def _generate_asset_id(self, name: str, symbol: str, owner: str) -> str:
        """Generate unique asset ID"""
        data = f"{name}{symbol}{owner}{datetime.utcnow().isoformat()}"
        return hashlib.sha256(data.encode()).hexdigest()
    
    def _validate_asset_parameters(self,
                                 name: str,
                                 symbol: str,
                                 total_supply: float,
                                 decimals: int,
                                 owner: str) -> bool:
        """
        Validate asset parameters
        
        Args:
            name: Asset name
            symbol: Asset symbol
            total_supply: Total supply
            decimals: Number of decimal places
            owner: Owner address
            
        Returns:
            bool: True if parameters are valid, False otherwise
        """
        if not name or not symbol or not owner:
            self._last_error = "Invalid name, symbol, or owner"
            return False
        
        if total_supply <= 0:
            self._last_error = "Total supply must be greater than 0"
            return False
        
        if decimals < 0 or decimals > 18:
            self._last_error = "Decimals must be between 0 and 18"
            return False
        
        return True
    
    def get_last_error(self) -> str:
        """
        Get the last error message
        
        Returns:
            str: Last error message
        """
        return self._last_error
    
    def clear_last_error(self) -> None:
        """Clear the last error message"""
        self._last_error = ""

# Export the manager instance
asset_manager = AssetManager()

__all__ = [
    'AssetManager',
    'AssetValidator',
    'AssetStorage'
] 