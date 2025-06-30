from typing import Dict, List, Optional, Any, Callable
from ..core.satox_manager import SatoxManager
from ..core.satox_error import SatoxError
import asyncio
import json
import logging
from datetime import datetime

class NetworkManager:
    """Manages P2P network connections and communication."""
    
    def __init__(self, satox_manager: SatoxManager):
        """Initialize the network manager.
        
        Args:
            satox_manager: Reference to the SatoxManager instance
        """
        self.satox_manager = satox_manager
        self.nodes: Dict[str, Dict[str, Any]] = {}  # node_id -> node_info
        self.connections: Dict[str, Any] = {}  # node_id -> connection
        self.message_handlers: Dict[str, List[Callable]] = {}  # message_type -> handlers
        self.is_running = False
        self.logger = logging.getLogger(__name__)
        
    async def start(self, host: str, port: int) -> None:
        """Start the network manager.
        
        Args:
            host: Host address to bind to
            port: Port to listen on
            
        Raises:
            SatoxError: If the network manager is already running
        """
        if self.is_running:
            raise SatoxError("Network manager is already running")
            
        try:
            # Start server
            self.server = await asyncio.start_server(
                self._handle_connection,
                host,
                port
            )
            
            self.is_running = True
            self.logger.info(f"Network manager started on {host}:{port}")
            
            # Start node discovery
            asyncio.create_task(self._start_node_discovery())
            
        except Exception as e:
            raise SatoxError(f"Failed to start network manager: {str(e)}")
            
    async def stop(self) -> None:
        """Stop the network manager.
        
        Raises:
            SatoxError: If the network manager is not running
        """
        if not self.is_running:
            raise SatoxError("Network manager is not running")
            
        try:
            # Close all connections
            for conn in self.connections.values():
                conn.close()
                
            # Stop server
            self.server.close()
            await self.server.wait_closed()
            
            self.is_running = False
            self.logger.info("Network manager stopped")
            
        except Exception as e:
            raise SatoxError(f"Failed to stop network manager: {str(e)}")
            
    async def connect_to_node(self, node_id: str, host: str, port: int) -> None:
        """Connect to a remote node.
        
        Args:
            node_id: ID of the node to connect to
            host: Host address of the node
            port: Port of the node
            
        Raises:
            SatoxError: If connection fails
        """
        if node_id in self.connections:
            raise SatoxError(f"Already connected to node {node_id}")
            
        try:
            # Create connection
            reader, writer = await asyncio.open_connection(host, port)
            
            # Store connection
            self.connections[node_id] = {
                'reader': reader,
                'writer': writer,
                'host': host,
                'port': port,
                'last_seen': datetime.now()
            }
            
            # Start message handler
            asyncio.create_task(self._handle_node_messages(node_id))
            
            self.logger.info(f"Connected to node {node_id} at {host}:{port}")
            
        except Exception as e:
            raise SatoxError(f"Failed to connect to node {node_id}: {str(e)}")
            
    async def disconnect_from_node(self, node_id: str) -> None:
        """Disconnect from a remote node.
        
        Args:
            node_id: ID of the node to disconnect from
            
        Raises:
            SatoxError: If node is not connected
        """
        if node_id not in self.connections:
            raise SatoxError(f"Not connected to node {node_id}")
            
        try:
            # Close connection
            conn = self.connections[node_id]
            conn['writer'].close()
            await conn['writer'].wait_closed()
            
            # Remove connection
            del self.connections[node_id]
            
            self.logger.info(f"Disconnected from node {node_id}")
            
        except Exception as e:
            raise SatoxError(f"Failed to disconnect from node {node_id}: {str(e)}")
            
    async def broadcast_message(self, message_type: str, data: Dict[str, Any]) -> None:
        """Broadcast a message to all connected nodes.
        
        Args:
            message_type: Type of message to broadcast
            data: Message data
        """
        message = {
            'type': message_type,
            'data': data,
            'timestamp': datetime.now().isoformat()
        }
        
        for node_id, conn in self.connections.items():
            try:
                await self._send_message(conn, message)
            except Exception as e:
                self.logger.error(f"Failed to broadcast to node {node_id}: {str(e)}")
                
    async def send_message(self, node_id: str, message_type: str, data: Dict[str, Any]) -> None:
        """Send a message to a specific node.
        
        Args:
            node_id: ID of the node to send to
            message_type: Type of message to send
            data: Message data
            
        Raises:
            SatoxError: If node is not connected
        """
        if node_id not in self.connections:
            raise SatoxError(f"Not connected to node {node_id}")
            
        message = {
            'type': message_type,
            'data': data,
            'timestamp': datetime.now().isoformat()
        }
        
        try:
            await self._send_message(self.connections[node_id], message)
        except Exception as e:
            raise SatoxError(f"Failed to send message to node {node_id}: {str(e)}")
            
    def register_message_handler(self, message_type: str, handler: Callable) -> None:
        """Register a handler for a specific message type.
        
        Args:
            message_type: Type of message to handle
            handler: Handler function
        """
        if message_type not in self.message_handlers:
            self.message_handlers[message_type] = []
            
        self.message_handlers[message_type].append(handler)
        
    def unregister_message_handler(self, message_type: str, handler: Callable) -> None:
        """Unregister a handler for a specific message type.
        
        Args:
            message_type: Type of message
            handler: Handler function to remove
        """
        if message_type in self.message_handlers:
            self.message_handlers[message_type].remove(handler)
            
    async def _handle_connection(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
        """Handle incoming connection.
        
        Args:
            reader: Stream reader
            writer: Stream writer
        """
        try:
            # Get peer address
            peer = writer.get_extra_info('peername')
            node_id = f"{peer[0]}:{peer[1]}"
            
            # Store connection
            self.connections[node_id] = {
                'reader': reader,
                'writer': writer,
                'host': peer[0],
                'port': peer[1],
                'last_seen': datetime.now()
            }
            
            # Start message handler
            asyncio.create_task(self._handle_node_messages(node_id))
            
            self.logger.info(f"New connection from {node_id}")
            
        except Exception as e:
            self.logger.error(f"Failed to handle connection: {str(e)}")
            writer.close()
            
    async def _handle_node_messages(self, node_id: str) -> None:
        """Handle messages from a node.
        
        Args:
            node_id: ID of the node
        """
        conn = self.connections[node_id]
        
        try:
            while True:
                # Read message
                data = await conn['reader'].read(1024)
                if not data:
                    break
                    
                # Parse message
                message = json.loads(data.decode())
                
                # Update last seen
                conn['last_seen'] = datetime.now()
                
                # Handle message
                await self._handle_message(node_id, message)
                
        except Exception as e:
            self.logger.error(f"Error handling messages from {node_id}: {str(e)}")
            
        finally:
            # Clean up connection
            await self.disconnect_from_node(node_id)
            
    async def _handle_message(self, node_id: str, message: Dict[str, Any]) -> None:
        """Handle a message from a node.
        
        Args:
            node_id: ID of the node
            message: Message data
        """
        message_type = message.get('type')
        if not message_type:
            return
            
        # Call handlers
        if message_type in self.message_handlers:
            for handler in self.message_handlers[message_type]:
                try:
                    await handler(node_id, message['data'])
                except Exception as e:
                    self.logger.error(f"Error in message handler: {str(e)}")
                    
    async def _send_message(self, conn: Dict[str, Any], message: Dict[str, Any]) -> None:
        """Send a message to a connection.
        
        Args:
            conn: Connection info
            message: Message to send
        """
        try:
            # Serialize message
            data = json.dumps(message).encode()
            
            # Send message
            conn['writer'].write(data)
            await conn['writer'].drain()
            
        except Exception as e:
            raise SatoxError(f"Failed to send message: {str(e)}")
            
    async def _start_node_discovery(self) -> None:
        """Start the node discovery process."""
        while self.is_running:
            try:
                # Broadcast discovery message
                await self.broadcast_message('discovery', {
                    'node_id': self.satox_manager.node_id,
                    'host': self.satox_manager.host,
                    'port': self.satox_manager.port
                })
                
                # Wait before next discovery
                await asyncio.sleep(60)
                
            except Exception as e:
                self.logger.error(f"Error in node discovery: {str(e)}")
                await asyncio.sleep(5) 