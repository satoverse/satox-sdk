"""WebSocket API Client for Satox."""

import asyncio
import json
import logging
from typing import Any, Callable, Dict, List, Optional, Union
from satox_bindings.core.error_handling import SatoxError
from satox_bindings.core.config_manager import ConfigManager

class WebSocketClient:
    """WebSocket API Client for interacting with Satox services."""

    def __init__(self, config_manager: ConfigManager):
        """Initialize the WebSocket client.
        
        Args:
            config_manager: Configuration manager instance
        """
        self.config = config_manager
        self.ws: Optional[websockets.WebSocketClientProtocol] = None
        self.base_url = self.config.get("api.websocket.base_url")
        self.timeout = self.config.get("api.websocket.timeout", 30)
        self.max_retries = self.config.get("api.websocket.max_retries", 3)
        self.retry_delay = self.config.get("api.websocket.retry_delay", 1)
        self.heartbeat_interval = self.config.get("api.websocket.heartbeat_interval", 30)
        self.subscriptions: Dict[str, List[Callable]] = {}
        self.is_connected = False
        self.logger = logging.getLogger(__name__)

    async def initialize(self) -> bool:
        """Initialize the WebSocket client.
        
        Returns:
            bool: True if initialization was successful
        """
        try:
            await self.connect()
            asyncio.create_task(self._heartbeat())
            return True
        except Exception as e:
            raise SatoxError(f"Failed to initialize WebSocket client: {str(e)}")

    async def shutdown(self) -> bool:
        """Shutdown the WebSocket client.
        
        Returns:
            bool: True if shutdown was successful
        """
        try:
            if self.ws:
                await self.ws.close()
            self.is_connected = False
            return True
        except Exception as e:
            raise SatoxError(f"Failed to shutdown WebSocket client: {str(e)}")

    async def connect(self) -> None:
        """Connect to the WebSocket server."""
        try:
            self.ws = await websockets.connect(
                self.base_url,
                ping_interval=None,  # We handle heartbeats manually
                close_timeout=self.timeout
            )
            self.is_connected = True
            asyncio.create_task(self._receive_messages())
        except Exception as e:
            raise SatoxError(f"Failed to connect to WebSocket server: {str(e)}")

    async def _heartbeat(self) -> None:
        """Send periodic heartbeat messages."""
        while self.is_connected:
            try:
                await self.send({"type": "heartbeat"})
                await asyncio.sleep(self.heartbeat_interval)
            except Exception as e:
                self.logger.error(f"Heartbeat failed: {str(e)}")
                await self._reconnect()

    async def _reconnect(self) -> None:
        """Reconnect to the WebSocket server."""
        retry_count = 0
        while retry_count < self.max_retries:
            try:
                await self.connect()
                # Resubscribe to all topics
                for topic in self.subscriptions.keys():
                    await self.subscribe(topic)
                return
            except Exception as e:
                retry_count += 1
                if retry_count == self.max_retries:
                    raise SatoxError(f"Failed to reconnect after {self.max_retries} attempts: {str(e)}")
                await asyncio.sleep(self.retry_delay * (2 ** retry_count))

    async def _receive_messages(self) -> None:
        """Receive and process incoming messages."""
        while self.is_connected:
            try:
                message = await self.ws.recv()
                data = json.loads(message)
                
                # Handle heartbeat response
                if data.get("type") == "heartbeat":
                    continue
                
                # Handle subscription messages
                topic = data.get("topic")
                if topic and topic in self.subscriptions:
                    for callback in self.subscriptions[topic]:
                        try:
                            await callback(data)
                        except Exception as e:
                            self.logger.error(f"Error in subscription callback: {str(e)}")
                
            except websockets.exceptions.ConnectionClosed:
                self.is_connected = False
                await self._reconnect()
            except Exception as e:
                self.logger.error(f"Error receiving message: {str(e)}")

    async def send(self, data: Dict[str, Any]) -> None:
        """Send a message to the WebSocket server.
        
        Args:
            data: Message data to send
        """
        if not self.is_connected:
            raise SatoxError("WebSocket client not connected")
        
        try:
            await self.ws.send(json.dumps(data))
        except Exception as e:
            raise SatoxError(f"Failed to send message: {str(e)}")

    async def subscribe(self, topic: str, callback: Callable) -> None:
        """Subscribe to a topic.
        
        Args:
            topic: Topic to subscribe to
            callback: Callback function to handle messages
        """
        if topic not in self.subscriptions:
            self.subscriptions[topic] = []
            await self.send({
                "type": "subscribe",
                "topic": topic
            })
        self.subscriptions[topic].append(callback)

    async def unsubscribe(self, topic: str, callback: Optional[Callable] = None) -> None:
        """Unsubscribe from a topic.
        
        Args:
            topic: Topic to unsubscribe from
            callback: Specific callback to remove (if None, remove all callbacks)
        """
        if topic in self.subscriptions:
            if callback:
                self.subscriptions[topic].remove(callback)
                if not self.subscriptions[topic]:
                    del self.subscriptions[topic]
                    await self.send({
                        "type": "unsubscribe",
                        "topic": topic
                    })
            else:
                del self.subscriptions[topic]
                await self.send({
                    "type": "unsubscribe",
                    "topic": topic
                })

    async def subscribe_to_blocks(self, callback: Callable) -> None:
        """Subscribe to new blocks.
        
        Args:
            callback: Callback function to handle block messages
        """
        await self.subscribe("blocks", callback)

    async def subscribe_to_transactions(self, callback: Callable) -> None:
        """Subscribe to new transactions.
        
        Args:
            callback: Callback function to handle transaction messages
        """
        await self.subscribe("transactions", callback)

    async def subscribe_to_assets(self, callback: Callable) -> None:
        """Subscribe to asset updates.
        
        Args:
            callback: Callback function to handle asset messages
        """
        await self.subscribe("assets", callback)

    async def subscribe_to_nfts(self, callback: Callable) -> None:
        """Subscribe to NFT updates.
        
        Args:
            callback: Callback function to handle NFT messages
        """
        await self.subscribe("nfts", callback)

    async def subscribe_to_wallets(self, callback: Callable) -> None:
        """Subscribe to wallet updates.
        
        Args:
            callback: Callback function to handle wallet messages
        """
        await self.subscribe("wallets", callback)

    async def subscribe_to_network(self, callback: Callable) -> None:
        """Subscribe to network status updates.
        
        Args:
            callback: Callback function to handle network messages
        """
        await self.subscribe("network", callback)

    async def subscribe_to_node(self, callback: Callable) -> None:
        """Subscribe to node status updates.
        
        Args:
            callback: Callback function to handle node messages
        """
        await self.subscribe("node", callback)

    async def subscribe_to_peers(self, callback: Callable) -> None:
        """Subscribe to peer updates.
        
        Args:
            callback: Callback function to handle peer messages
        """
        await self.subscribe("peers", callback)

    async def subscribe_to_sync(self, callback: Callable) -> None:
        """Subscribe to sync status updates.
        
        Args:
            callback: Callback function to handle sync messages
        """
        await self.subscribe("sync", callback) 