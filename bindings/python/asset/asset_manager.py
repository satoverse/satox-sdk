"""
Asset management for Satoxcoin.
"""

from typing import Dict, List, Optional, Any
from dataclasses import dataclass
from datetime import datetime

@dataclass
class Asset:
    """Represents a Satoxcoin asset."""
    name: str
    type: str
    amount: float
    owner: str
    metadata: Dict[str, Any]
    created_at: datetime = datetime.now()
    updated_at: datetime = datetime.now()

class AssetError(Exception):
    """Base class for asset errors."""
    pass

class AssetManager:
    """Manages Satoxcoin assets."""
    
    def __init__(self):
        """Initialize the asset manager."""
        self._assets: Dict[str, Asset] = {}
        self._balances: Dict[str, Dict[str, float]] = {}  # owner -> asset_name -> amount
    
    def create_asset(self, name: str, type: str, amount: float, owner: str, metadata: Dict[str, Any]) -> Asset:
        """Create a new asset."""
        if name in self._assets:
            raise ValueError(f"Asset {name} already exists")
        
        asset = Asset(
            name=name,
            type=type,
            amount=amount,
            owner=owner,
            metadata=metadata
        )
        
        self._assets[name] = asset
        self._update_balance(owner, name, amount)
        return asset
    
    def transfer_asset(self, name: str, from_owner: str, to_owner: str, amount: float) -> None:
        """Transfer an asset between owners."""
        if name not in self._assets:
            raise ValueError(f"Asset {name} does not exist")
        
        # Check if sender has enough balance
        current_balance = self.get_asset_balance(name, from_owner)
        if current_balance < amount:
            raise ValueError(f"Insufficient balance for {name}")
        
        # Update balances
        self._update_balance(from_owner, name, -amount)
        self._update_balance(to_owner, name, amount)
        
        # Update asset owner if all amount is transferred
        if current_balance == amount:
            self._assets[name].owner = to_owner
    
    def get_asset_balance(self, name: str, owner: str) -> float:
        """Get the balance of an asset for an owner."""
        return self._balances.get(owner, {}).get(name, 0.0)
    
    def list_assets(self, owner: Optional[str] = None) -> List[Asset]:
        """List all assets, optionally filtered by owner."""
        if owner is None:
            return list(self._assets.values())
        return [asset for asset in self._assets.values() if asset.owner == owner]
    
    def store_asset_metadata(self, name: str, metadata: Dict[str, Any]) -> str:
        """Store asset metadata and return IPFS hash."""
        if name not in self._assets:
            raise ValueError(f"Asset {name} does not exist")
        
        # In a real implementation, this would store the metadata in IPFS
        # For now, we'll just update the asset's metadata
        self._assets[name].metadata.update(metadata)
        self._assets[name].updated_at = datetime.now()
        
        # Return a dummy IPFS hash
        return f"ipfs://dummy_hash"
    
    def get_asset_metadata(self, ipfs_hash: str) -> Dict[str, Any]:
        """Get asset metadata from IPFS hash."""
        # In a real implementation, this would retrieve metadata from IPFS
        # For now, we'll return a dummy metadata
        return {"ipfs_hash": ipfs_hash, "timestamp": datetime.now().isoformat()}
    
    def _update_balance(self, owner: str, asset_name: str, amount: float) -> None:
        """Update the balance of an asset for an owner."""
        if owner not in self._balances:
            self._balances[owner] = {}
        
        current_balance = self._balances[owner].get(asset_name, 0.0)
        new_balance = current_balance + amount
        
        if new_balance < 0:
            raise ValueError(f"Negative balance not allowed for {asset_name}")
        
        self._balances[owner][asset_name] = new_balance 