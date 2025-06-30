from typing import Dict, List, Optional, Any, Union, Callable
import json
import logging
import asyncio
import websockets
from ..core.satox_error import SatoxError

class WebSocketClient:
    """Handles WebSocket connections."""
    
    def __init__(self, url: str, api_key: Optional[str] = None):
        """Initialize the WebSocket client.
        
        Args:
            url: WebSocket URL
            api_key: API key for authentication
        """
        self.url = url
        self.api_key = api_key
        self.websocket = None
        self.connected = False
        self.subscriptions = {}
        self.logger = logging.getLogger(__name__)
        
    async def connect(self) -> None:
        """Connect to WebSocket server.
        
        Raises:
            SatoxError: If connection fails
        """
        try:
            headers = {}
            if self.api_key:
                headers['Authorization'] = f'Bearer {self.api_key}'
                
            self.websocket = await websockets.connect(self.url, extra_headers=headers)
            self.connected = True
            self.logger.info("Connected to WebSocket server")
            
        except Exception as e:
            raise SatoxError(f"Failed to connect to WebSocket server: {str(e)}")
            
    async def disconnect(self) -> None:
        """Disconnect from WebSocket server.
        
        Raises:
            SatoxError: If disconnection fails
        """
        try:
            if self.websocket:
                await self.websocket.close()
                self.websocket = None
                self.connected = False
                self.logger.info("Disconnected from WebSocket server")
                
        except Exception as e:
            raise SatoxError(f"Failed to disconnect from WebSocket server: {str(e)}")
            
    async def send(self, message: Dict[str, Any]) -> None:
        """Send a message.
        
        Args:
            message: Message to send
            
        Raises:
            SatoxError: If sending fails
        """
        if not self.connected:
            raise SatoxError("Not connected to WebSocket server")
            
        try:
            await self.websocket.send(json.dumps(message))
            self.logger.debug(f"Sent message: {message}")
            
        except Exception as e:
            raise SatoxError(f"Failed to send message: {str(e)}")
            
    async def receive(self) -> Dict[str, Any]:
        """Receive a message.
        
        Returns:
            Received message
            
        Raises:
            SatoxError: If receiving fails
        """
        if not self.connected:
            raise SatoxError("Not connected to WebSocket server")
            
        try:
            message = await self.websocket.recv()
            data = json.loads(message)
            self.logger.debug(f"Received message: {data}")
            return data
            
        except Exception as e:
            raise SatoxError(f"Failed to receive message: {str(e)}")
            
    async def subscribe(self, channel: str, callback: Callable[[Dict[str, Any]], None]) -> None:
        """Subscribe to a channel.
        
        Args:
            channel: Channel name
            callback: Callback function
            
        Raises:
            SatoxError: If subscription fails
        """
        try:
            await self.send({
                'type': 'subscribe',
                'channel': channel
            })
            
            self.subscriptions[channel] = callback
            self.logger.info(f"Subscribed to channel: {channel}")
            
        except Exception as e:
            raise SatoxError(f"Failed to subscribe to channel: {str(e)}")
            
    async def unsubscribe(self, channel: str) -> None:
        """Unsubscribe from a channel.
        
        Args:
            channel: Channel name
            
        Raises:
            SatoxError: If unsubscription fails
        """
        try:
            await self.send({
                'type': 'unsubscribe',
                'channel': channel
            })
            
            if channel in self.subscriptions:
                del self.subscriptions[channel]
                self.logger.info(f"Unsubscribed from channel: {channel}")
                
        except Exception as e:
            raise SatoxError(f"Failed to unsubscribe from channel: {str(e)}")
            
    async def listen(self) -> None:
        """Listen for messages.
        
        Raises:
            SatoxError: If listening fails
        """
        if not self.connected:
            raise SatoxError("Not connected to WebSocket server")
            
        try:
            while self.connected:
                message = await self.receive()
                
                if 'channel' in message and message['channel'] in self.subscriptions:
                    callback = self.subscriptions[message['channel']]
                    await callback(message)
                    
        except Exception as e:
            raise SatoxError(f"Failed to listen for messages: {str(e)}")
            
    def set_api_key(self, api_key: str) -> None:
        """Set API key for authentication.
        
        Args:
            api_key: API key
        """
        self.api_key = api_key
        
    def is_connected(self) -> bool:
        """Check if connected to WebSocket server.
        
        Returns:
            True if connected
        """
        return self.connected 