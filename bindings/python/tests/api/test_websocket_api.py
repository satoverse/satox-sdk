"""Tests for the WebSocket API client."""

import pytest
import json
import asyncio
from unittest.mock import patch, MagicMock
from satox_bindings.api.websocket import WebSocketClient
from satox_bindings.core.error_handling import SatoxError

@pytest.fixture
async def ws_client():
    """Create a WebSocket client instance for testing."""
    client = WebSocketClient("ws://localhost:8000")
    await client.initialize()
    yield client
    await client.shutdown()

@pytest.fixture
def mock_websocket():
    """Create a mock WebSocket connection."""
    ws = MagicMock()
    ws.send = MagicMock()
    ws.receive = MagicMock()
    ws.close = MagicMock()
    return ws

@pytest.mark.asyncio
async def test_initialize(ws_client):
    """Test WebSocket client initialization."""
    assert await ws_client.initialize() is True
    assert ws_client.get_last_error() == ""

@pytest.mark.asyncio
async def test_shutdown(ws_client):
    """Test WebSocket client shutdown."""
    assert await ws_client.shutdown() is True

@pytest.mark.asyncio
async def test_connect(ws_client, mock_websocket):
    """Test WebSocket connection."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        assert await ws_client.connect() is True
        assert ws_client.is_connected() is True

@pytest.mark.asyncio
async def test_disconnect(ws_client, mock_websocket):
    """Test WebSocket disconnection."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        await ws_client.connect()
        await ws_client.disconnect()
        assert ws_client.is_connected() is False
        mock_websocket.close.assert_called_once()

@pytest.mark.asyncio
async def test_send_message(ws_client, mock_websocket):
    """Test sending a message."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        await ws_client.connect()
        message = {"type": "test", "data": "hello"}
        await ws_client.send(message)
        mock_websocket.send.assert_called_once_with(json.dumps(message))

@pytest.mark.asyncio
async def test_receive_message(ws_client, mock_websocket):
    """Test receiving a message."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        mock_websocket.receive.return_value = json.dumps({"type": "test", "data": "hello"})
        await ws_client.connect()
        message = await ws_client.receive()
        assert message == {"type": "test", "data": "hello"}

@pytest.mark.asyncio
async def test_subscribe(ws_client, mock_websocket):
    """Test subscribing to a channel."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        await ws_client.connect()
        await ws_client.subscribe("test_channel")
        mock_websocket.send.assert_called_once_with(
            json.dumps({"type": "subscribe", "channel": "test_channel"})
        )

@pytest.mark.asyncio
async def test_unsubscribe(ws_client, mock_websocket):
    """Test unsubscribing from a channel."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        await ws_client.connect()
        await ws_client.unsubscribe("test_channel")
        mock_websocket.send.assert_called_once_with(
            json.dumps({"type": "unsubscribe", "channel": "test_channel"})
        )

@pytest.mark.asyncio
async def test_connection_error(ws_client):
    """Test connection error handling."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.side_effect = Exception("Connection failed")
        with pytest.raises(SatoxError) as exc_info:
            await ws_client.connect()
        assert "Connection failed" in str(exc_info.value)

@pytest.mark.asyncio
async def test_message_handling(ws_client, mock_websocket):
    """Test message handling with callbacks."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        received_messages = []
        
        async def message_handler(message):
            received_messages.append(message)
        
        ws_client.set_message_handler(message_handler)
        await ws_client.connect()
        
        test_message = {"type": "test", "data": "hello"}
        mock_websocket.receive.return_value = json.dumps(test_message)
        
        await ws_client.receive()
        assert received_messages[0] == test_message

@pytest.mark.asyncio
async def test_reconnection(ws_client, mock_websocket):
    """Test automatic reconnection."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        await ws_client.connect()
        
        # Simulate connection loss
        mock_websocket.receive.side_effect = Exception("Connection lost")
        
        # Should attempt to reconnect
        with pytest.raises(SatoxError):
            await ws_client.receive()
        
        assert mock_connect.call_count > 1

@pytest.mark.asyncio
async def test_heartbeat(ws_client, mock_websocket):
    """Test heartbeat mechanism."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        await ws_client.connect()
        
        # Enable heartbeat
        ws_client.enable_heartbeat(interval=1)
        await asyncio.sleep(1.1)  # Wait for heartbeat
        
        # Should have sent at least one heartbeat
        assert mock_websocket.send.call_count > 0
        last_message = json.loads(mock_websocket.send.call_args[0][0])
        assert last_message["type"] == "heartbeat"

@pytest.mark.asyncio
async def test_binary_messages(ws_client, mock_websocket):
    """Test binary message handling."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        await ws_client.connect()
        
        binary_data = b"test binary data"
        mock_websocket.receive.return_value = binary_data
        
        message = await ws_client.receive()
        assert message == binary_data

@pytest.mark.asyncio
async def test_error_messages(ws_client, mock_websocket):
    """Test error message handling."""
    with patch("websockets.connect") as mock_connect:
        mock_connect.return_value.__aenter__.return_value = mock_websocket
        await ws_client.connect()
        
        error_message = {"type": "error", "code": 400, "message": "Bad Request"}
        mock_websocket.receive.return_value = json.dumps(error_message)
        
        with pytest.raises(SatoxError) as exc_info:
            await ws_client.receive()
        assert "Bad Request" in str(exc_info.value) 