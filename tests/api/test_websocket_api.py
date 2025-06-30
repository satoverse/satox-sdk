import unittest
from unittest.mock import Mock, patch, AsyncMock
import asyncio
import json
from satox.api.websocket_api import WebSocketClient, WebSocketMessage, WebSocketError

class TestWebSocketClient(unittest.TestCase):
    def setUp(self):
        self.url = "ws://api.satox.io/ws"
        self.api_key = "test_api_key"
        self.client = WebSocketClient(self.url, self.api_key)

    @patch('websockets.connect')
    async def test_connection(self, mock_connect):
        """Test WebSocket connection"""
        mock_ws = AsyncMock()
        mock_connect.return_value = mock_ws

        await self.client.connect()
        self.assertTrue(self.client.is_connected())
        mock_connect.assert_called_once()

    @patch('websockets.connect')
    async def test_authentication(self, mock_connect):
        """Test WebSocket authentication"""
        mock_ws = AsyncMock()
        mock_connect.return_value = mock_ws

        await self.client.connect()
        mock_ws.send.assert_called_once()
        auth_message = json.loads(mock_ws.send.call_args[0][0])
        self.assertEqual(auth_message['type'], 'auth')
        self.assertEqual(auth_message['api_key'], self.api_key)

    @patch('websockets.connect')
    async def test_subscribe(self, mock_connect):
        """Test subscription to channels"""
        mock_ws = AsyncMock()
        mock_connect.return_value = mock_ws

        await self.client.connect()
        await self.client.subscribe('transactions')
        
        mock_ws.send.assert_called()
        subscribe_message = json.loads(mock_ws.send.call_args[0][0])
        self.assertEqual(subscribe_message['type'], 'subscribe')
        self.assertEqual(subscribe_message['channel'], 'transactions')

    @patch('websockets.connect')
    async def test_message_handling(self, mock_connect):
        """Test message handling"""
        mock_ws = AsyncMock()
        mock_connect.return_value = mock_ws

        test_message = {
            'type': 'transaction',
            'data': {'id': '123', 'amount': 100}
        }
        mock_ws.recv.return_value = json.dumps(test_message)

        await self.client.connect()
        message = await self.client.receive()
        
        self.assertEqual(message.type, 'transaction')
        self.assertEqual(message.data['id'], '123')
        self.assertEqual(message.data['amount'], 100)

    @patch('websockets.connect')
    async def test_error_handling(self, mock_connect):
        """Test error handling"""
        mock_ws = AsyncMock()
        mock_connect.return_value = mock_ws
        mock_ws.recv.side_effect = Exception("Connection error")

        await self.client.connect()
        with self.assertRaises(WebSocketError):
            await self.client.receive()

    @patch('websockets.connect')
    async def test_reconnection(self, mock_connect):
        """Test automatic reconnection"""
        mock_ws = AsyncMock()
        mock_connect.return_value = mock_ws
        mock_ws.recv.side_effect = [Exception("Connection lost"), "{}"]

        await self.client.connect()
        await self.client.receive()  # Should trigger reconnection
        self.assertTrue(self.client.is_connected())

    @patch('websockets.connect')
    async def test_heartbeat(self, mock_connect):
        """Test heartbeat mechanism"""
        mock_ws = AsyncMock()
        mock_connect.return_value = mock_ws

        await self.client.connect()
        await asyncio.sleep(0.1)  # Allow time for heartbeat
        
        mock_ws.send.assert_called()
        heartbeat_message = json.loads(mock_ws.send.call_args[0][0])
        self.assertEqual(heartbeat_message['type'], 'ping')

    @patch('websockets.connect')
    async def test_close(self, mock_connect):
        """Test connection closure"""
        mock_ws = AsyncMock()
        mock_connect.return_value = mock_ws

        await self.client.connect()
        await self.client.close()
        
        self.assertFalse(self.client.is_connected())
        mock_ws.close.assert_called_once()

if __name__ == '__main__':
    unittest.main() 