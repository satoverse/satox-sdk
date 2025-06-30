"""
Satox SDK Blockchain Manager Binding
"""

from typing import Dict, List, Optional, Any
import asyncio
from .error import SatoxError
from .types import BlockInfo, TransactionInfo, NetworkType

class BlockchainManager:
    """Wrapper for the C++ BlockchainManager"""
    
    def __init__(self):
        self._core = None
        self._initialized = False
        self._connected = False
        self._callbacks = []
    
    async def initialize(self, config: Dict[str, Any]) -> bool:
        """Initialize blockchain manager"""
        if self._initialized:
            raise SatoxError("BlockchainManager already initialized")
        
        try:
            result = await self._call_native("initialize", config)
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize BlockchainManager: {str(e)}")
    
    async def connect(self, network: NetworkType, rpc_endpoint: str, credentials: Optional[Dict] = None) -> bool:
        """Connect to blockchain network"""
        self._ensure_initialized()
        
        try:
            config = {
                "network": network.value,
                "rpc_endpoint": rpc_endpoint,
                "credentials": credentials or {}
            }
            result = await self._call_native("connect", config)
            self._connected = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to connect to blockchain: {str(e)}")
    
    async def get_block_height(self) -> int:
        """Get current block height"""
        self._ensure_connected()
        
        try:
            return await self._call_native("get_block_height")
        except Exception as e:
            raise SatoxError(f"Failed to get block height: {str(e)}")
    
    async def get_block_info(self, block_hash: str) -> BlockInfo:
        """Get block information"""
        self._ensure_connected()
        
        try:
            result = await self._call_native("get_block_info", block_hash)
            return BlockInfo(**result)
        except Exception as e:
            raise SatoxError(f"Failed to get block info: {str(e)}")
    
    async def get_transaction_info(self, txid: str) -> TransactionInfo:
        """Get transaction information"""
        self._ensure_connected()
        
        try:
            result = await self._call_native("get_transaction_info", txid)
            return TransactionInfo(**result)
        except Exception as e:
            raise SatoxError(f"Failed to get transaction info: {str(e)}")
    
    async def send_raw_transaction(self, raw_tx: str) -> str:
        """Send raw transaction"""
        self._ensure_connected()
        
        try:
            return await self._call_native("send_raw_transaction", raw_tx)
        except Exception as e:
            raise SatoxError(f"Failed to send raw transaction: {str(e)}")
    
    def _ensure_initialized(self):
        if not self._initialized:
            raise SatoxError("BlockchainManager not initialized")
    
    def _ensure_connected(self):
        if not self._connected:
            raise SatoxError("Not connected to blockchain")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        raise NotImplementedError("Native binding not implemented") 