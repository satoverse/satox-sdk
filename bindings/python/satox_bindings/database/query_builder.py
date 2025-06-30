from typing import Dict, List, Optional, Any, Union
from ..core.satox_error import SatoxError

class QueryBuilder:
    """Builds SQL queries."""
    
    def __init__(self):
        """Initialize the query builder."""
        self.query = ""
        self.params = []
        self.table_name = ""
        self.columns = []
        self.conditions = []
        self.order_by = []
        self.group_by = []
        self.having = []
        self.limit = None
        self.offset = None
        self.join_tables = []
        self.join_conditions = []
        
    def select(self, table_name: str, columns: List[str]) -> 'QueryBuilder':
        """Start a SELECT query.
        
        Args:
            table_name: Name of table
            columns: Columns to select
            
        Returns:
            Self for chaining
        """
        self.table_name = table_name
        self.columns = columns
        return self
        
    def insert(self, table_name: str, data: Dict[str, Any]) -> 'QueryBuilder':
        """Start an INSERT query.
        
        Args:
            table_name: Name of table
            data: Data to insert
            
        Returns:
            Self for chaining
        """
        self.table_name = table_name
        columns = list(data.keys())
        values = list(data.values())
        
        self.query = f"INSERT INTO {table_name} ({', '.join(columns)}) VALUES ({', '.join(['?' for _ in values])})"
        self.params = values
        return self
        
    def update(self, table_name: str, data: Dict[str, Any]) -> 'QueryBuilder':
        """Start an UPDATE query.
        
        Args:
            table_name: Name of table
            data: Data to update
            
        Returns:
            Self for chaining
        """
        self.table_name = table_name
        set_clause = ", ".join(f"{key} = ?" for key in data.keys())
        self.query = f"UPDATE {table_name} SET {set_clause}"
        self.params = list(data.values())
        return self
        
    def delete(self, table_name: str) -> 'QueryBuilder':
        """Start a DELETE query.
        
        Args:
            table_name: Name of table
            
        Returns:
            Self for chaining
        """
        self.table_name = table_name
        self.query = f"DELETE FROM {table_name}"
        return self
        
    def where(self, condition: str, params: Optional[tuple] = None) -> 'QueryBuilder':
        """Add a WHERE condition.
        
        Args:
            condition: WHERE condition
            params: Condition parameters
            
        Returns:
            Self for chaining
        """
        self.conditions.append(condition)
        if params:
            self.params.extend(params)
        return self
        
    def order_by(self, columns: List[str], direction: str = "ASC") -> 'QueryBuilder':
        """Add ORDER BY clause.
        
        Args:
            columns: Columns to order by
            direction: Order direction (ASC/DESC)
            
        Returns:
            Self for chaining
        """
        self.order_by.extend(f"{col} {direction}" for col in columns)
        return self
        
    def group_by(self, columns: List[str]) -> 'QueryBuilder':
        """Add GROUP BY clause.
        
        Args:
            columns: Columns to group by
            
        Returns:
            Self for chaining
        """
        self.group_by.extend(columns)
        return self
        
    def having(self, condition: str, params: Optional[tuple] = None) -> 'QueryBuilder':
        """Add HAVING clause.
        
        Args:
            condition: HAVING condition
            params: Condition parameters
            
        Returns:
            Self for chaining
        """
        self.having.append(condition)
        if params:
            self.params.extend(params)
        return self
        
    def limit(self, limit: int) -> 'QueryBuilder':
        """Add LIMIT clause.
        
        Args:
            limit: Number of rows to limit
            
        Returns:
            Self for chaining
        """
        self.limit = limit
        return self
        
    def offset(self, offset: int) -> 'QueryBuilder':
        """Add OFFSET clause.
        
        Args:
            offset: Number of rows to offset
            
        Returns:
            Self for chaining
        """
        self.offset = offset
        return self
        
    def join(self, table: str, condition: str, params: Optional[tuple] = None) -> 'QueryBuilder':
        """Add a JOIN clause.
        
        Args:
            table: Table to join
            condition: Join condition
            params: Condition parameters
            
        Returns:
            Self for chaining
        """
        self.join_tables.append(table)
        self.join_conditions.append(condition)
        if params:
            self.params.extend(params)
        return self
        
    def build(self) -> tuple:
        """Build the query.
        
        Returns:
            Tuple of (query, params)
            
        Raises:
            SatoxError: If query building fails
        """
        try:
            if not self.query:
                # Build SELECT query
                self.query = f"SELECT {', '.join(self.columns)} FROM {self.table_name}"
                
                # Add joins
                for table, condition in zip(self.join_tables, self.join_conditions):
                    self.query += f" JOIN {table} ON {condition}"
                    
                # Add conditions
                if self.conditions:
                    self.query += " WHERE " + " AND ".join(self.conditions)
                    
                # Add group by
                if self.group_by:
                    self.query += " GROUP BY " + ", ".join(self.group_by)
                    
                # Add having
                if self.having:
                    self.query += " HAVING " + " AND ".join(self.having)
                    
                # Add order by
                if self.order_by:
                    self.query += " ORDER BY " + ", ".join(self.order_by)
                    
                # Add limit
                if self.limit is not None:
                    self.query += f" LIMIT {self.limit}"
                    
                # Add offset
                if self.offset is not None:
                    self.query += f" OFFSET {self.offset}"
                    
            return self.query, tuple(self.params)
            
        except Exception as e:
            raise SatoxError(f"Failed to build query: {str(e)}")
            
    def reset(self) -> None:
        """Reset the query builder."""
        self.query = ""
        self.params = []
        self.table_name = ""
        self.columns = []
        self.conditions = []
        self.order_by = []
        self.group_by = []
        self.having = []
        self.limit = None
        self.offset = None
        self.join_tables = []
        self.join_conditions = [] 