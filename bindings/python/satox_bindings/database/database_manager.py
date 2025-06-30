from typing import Dict, List, Optional, Any, Union
from ..core.satox_error import SatoxError
import sqlite3
import json
import logging
from datetime import datetime
import os

class DatabaseManager:
    """Manages database operations."""
    
    def __init__(self, db_path: str):
        """Initialize the database manager.
        
        Args:
            db_path: Path to database file
        """
        self.db_path = db_path
        self.connection = None
        self.logger = logging.getLogger(__name__)
        
        # Create database directory if it doesn't exist
        os.makedirs(os.path.dirname(db_path), exist_ok=True)
        
    def connect(self) -> None:
        """Connect to the database.
        
        Raises:
            SatoxError: If connection fails
        """
        try:
            self.connection = sqlite3.connect(self.db_path)
            self.connection.row_factory = sqlite3.Row
            
        except Exception as e:
            raise SatoxError(f"Failed to connect to database: {str(e)}")
            
    def disconnect(self) -> None:
        """Disconnect from the database."""
        if self.connection:
            self.connection.close()
            self.connection = None
            
    def execute_query(self, query: str, params: Optional[tuple] = None) -> List[Dict[str, Any]]:
        """Execute a database query.
        
        Args:
            query: SQL query
            params: Query parameters
            
        Returns:
            Query results
            
        Raises:
            SatoxError: If query execution fails
        """
        if not self.connection:
            self.connect()
            
        try:
            cursor = self.connection.cursor()
            if params:
                cursor.execute(query, params)
            else:
                cursor.execute(query)
                
            self.connection.commit()
            
            # Convert results to list of dictionaries
            results = []
            for row in cursor.fetchall():
                results.append(dict(row))
                
            return results
            
        except Exception as e:
            self.connection.rollback()
            raise SatoxError(f"Failed to execute query: {str(e)}")
            
    def create_table(self, table_name: str, columns: Dict[str, str]) -> None:
        """Create a database table.
        
        Args:
            table_name: Name of table
            columns: Column definitions (name -> type)
            
        Raises:
            SatoxError: If table creation fails
        """
        query = f"CREATE TABLE IF NOT EXISTS {table_name} ("
        query += ", ".join(f"{name} {type_}" for name, type_ in columns.items())
        query += ")"
        
        self.execute_query(query)
        
    def insert(self, table_name: str, data: Dict[str, Any]) -> int:
        """Insert data into a table.
        
        Args:
            table_name: Name of table
            data: Data to insert
            
        Returns:
            ID of inserted row
            
        Raises:
            SatoxError: If insertion fails
        """
        columns = ", ".join(data.keys())
        placeholders = ", ".join("?" * len(data))
        query = f"INSERT INTO {table_name} ({columns}) VALUES ({placeholders})"
        
        self.execute_query(query, tuple(data.values()))
        return self.connection.lastrowid
        
    def update(self, table_name: str, data: Dict[str, Any], condition: str, params: tuple) -> None:
        """Update data in a table.
        
        Args:
            table_name: Name of table
            data: Data to update
            condition: WHERE condition
            params: Condition parameters
            
        Raises:
            SatoxError: If update fails
        """
        set_clause = ", ".join(f"{key} = ?" for key in data.keys())
        query = f"UPDATE {table_name} SET {set_clause} WHERE {condition}"
        
        self.execute_query(query, tuple(data.values()) + params)
        
    def delete(self, table_name: str, condition: str, params: tuple) -> None:
        """Delete data from a table.
        
        Args:
            table_name: Name of table
            condition: WHERE condition
            params: Condition parameters
            
        Raises:
            SatoxError: If deletion fails
        """
        query = f"DELETE FROM {table_name} WHERE {condition}"
        self.execute_query(query, params)
        
    def select(self, table_name: str, columns: List[str], condition: Optional[str] = None, params: Optional[tuple] = None) -> List[Dict[str, Any]]:
        """Select data from a table.
        
        Args:
            table_name: Name of table
            columns: Columns to select
            condition: WHERE condition
            params: Condition parameters
            
        Returns:
            Selected data
            
        Raises:
            SatoxError: If selection fails
        """
        query = f"SELECT {', '.join(columns)} FROM {table_name}"
        if condition:
            query += f" WHERE {condition}"
            
        return self.execute_query(query, params)
        
    def backup(self, backup_path: str) -> None:
        """Backup the database.
        
        Args:
            backup_path: Path to backup file
            
        Raises:
            SatoxError: If backup fails
        """
        try:
            # Create backup directory if it doesn't exist
            os.makedirs(os.path.dirname(backup_path), exist_ok=True)
            
            # Create backup connection
            backup_conn = sqlite3.connect(backup_path)
            
            # Backup database
            self.connection.backup(backup_conn)
            
            # Close backup connection
            backup_conn.close()
            
        except Exception as e:
            raise SatoxError(f"Failed to backup database: {str(e)}")
            
    def restore(self, backup_path: str) -> None:
        """Restore database from backup.
        
        Args:
            backup_path: Path to backup file
            
        Raises:
            SatoxError: If restore fails
        """
        try:
            # Create backup connection
            backup_conn = sqlite3.connect(backup_path)
            
            # Restore database
            backup_conn.backup(self.connection)
            
            # Close backup connection
            backup_conn.close()
            
        except Exception as e:
            raise SatoxError(f"Failed to restore database: {str(e)}")
            
    def vacuum(self) -> None:
        """Vacuum the database to reclaim space.
        
        Raises:
            SatoxError: If vacuum fails
        """
        try:
            self.execute_query("VACUUM")
            
        except Exception as e:
            raise SatoxError(f"Failed to vacuum database: {str(e)}")
            
    def get_table_info(self, table_name: str) -> List[Dict[str, Any]]:
        """Get information about a table.
        
        Args:
            table_name: Name of table
            
        Returns:
            Table information
            
        Raises:
            SatoxError: If info retrieval fails
        """
        return self.execute_query(f"PRAGMA table_info({table_name})")
        
    def get_tables(self) -> List[str]:
        """Get list of all tables.
        
        Returns:
            List of table names
        """
        results = self.execute_query("SELECT name FROM sqlite_master WHERE type='table'")
        return [row['name'] for row in results] 