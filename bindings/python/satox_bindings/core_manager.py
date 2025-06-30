"""
Satox SDK Core Manager Binding
"""

from typing import Dict, List, Optional, Any
import asyncio
from .error import SatoxError
from .types import CoreConfig, CoreStatus

class CoreManager:
    """Wrapper for the C++ CoreManager"""
    
    def __init__(self):
        self._core = None  # Will be set by native binding
        self._initialized = False
        self._callbacks = []
    
    async def initialize(self, config: Optional[CoreConfig] = None) -> bool:
        """Initialize the core manager"""
        if self._initialized:
            raise SatoxError("CoreManager already initialized")
        
        try:
            # Call C++ core implementation
            result = await self._call_native("initialize", config or {})
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize CoreManager: {str(e)}")
    
    async def shutdown(self) -> bool:
        """Shutdown the core manager"""
        if not self._initialized:
            return True
        
        try:
            result = await self._call_native("shutdown")
            self._initialized = False
            return result
        except Exception as e:
            raise SatoxError(f"Failed to shutdown CoreManager: {str(e)}")
    
    async def get_status(self) -> CoreStatus:
        """Get core manager status"""
        self._ensure_initialized()
        return await self._call_native("get_status")
    
    def register_callback(self, callback):
        """Register a callback for core events"""
        self._callbacks.append(callback)
    
    def unregister_callback(self, callback):
        """Unregister a callback"""
        if callback in self._callbacks:
            self._callbacks.remove(callback)
    
    def _ensure_initialized(self):
        """Ensure the manager is initialized"""
        if not self._initialized:
            raise SatoxError("CoreManager not initialized")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        # This will be implemented by the native binding
        raise NotImplementedError("Native binding not implemented") 