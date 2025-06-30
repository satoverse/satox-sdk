"""
Satox SDK Wallet Manager Binding
"""

from typing import Dict, List, Optional, Any
import asyncio
from .error import SatoxError
from .types import Wallet

class WalletManager:
    """Wrapper for the C++ WalletManager"""
    
    def __init__(self):
        self._core = None
        self._initialized = False
        self._callbacks = []
    
    async def initialize(self, config: Dict[str, Any]) -> bool:
        """Initialize wallet manager"""
        if self._initialized:
            raise SatoxError("WalletManager already initialized")
        
        try:
            result = await self._call_native("initialize", config)
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize WalletManager: {str(e)}")
    
    async def create_wallet(self, password: Optional[str] = None) -> Wallet:
        """Create a new wallet"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("create_wallet", password)
            return Wallet(**result)
        except Exception as e:
            raise SatoxError(f"Failed to create wallet: {str(e)}")
    
    async def load_wallet(self, wallet_path: str, password: Optional[str] = None) -> Wallet:
        """Load existing wallet"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("load_wallet", wallet_path, password)
            return Wallet(**result)
        except Exception as e:
            raise SatoxError(f"Failed to load wallet: {str(e)}")
    
    async def get_balance(self, address: str) -> Dict[str, float]:
        """Get wallet balance"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("get_balance", address)
        except Exception as e:
            raise SatoxError(f"Failed to get balance: {str(e)}")
    
    async def send_transaction(self, from_address: str, to_address: str, amount: float, asset_id: Optional[str] = None) -> str:
        """Send transaction"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("send_transaction", from_address, to_address, amount, asset_id)
        except Exception as e:
            raise SatoxError(f"Failed to send transaction: {str(e)}")
    
    async def get_transaction_history(self, address: str) -> List[Dict[str, Any]]:
        """Get transaction history"""
        self._ensure_initialized()
        
        try:
            return await self._call_native("get_transaction_history", address)
        except Exception as e:
            raise SatoxError(f"Failed to get transaction history: {str(e)}")
    
    def _ensure_initialized(self):
        if not self._initialized:
            raise SatoxError("WalletManager not initialized")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        raise NotImplementedError("Native binding not implemented") 