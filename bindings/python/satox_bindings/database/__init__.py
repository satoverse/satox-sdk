"""
Satox SDK Database Module
"""

from typing import Dict, List, Optional, Any, Union, TypeVar, Generic
from datetime import datetime
import json
import sqlite3
import asyncio
import aiosqlite
from pathlib import Path

T = TypeVar('T')

class DatabaseModel(Generic[T]):
    """Base class for database models"""
    
    def __init__(self, table_name: str):
        self.table_name = table_name
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert model to dictionary"""
        raise NotImplementedError
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DatabaseModel':
        """Create model from dictionary"""
        raise NotImplementedError

class DatabaseManager:
    """Manages database operations"""
    
    def __init__(self, db_path: Union[str, Path]):
        self._db_path = Path(db_path)
        self._connection: Optional[aiosqlite.Connection] = None
        self._last_error = ""
    
    async def initialize(self) -> bool:
        """
        Initialize database manager
        
        Returns:
            bool: True if initialization successful, False otherwise
        """
        try:
            # Create database directory if it doesn't exist
            self._db_path.parent.mkdir(parents=True, exist_ok=True)
            
            # Connect to database
            self._connection = await aiosqlite.connect(str(self._db_path))
            
            # Enable foreign keys
            await self._connection.execute("PRAGMA foreign_keys = ON")
            
            # Create tables
            await self._create_tables()
            
            return True
            
        except Exception as e:
            self._last_error = f"Failed to initialize database: {str(e)}"
            return False
    
    async def shutdown(self) -> None:
        """Shutdown database manager"""
        if self._connection:
            await self._connection.close()
            self._connection = None
    
    async def create_table(self,
                          table_name: str,
                          columns: Dict[str, str],
                          primary_key: Optional[str] = None,
                          foreign_keys: Optional[List[Dict[str, str]]] = None) -> bool:
        """
        Create database table
        
        Args:
            table_name: Table name
            columns: Column definitions
            primary_key: Primary key column
            foreign_keys: Foreign key definitions
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            if not self._connection:
                self._last_error = "Database not initialized"
                return False
            
            # Build column definitions
            column_defs = []
            for name, definition in columns.items():
                if primary_key and name == primary_key:
                    column_defs.append(f"{name} {definition} PRIMARY KEY")
                else:
                    column_defs.append(f"{name} {definition}")
            
            # Add foreign keys
            if foreign_keys:
                for fk in foreign_keys:
                    column_defs.append(
                        f"FOREIGN KEY ({fk['column']}) "
                        f"REFERENCES {fk['references']} ({fk['on']})"
                    )
            
            # Create table
            query = f"CREATE TABLE IF NOT EXISTS {table_name} ({', '.join(column_defs)})"
            await self._connection.execute(query)
            await self._connection.commit()
            
            return True
            
        except Exception as e:
            self._last_error = f"Failed to create table: {str(e)}"
            return False
    
    async def insert(self,
                    table_name: str,
                    data: Dict[str, Any]) -> bool:
        """
        Insert data into table
        
        Args:
            table_name: Table name
            data: Data to insert
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            if not self._connection:
                self._last_error = "Database not initialized"
                return False
            
            # Build query
            columns = ', '.join(data.keys())
            placeholders = ', '.join(['?' for _ in data])
            query = f"INSERT INTO {table_name} ({columns}) VALUES ({placeholders})"
            
            # Execute query
            await self._connection.execute(query, list(data.values()))
            await self._connection.commit()
            
            return True
            
        except Exception as e:
            self._last_error = f"Failed to insert data: {str(e)}"
            return False
    
    async def update(self,
                    table_name: str,
                    data: Dict[str, Any],
                    where: Dict[str, Any]) -> bool:
        """
        Update data in table
        
        Args:
            table_name: Table name
            data: Data to update
            where: Where conditions
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            if not self._connection:
                self._last_error = "Database not initialized"
                return False
            
            # Build query
            set_clause = ', '.join([f"{k} = ?" for k in data.keys()])
            where_clause = ' AND '.join([f"{k} = ?" for k in where.keys()])
            query = f"UPDATE {table_name} SET {set_clause} WHERE {where_clause}"
            
            # Execute query
            await self._connection.execute(query, list(data.values()) + list(where.values()))
            await self._connection.commit()
            
            return True
            
        except Exception as e:
            self._last_error = f"Failed to update data: {str(e)}"
            return False
    
    async def delete(self,
                    table_name: str,
                    where: Dict[str, Any]) -> bool:
        """
        Delete data from table
        
        Args:
            table_name: Table name
            where: Where conditions
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            if not self._connection:
                self._last_error = "Database not initialized"
                return False
            
            # Build query
            where_clause = ' AND '.join([f"{k} = ?" for k in where.keys()])
            query = f"DELETE FROM {table_name} WHERE {where_clause}"
            
            # Execute query
            await self._connection.execute(query, list(where.values()))
            await self._connection.commit()
            
            return True
            
        except Exception as e:
            self._last_error = f"Failed to delete data: {str(e)}"
            return False
    
    async def query(self,
                   table_name: str,
                   columns: Optional[List[str]] = None,
                   where: Optional[Dict[str, Any]] = None,
                   order_by: Optional[str] = None,
                   limit: Optional[int] = None) -> List[Dict[str, Any]]:
        """
        Query data from table
        
        Args:
            table_name: Table name
            columns: Columns to select
            where: Where conditions
            order_by: Order by clause
            limit: Limit clause
            
        Returns:
            List[Dict[str, Any]]: Query results
        """
        try:
            if not self._connection:
                self._last_error = "Database not initialized"
                return []
            
            # Build query
            columns_str = ', '.join(columns) if columns else '*'
            query = f"SELECT {columns_str} FROM {table_name}"
            
            params = []
            if where:
                where_clause = ' AND '.join([f"{k} = ?" for k in where.keys()])
                query += f" WHERE {where_clause}"
                params.extend(where.values())
            
            if order_by:
                query += f" ORDER BY {order_by}"
            
            if limit:
                query += f" LIMIT {limit}"
            
            # Execute query
            async with self._connection.execute(query, params) as cursor:
                columns = [description[0] for description in cursor.description]
                return [dict(zip(columns, row)) for row in await cursor.fetchall()]
            
        except Exception as e:
            self._last_error = f"Failed to query data: {str(e)}"
            return []
    
    async def _create_tables(self) -> None:
        """Create default tables"""
        # TODO: Create default tables
        pass
    
    def get_last_error(self) -> str:
        """
        Get the last error message
        
        Returns:
            str: Last error message
        """
        return self._last_error
    
    def clear_last_error(self) -> None:
        """Clear the last error message"""
        self._last_error = ""

# Export the manager instance
database_manager = DatabaseManager("satox.db")
