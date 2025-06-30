"""
Satox SDK Network Manager Binding
"""

from typing import Dict, List, Optional, Any
import asyncio
from .error import SatoxError

class NetworkManager:
    """Wrapper for the C++ NetworkManager"""
    
    def __init__(self):
        self._core = None
        self._initialized = False
        self._connected = False
        self._callbacks = []
    
    async def initialize(self, config: Dict[str, Any]) -> bool:
        """Initialize network manager"""
        if self._initialized:
            raise SatoxError("NetworkManager already initialized")
        
        try:
            result = await self._call_native("initialize", config)
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize NetworkManager: {str(e)}")
    
    async def connect(self, host: str, port: int) -> bool:
        """Connect to network"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("connect", host, port)
            self._connected = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to connect to network: {str(e)}")
    
    async def disconnect(self) -> bool:
        """Disconnect from network"""
        if not self._connected:
            return True
        
        try:
            result = await self._call_native("disconnect")
            self._connected = False
            return result
        except Exception as e:
            raise SatoxError(f"Failed to disconnect: {str(e)}")
    
    async def send_message(self, message: Dict[str, Any]) -> bool:
        """Send message over network"""
        self._ensure_connected()
        
        try:
            return await self._call_native("send_message", message)
        except Exception as e:
            raise SatoxError(f"Failed to send message: {str(e)}")
    
    def _ensure_initialized(self):
        if not self._initialized:
            raise SatoxError("NetworkManager not initialized")
    
    def _ensure_connected(self):
        if not self._connected:
            raise SatoxError("Not connected to network")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        raise NotImplementedError("Native binding not implemented") 