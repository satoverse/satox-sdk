"""
Network Manager Python Bindings
"""

from typing import Dict, List, Optional, Any, Callable
import satox_cpp

class NetworkManager:
    """Python wrapper for the C++ NetworkManager class."""
    
    def __init__(self):
        """Initialize the Network Manager."""
        self._manager = satox_cpp.NetworkManager()
        self._initialized = False
        self._callbacks = {}
    
    def initialize(self) -> bool:
        """Initialize the network manager.
        
        Returns:
            bool: True if initialization was successful, False otherwise.
        """
        if not self._initialized:
            self._initialized = self._manager.initialize()
        return self._initialized
    
    def shutdown(self) -> bool:
        """Shutdown the network manager.
        
        Returns:
            bool: True if shutdown was successful, False otherwise.
        """
        if self._initialized:
            self._initialized = not self._manager.shutdown()
        return not self._initialized
    
    def connect(self, host: str, port: int) -> bool:
        """Connect to a network node.
        
        Args:
            host (str): The host address
            port (int): The port number
            
        Returns:
            bool: True if connection was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        if not host or port <= 0:
            raise ValueError("Invalid host or port")
        
        return self._manager.connect(host, port)
    
    def disconnect(self, host: str, port: int) -> bool:
        """Disconnect from a network node.
        
        Args:
            host (str): The host address
            port (int): The port number
            
        Returns:
            bool: True if disconnection was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        if not host or port <= 0:
            raise ValueError("Invalid host or port")
        
        return self._manager.disconnect(host, port)
    
    def send_message(self, host: str, port: int, message: bytes) -> bool:
        """Send a message to a network node.
        
        Args:
            host (str): The host address
            port (int): The port number
            message (bytes): The message to send
            
        Returns:
            bool: True if sending was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        if not host or port <= 0 or not message:
            raise ValueError("Invalid input")
        
        return self._manager.send_message(host, port, message)
    
    def broadcast_message(self, message: bytes) -> bool:
        """Broadcast a message to all connected nodes.
        
        Args:
            message (bytes): The message to broadcast
            
        Returns:
            bool: True if broadcasting was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        if not message:
            raise ValueError("Invalid message")
        
        return self._manager.broadcast_message(message)
    
    def register_message_handler(self, message_type: str, handler: Callable[[bytes], None]) -> bool:
        """Register a handler for a specific message type.
        
        Args:
            message_type (str): The type of message to handle
            handler (Callable[[bytes], None]): The handler function
            
        Returns:
            bool: True if registration was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        if not message_type or not handler:
            raise ValueError("Invalid input")
        
        self._callbacks[message_type] = handler
        return self._manager.register_message_handler(message_type, self._handle_message)
    
    def unregister_message_handler(self, message_type: str) -> bool:
        """Unregister a message handler.
        
        Args:
            message_type (str): The type of message to unregister
            
        Returns:
            bool: True if unregistration was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        if not message_type:
            raise ValueError("Invalid message type")
        
        if message_type in self._callbacks:
            del self._callbacks[message_type]
        return self._manager.unregister_message_handler(message_type)
    
    def _handle_message(self, message_type: str, message: bytes) -> None:
        """Internal message handler that calls the registered Python callback.
        
        Args:
            message_type (str): The type of message
            message (bytes): The message data
        """
        if message_type in self._callbacks:
            self._callbacks[message_type](message)
    
    def get_connected_nodes(self) -> List[Dict[str, Any]]:
        """Get information about connected nodes.
        
        Returns:
            List[Dict[str, Any]]: List of node information
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        return self._manager.get_connected_nodes()
    
    def get_node_info(self, host: str, port: int) -> Optional[Dict[str, Any]]:
        """Get information about a specific node.
        
        Args:
            host (str): The host address
            port (int): The port number
            
        Returns:
            Optional[Dict[str, Any]]: Node information, or None if not found
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        if not host or port <= 0:
            raise ValueError("Invalid host or port")
        
        return self._manager.get_node_info(host, port)
    
    def is_connected(self, host: str, port: int) -> bool:
        """Check if connected to a specific node.
        
        Args:
            host (str): The host address
            port (int): The port number
            
        Returns:
            bool: True if connected, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        if not host or port <= 0:
            raise ValueError("Invalid host or port")
        
        return self._manager.is_connected(host, port)
    
    def get_connection_count(self) -> int:
        """Get the number of active connections.
        
        Returns:
            int: The number of connections
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        return self._manager.get_connection_count()
    
    def is_initialized(self) -> bool:
        """Check if the network manager is initialized.
        
        Returns:
            bool: True if initialized, False otherwise.
        """
        return self._initialized
    
    def get_version(self) -> str:
        """Get the version of the network manager.
        
        Returns:
            str: The version string
        """
        return self._manager.get_version()
    
    def get_network_type(self) -> str:
        """Get the network type.
        
        Returns:
            str: The network type
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        return self._manager.get_network_type()
    
    def get_network_status(self) -> Dict[str, Any]:
        """Get the current network status.
        
        Returns:
            Dict[str, Any]: Network status information
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Network Manager not initialized")
        
        return self._manager.get_network_status() 