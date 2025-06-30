"""
Satox SDK IPFS Manager Binding
"""

from typing import Dict, List, Optional, Any
import asyncio
from .error import SatoxError

class IPFSManager:
    """Wrapper for the C++ IPFSManager"""
    
    def __init__(self):
        self._core = None
        self._initialized = False
        self._callbacks = []
    
    async def initialize(self, config: Dict[str, Any]) -> bool:
        """Initialize IPFS manager"""
        if self._initialized:
            raise SatoxError("IPFSManager already initialized")
        
        try:
            result = await self._call_native("initialize", config)
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize IPFSManager: {str(e)}")
    
    async def upload_file(self, file_path: str) -> str:
        """Upload file to IPFS"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("upload_file", file_path)
        except Exception as e:
            raise SatoxError(f"Failed to upload file: {str(e)}")
    
    async def download_file(self, ipfs_hash: str, output_path: str) -> bool:
        """Download file from IPFS"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("download_file", ipfs_hash, output_path)
        except Exception as e:
            raise SatoxError(f"Failed to download file: {str(e)}")
    
    async def upload_data(self, data: bytes) -> str:
        """Upload data to IPFS"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("upload_data", data)
        except Exception as e:
            raise SatoxError(f"Failed to upload data: {str(e)}")
    
    async def get_data(self, ipfs_hash: str) -> bytes:
        """Get data from IPFS"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("get_data", ipfs_hash)
        except Exception as e:
            raise SatoxError(f"Failed to get data: {str(e)}")
    
    def _ensure_initialized(self):
        if not self._initialized:
            raise SatoxError("IPFSManager not initialized")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        raise NotImplementedError("Native binding not implemented") 