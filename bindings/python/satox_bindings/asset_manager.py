"""
Satox SDK Asset Manager Binding
"""

from typing import Dict, List, Optional, Any
import asyncio
from .error import SatoxError
from .types import Asset, AssetType

class AssetManager:
    """Wrapper for the C++ AssetManager"""
    
    def __init__(self):
        self._core = None
        self._initialized = False
        self._callbacks = []
    
    async def initialize(self, config: Dict[str, Any]) -> bool:
        """Initialize asset manager"""
        if self._initialized:
            raise SatoxError("AssetManager already initialized")
        
        try:
            result = await self._call_native("initialize", config)
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize AssetManager: {str(e)}")
    
    async def create_asset(self, config: Dict[str, Any]) -> Asset:
        """Create a new asset"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("create_asset", config)
            return Asset(**result)
        except Exception as e:
            raise SatoxError(f"Failed to create asset: {str(e)}")
    
    async def get_asset(self, asset_id: str) -> Asset:
        """Get asset information"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("get_asset", asset_id)
            return Asset(**result)
        except Exception as e:
            raise SatoxError(f"Failed to get asset: {str(e)}")
    
    async def update_asset(self, asset_id: str, metadata: Dict[str, Any]) -> bool:
        """Update asset metadata"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("update_asset", asset_id, metadata)
        except Exception as e:
            raise SatoxError(f"Failed to update asset: {str(e)}")
    
    async def transfer_asset(self, asset_id: str, from_address: str, to_address: str, amount: int) -> str:
        """Transfer asset"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("transfer_asset", asset_id, from_address, to_address, amount)
        except Exception as e:
            raise SatoxError(f"Failed to transfer asset: {str(e)}")
    
    async def get_balance(self, address: str, asset_id: Optional[str] = None) -> Dict[str, int]:
        """Get asset balance"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("get_balance", address, asset_id)
        except Exception as e:
            raise SatoxError(f"Failed to get balance: {str(e)}")
    
    async def list_assets(self, owner: Optional[str] = None, asset_type: Optional[AssetType] = None) -> List[Asset]:
        """List assets"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("list_assets", owner, asset_type.value if asset_type else None)
            return [Asset(**asset) for asset in result]
        except Exception as e:
            raise SatoxError(f"Failed to list assets: {str(e)}")
    
    def _ensure_initialized(self):
        if not self._initialized:
            raise SatoxError("AssetManager not initialized")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        raise NotImplementedError("Native binding not implemented") 