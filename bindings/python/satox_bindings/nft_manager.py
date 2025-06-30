"""
Satox SDK NFT Manager Binding
"""

from typing import Dict, List, Optional, Any
import asyncio
from .error import SatoxError
from .types import NFT

class NFTManager:
    """Wrapper for the C++ NFTManager"""
    
    def __init__(self):
        self._core = None
        self._initialized = False
        self._callbacks = []
    
    async def initialize(self, config: Dict[str, Any]) -> bool:
        """Initialize NFT manager"""
        if self._initialized:
            raise SatoxError("NFTManager already initialized")
        
        try:
            result = await self._call_native("initialize", config)
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize NFTManager: {str(e)}")
    
    async def create_nft(self, config: Dict[str, Any]) -> NFT:
        """Create a new NFT"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("create_nft", config)
            return NFT(**result)
        except Exception as e:
            raise SatoxError(f"Failed to create NFT: {str(e)}")
    
    async def get_nft(self, nft_id: str) -> NFT:
        """Get NFT information"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("get_nft", nft_id)
            return NFT(**result)
        except Exception as e:
            raise SatoxError(f"Failed to get NFT: {str(e)}")
    
    async def update_nft(self, nft_id: str, metadata: Dict[str, Any]) -> bool:
        """Update NFT metadata"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("update_nft", nft_id, metadata)
        except Exception as e:
            raise SatoxError(f"Failed to update NFT: {str(e)}")
    
    async def transfer_nft(self, nft_id: str, from_address: str, to_address: str) -> str:
        """Transfer NFT"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("transfer_nft", nft_id, from_address, to_address)
        except Exception as e:
            raise SatoxError(f"Failed to transfer NFT: {str(e)}")
    
    async def list_nfts(self, owner: Optional[str] = None) -> List[NFT]:
        """List NFTs"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("list_nfts", owner)
            return [NFT(**nft) for nft in result]
        except Exception as e:
            raise SatoxError(f"Failed to list NFTs: {str(e)}")
    
    def _ensure_initialized(self):
        if not self._initialized:
            raise SatoxError("NFTManager not initialized")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        raise NotImplementedError("Native binding not implemented") 