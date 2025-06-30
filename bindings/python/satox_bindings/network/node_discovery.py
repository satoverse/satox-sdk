from typing import Dict, List, Optional, Any
from ..core.satox_manager import SatoxManager
from ..core.satox_error import SatoxError
import asyncio
import logging
from datetime import datetime, timedelta

class NodeDiscovery:
    """Manages node discovery and peer management."""
    
    def __init__(self, satox_manager: SatoxManager):
        """Initialize the node discovery manager.
        
        Args:
            satox_manager: Reference to the SatoxManager instance
        """
        self.satox_manager = satox_manager
        self.nodes: Dict[str, Dict[str, Any]] = {}  # node_id -> node_info
        self.bootstrap_nodes: List[Dict[str, Any]] = []  # List of bootstrap nodes
        self.is_running = False
        self.logger = logging.getLogger(__name__)
        
    async def start(self) -> None:
        """Start the node discovery process.
        
        Raises:
            SatoxError: If node discovery is already running
        """
        if self.is_running:
            raise SatoxError("Node discovery is already running")
            
        try:
            self.is_running = True
            
            # Start discovery tasks
            asyncio.create_task(self._discover_nodes())
            asyncio.create_task(self._cleanup_stale_nodes())
            
            self.logger.info("Node discovery started")
            
        except Exception as e:
            raise SatoxError(f"Failed to start node discovery: {str(e)}")
            
    async def stop(self) -> None:
        """Stop the node discovery process.
        
        Raises:
            SatoxError: If node discovery is not running
        """
        if not self.is_running:
            raise SatoxError("Node discovery is not running")
            
        try:
            self.is_running = False
            self.logger.info("Node discovery stopped")
            
        except Exception as e:
            raise SatoxError(f"Failed to stop node discovery: {str(e)}")
            
    def add_bootstrap_node(self, host: str, port: int) -> None:
        """Add a bootstrap node.
        
        Args:
            host: Host address of the bootstrap node
            port: Port of the bootstrap node
        """
        self.bootstrap_nodes.append({
            'host': host,
            'port': port,
            'last_seen': datetime.now()
        })
        
    def remove_bootstrap_node(self, host: str, port: int) -> None:
        """Remove a bootstrap node.
        
        Args:
            host: Host address of the bootstrap node
            port: Port of the bootstrap node
        """
        self.bootstrap_nodes = [
            node for node in self.bootstrap_nodes
            if node['host'] != host or node['port'] != port
        ]
        
    def get_known_nodes(self) -> List[Dict[str, Any]]:
        """Get list of known nodes.
        
        Returns:
            List of known nodes with their information
        """
        return list(self.nodes.values())
        
    def get_bootstrap_nodes(self) -> List[Dict[str, Any]]:
        """Get list of bootstrap nodes.
        
        Returns:
            List of bootstrap nodes with their information
        """
        return list(self.bootstrap_nodes)
        
    async def _discover_nodes(self) -> None:
        """Discover new nodes in the network."""
        while self.is_running:
            try:
                # Try bootstrap nodes first
                for node in self.bootstrap_nodes:
                    try:
                        await self._discover_from_node(node['host'], node['port'])
                    except Exception as e:
                        self.logger.error(f"Failed to discover from bootstrap node {node['host']}:{node['port']}: {str(e)}")
                        
                # Try known nodes
                for node_id, node in self.nodes.items():
                    try:
                        await self._discover_from_node(node['host'], node['port'])
                    except Exception as e:
                        self.logger.error(f"Failed to discover from node {node_id}: {str(e)}")
                        
                # Wait before next discovery
                await asyncio.sleep(300)  # 5 minutes
                
            except Exception as e:
                self.logger.error(f"Error in node discovery: {str(e)}")
                await asyncio.sleep(5)
                
    async def _discover_from_node(self, host: str, port: int) -> None:
        """Discover nodes from a specific node.
        
        Args:
            host: Host address of the node
            port: Port of the node
        """
        try:
            # Connect to node
            reader, writer = await asyncio.open_connection(host, port)
            
            # Send discovery request
            message = {
                'type': 'discovery_request',
                'data': {
                    'node_id': self.satox_manager.node_id,
                    'host': self.satox_manager.host,
                    'port': self.satox_manager.port
                }
            }
            
            writer.write(str(message).encode())
            await writer.drain()
            
            # Read response
            data = await reader.read(1024)
            response = eval(data.decode())
            
            # Process discovered nodes
            if response['type'] == 'discovery_response':
                for node in response['data']['nodes']:
                    self._add_node(node)
                    
            # Close connection
            writer.close()
            await writer.wait_closed()
            
        except Exception as e:
            raise SatoxError(f"Failed to discover from node {host}:{port}: {str(e)}")
            
    def _add_node(self, node_info: Dict[str, Any]) -> None:
        """Add a node to the known nodes list.
        
        Args:
            node_info: Information about the node
        """
        node_id = node_info['node_id']
        
        if node_id != self.satox_manager.node_id:  # Don't add self
            self.nodes[node_id] = {
                'node_id': node_id,
                'host': node_info['host'],
                'port': node_info['port'],
                'last_seen': datetime.now()
            }
            
    async def _cleanup_stale_nodes(self) -> None:
        """Remove stale nodes from the known nodes list."""
        while self.is_running:
            try:
                now = datetime.now()
                stale_time = timedelta(hours=1)  # Nodes not seen in 1 hour are considered stale
                
                # Remove stale nodes
                self.nodes = {
                    node_id: node
                    for node_id, node in self.nodes.items()
                    if now - node['last_seen'] < stale_time
                }
                
                # Wait before next cleanup
                await asyncio.sleep(300)  # 5 minutes
                
            except Exception as e:
                self.logger.error(f"Error in cleanup: {str(e)}")
                await asyncio.sleep(5) 