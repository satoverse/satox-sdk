"""
Satox SDK Security Manager Binding
"""

from typing import Dict, List, Optional, Any
import asyncio
from .error import SatoxError

class SecurityManager:
    """Wrapper for the C++ SecurityManager"""
    
    def __init__(self):
        self._core = None
        self._initialized = False
        self._callbacks = []
    
    async def initialize(self, config: Dict[str, Any]) -> bool:
        """Initialize security manager"""
        if self._initialized:
            raise SatoxError("SecurityManager already initialized")
        
        try:
            result = await self._call_native("initialize", config)
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize SecurityManager: {str(e)}")
    
    async def generate_key_pair(self) -> Dict[str, str]:
        """Generate key pair"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("generate_key_pair")
        except Exception as e:
            raise SatoxError(f"Failed to generate key pair: {str(e)}")
    
    async def encrypt_data(self, data: bytes, public_key: str) -> bytes:
        """Encrypt data"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("encrypt_data", data, public_key)
        except Exception as e:
            raise SatoxError(f"Failed to encrypt data: {str(e)}")
    
    async def decrypt_data(self, encrypted_data: bytes, private_key: str) -> bytes:
        """Decrypt data"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("decrypt_data", encrypted_data, private_key)
        except Exception as e:
            raise SatoxError(f"Failed to decrypt data: {str(e)}")
    
    async def sign_data(self, data: bytes, private_key: str) -> bytes:
        """Sign data"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("sign_data", data, private_key)
        except Exception as e:
            raise SatoxError(f"Failed to sign data: {str(e)}")
    
    async def verify_signature(self, data: bytes, signature: bytes, public_key: str) -> bool:
        """Verify signature"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("verify_signature", data, signature, public_key)
        except Exception as e:
            raise SatoxError(f"Failed to verify signature: {str(e)}")
    
    def _ensure_initialized(self):
        if not self._initialized:
            raise SatoxError("SecurityManager not initialized")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        raise NotImplementedError("Native binding not implemented") 