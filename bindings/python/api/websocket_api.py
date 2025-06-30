"""
WebSocket API client for Satox.
"""

import json
import asyncio
from typing import Dict, Any, Optional, Callable, Awaitable
import websockets
from websockets.exceptions import WebSocketException

class WebSocketError(Exception):
    """Base class for WebSocket errors."""
    def __init__(self, message: str, code: Optional[int] = None):
        self.message = message
        self.code = code
        super().__init__(f"WebSocket Error: {message} (Code: {code})")

class WebSocketMessage:
    """Represents a WebSocket message."""
    def __init__(self, data: Dict[str, Any]):
        self.type = data.get('type')
        self.payload = data.get('payload')
        self.timestamp = data.get('timestamp')

class WebSocketClient:
    """WebSocket client for Satox."""
    
    def __init__(self, url: str, api_key: str):
        self.url = url
        self.api_key = api_key
        self.websocket: Optional[websockets.WebSocketClientProtocol] = None
        self.connected = False
        self._message_handlers: Dict[str, Callable[[WebSocketMessage], Awaitable[None]]] = {}
        self._error_handlers: Dict[str, Callable[[WebSocketError], Awaitable[None]]] = {}

    async def connect(self):
        """Connect to the WebSocket server."""
        try:
            self.websocket = await websockets.connect(
                self.url,
                extra_headers={'Authorization': f'Bearer {self.api_key}'}
            )
            self.connected = True
        except WebSocketException as e:
            raise WebSocketError(str(e), 1006)

    async def disconnect(self):
        """Disconnect from the WebSocket server."""
        if self.websocket:
            await self.websocket.close()
            self.connected = False

    async def send(self, message_type: str, payload: Dict[str, Any]):
        """Send a message."""
        if not self.connected:
            raise WebSocketError("Not connected", 1006)

        message = {
            'type': message_type,
            'payload': payload,
            'timestamp': int(asyncio.get_event_loop().time() * 1000)
        }

        try:
            await self.websocket.send(json.dumps(message))
        except WebSocketException as e:
            raise WebSocketError(str(e), 1006)

    async def receive(self) -> WebSocketMessage:
        """Receive a message."""
        if not self.connected:
            raise WebSocketError("Not connected", 1006)

        try:
            message = await self.websocket.recv()
            data = json.loads(message)
            return WebSocketMessage(data)
        except WebSocketException as e:
            raise WebSocketError(str(e), 1006)
        except json.JSONDecodeError as e:
            raise WebSocketError(f"Invalid message format: {str(e)}", 1007)

    def on_message(self, message_type: str):
        """Decorator for message handlers."""
        def decorator(handler: Callable[[WebSocketMessage], Awaitable[None]]):
            self._message_handlers[message_type] = handler
            return handler
        return decorator

    def on_error(self, error_type: str):
        """Decorator for error handlers."""
        def decorator(handler: Callable[[WebSocketError], Awaitable[None]]):
            self._error_handlers[error_type] = handler
            return handler
        return decorator

    async def _handle_message(self, message: WebSocketMessage):
        """Handle incoming message."""
        handler = self._message_handlers.get(message.type)
        if handler:
            await handler(message)

    async def _handle_error(self, error: WebSocketError):
        """Handle error."""
        handler = self._error_handlers.get(error.__class__.__name__)
        if handler:
            await handler(error)

    async def start_listening(self):
        """Start listening for messages."""
        if not self.connected:
            raise WebSocketError("Not connected", 1006)

        try:
            while self.connected:
                message = await self.receive()
                await self._handle_message(message)
        except WebSocketError as e:
            await self._handle_error(e)
            raise 