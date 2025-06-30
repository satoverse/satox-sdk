from typing import Dict, List, Optional, Any
from ..core.satox_error import SatoxError
import asyncio
import logging
from datetime import datetime, timedelta
import random

class LoadBalancer:
    """Handles load balancing for network connections."""
    
    def __init__(self):
        """Initialize the load balancer."""
        self.nodes: Dict[str, Dict[str, Any]] = {}  # node_id -> node_info
        self.connection_pool: Dict[str, List[Any]] = {}  # node_id -> list of connections
        self.max_connections_per_node = 10
        self.connection_timeout = 300  # 5 minutes
        self.logger = logging.getLogger(__name__)
        
    async def add_node(self, node_id: str, host: str, port: int, weight: int = 1) -> None:
        """Add a node to the load balancer.
        
        Args:
            node_id: ID of the node
            host: Host address of the node
            port: Port of the node
            weight: Node weight for load balancing
            
        Raises:
            SatoxError: If node already exists
        """
        if node_id in self.nodes:
            raise SatoxError(f"Node {node_id} already exists")
            
        self.nodes[node_id] = {
            'host': host,
            'port': port,
            'weight': weight,
            'active_connections': 0,
            'last_seen': datetime.now()
        }
        
        self.connection_pool[node_id] = []
        
    async def remove_node(self, node_id: str) -> None:
        """Remove a node from the load balancer.
        
        Args:
            node_id: ID of the node to remove
            
        Raises:
            SatoxError: If node does not exist
        """
        if node_id not in self.nodes:
            raise SatoxError(f"Node {node_id} does not exist")
            
        # Close all connections
        for conn in self.connection_pool[node_id]:
            try:
                conn.close()
            except Exception as e:
                self.logger.error(f"Error closing connection: {str(e)}")
                
        # Remove node
        del self.nodes[node_id]
        del self.connection_pool[node_id]
        
    async def get_connection(self, node_id: Optional[str] = None) -> Any:
        """Get a connection to a node.
        
        Args:
            node_id: ID of the node to connect to (if None, a node will be selected)
            
        Returns:
            Connection to the node
            
        Raises:
            SatoxError: If no nodes are available or connection fails
        """
        if not self.nodes:
            raise SatoxError("No nodes available")
            
        if node_id:
            if node_id not in self.nodes:
                raise SatoxError(f"Node {node_id} does not exist")
                
            return await self._get_node_connection(node_id)
            
        # Select node based on load
        selected_node = self._select_node()
        return await self._get_node_connection(selected_node)
        
    async def release_connection(self, node_id: str, connection: Any) -> None:
        """Release a connection back to the pool.
        
        Args:
            node_id: ID of the node
            connection: Connection to release
            
        Raises:
            SatoxError: If node does not exist
        """
        if node_id not in self.nodes:
            raise SatoxError(f"Node {node_id} does not exist")
            
        # Update node info
        self.nodes[node_id]['active_connections'] -= 1
        
        # Add connection back to pool
        self.connection_pool[node_id].append(connection)
        
    async def _get_node_connection(self, node_id: str) -> Any:
        """Get a connection to a specific node.
        
        Args:
            node_id: ID of the node
            
        Returns:
            Connection to the node
            
        Raises:
            SatoxError: If connection fails
        """
        node = self.nodes[node_id]
        
        # Check connection limit
        if node['active_connections'] >= self.max_connections_per_node:
            raise SatoxError(f"Connection limit reached for node {node_id}")
            
        try:
            # Try to get connection from pool
            if self.connection_pool[node_id]:
                connection = self.connection_pool[node_id].pop()
                
                # Check if connection is still valid
                if await self._check_connection(connection):
                    node['active_connections'] += 1
                    return connection
                    
            # Create new connection
            connection = await self._create_connection(node['host'], node['port'])
            node['active_connections'] += 1
            return connection
            
        except Exception as e:
            raise SatoxError(f"Failed to get connection to node {node_id}: {str(e)}")
            
    async def _create_connection(self, host: str, port: int) -> Any:
        """Create a new connection.
        
        Args:
            host: Host address
            port: Port
            
        Returns:
            New connection
            
        Raises:
            SatoxError: If connection creation fails
        """
        try:
            reader, writer = await asyncio.open_connection(host, port)
            return {
                'reader': reader,
                'writer': writer,
                'created_at': datetime.now()
            }
            
        except Exception as e:
            raise SatoxError(f"Failed to create connection: {str(e)}")
            
    async def _check_connection(self, connection: Dict[str, Any]) -> bool:
        """Check if a connection is still valid.
        
        Args:
            connection: Connection to check
            
        Returns:
            True if connection is valid, False otherwise
        """
        try:
            # Check if connection is too old
            if datetime.now() - connection['created_at'] > timedelta(seconds=self.connection_timeout):
                return False
                
            # Try to write to connection
            connection['writer'].write(b'')
            await connection['writer'].drain()
            
            return True
            
        except Exception:
            return False
            
    def _select_node(self) -> str:
        """Select a node based on load.
        
        Returns:
            ID of the selected node
        """
        # Filter active nodes
        active_nodes = {
            node_id: node
            for node_id, node in self.nodes.items()
            if node['active_connections'] < self.max_connections_per_node
        }
        
        if not active_nodes:
            raise SatoxError("No active nodes available")
            
        # Calculate total weight
        total_weight = sum(node['weight'] for node in active_nodes.values())
        
        # Select node based on weight
        r = random.uniform(0, total_weight)
        current_weight = 0
        
        for node_id, node in active_nodes.items():
            current_weight += node['weight']
            if r <= current_weight:
                return node_id
                
        # Fallback to first node
        return next(iter(active_nodes))
        
    def get_node_info(self, node_id: str) -> Dict[str, Any]:
        """Get information about a node.
        
        Args:
            node_id: ID of the node
            
        Returns:
            Node information
            
        Raises:
            SatoxError: If node does not exist
        """
        if node_id not in self.nodes:
            raise SatoxError(f"Node {node_id} does not exist")
            
        return self.nodes[node_id].copy()
        
    def get_all_nodes(self) -> List[Dict[str, Any]]:
        """Get information about all nodes.
        
        Returns:
            List of node information
        """
        return [node.copy() for node in self.nodes.values()] 