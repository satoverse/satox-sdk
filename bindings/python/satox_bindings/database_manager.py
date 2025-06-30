"""
Satox SDK Database Manager Binding
"""

from typing import Dict, List, Optional, Any, Union
from enum import Enum
import asyncio
from .error import SatoxError
from .types import DatabaseConfig, QueryResult, DatabaseType

class DatabaseManager:
    """Wrapper for the C++ DatabaseManager"""
    
    def __init__(self):
        self._core = None
        self._initialized = False
        self._connected = False
        self._callbacks = []
    
    async def initialize(self, config: DatabaseConfig) -> bool:
        """Initialize database manager"""
        if self._initialized:
            raise SatoxError("DatabaseManager already initialized")
        
        try:
            result = await self._call_native("initialize", config)
            self._initialized = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to initialize DatabaseManager: {str(e)}")
    
    async def connect(self, db_type: DatabaseType, connection_string: str) -> bool:
        """Connect to database"""
        self._ensure_initialized()
        
        try:
            result = await self._call_native("connect", db_type.value, connection_string)
            self._connected = result
            return result
        except Exception as e:
            raise SatoxError(f"Failed to connect to database: {str(e)}")
    
    async def execute_query(self, query: str, params: Optional[Dict] = None) -> QueryResult:
        """Execute a database query"""
        self._ensure_connected()
        
        try:
            result = await self._call_native("execute_query", query, params or {})
            return QueryResult(**result)
        except Exception as e:
            return QueryResult(success=False, rows=[], error=str(e))
    
    async def execute_transaction(self, queries: List[str]) -> bool:
        """Execute a transaction"""
        self._ensure_connected()
        
        try:
            return await self._call_native("execute_transaction", queries)
        except Exception as e:
            raise SatoxError(f"Failed to execute transaction: {str(e)}")
    
    def _ensure_initialized(self):
        if not self._initialized:
            raise SatoxError("DatabaseManager not initialized")
    
    def _ensure_connected(self):
        if not self._connected:
            raise SatoxError("Not connected to database")
    
    async def _call_native(self, method: str, *args) -> Any:
        """Call native C++ method"""
        raise NotImplementedError("Native binding not implemented") 