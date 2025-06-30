"""Tests for the network manager module."""

import pytest
import aiohttp
from unittest.mock import patch, MagicMock
from satox_bindings.core import NetworkManager

@pytest.fixture
async def network_manager():
    """Create a network manager instance for testing."""
    manager = NetworkManager()
    await manager.initialize()
    yield manager
    await manager.shutdown()

@pytest.fixture
def mock_response():
    """Create a mock response for testing."""
    response = MagicMock()
    response.status = 200
    return response

@pytest.mark.asyncio
async def test_initialize(network_manager):
    """Test network manager initialization."""
    assert await network_manager.initialize() is True
    assert network_manager.get_last_error() == ""

@pytest.mark.asyncio
async def test_shutdown(network_manager):
    """Test network manager shutdown."""
    assert await network_manager.shutdown() is True
    assert network_manager.get_peers() == []

@pytest.mark.asyncio
async def test_add_peer(network_manager):
    """Test adding a peer."""
    assert await network_manager.add_peer("peer1", "http://localhost:8000") is True
    peer = network_manager.get_peer("peer1")
    assert peer is not None
    assert peer["address"] == "http://localhost:8000"
    assert peer["connected"] is False

@pytest.mark.asyncio
async def test_remove_peer(network_manager):
    """Test removing a peer."""
    await network_manager.add_peer("peer1", "http://localhost:8000")
    assert await network_manager.remove_peer("peer1") is True
    assert network_manager.get_peer("peer1") is None

@pytest.mark.asyncio
async def test_connect_to_peer(network_manager, mock_response):
    """Test connecting to a peer."""
    await network_manager.add_peer("peer1", "http://localhost:8000")
    
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.return_value.__aenter__.return_value = mock_response
        assert await network_manager.connect_to_peer("peer1") is True
        peer = network_manager.get_peer("peer1")
        assert peer is not None
        assert peer["connected"] is True

@pytest.mark.asyncio
async def test_send_message(network_manager, mock_response):
    """Test sending a message to a peer."""
    await network_manager.add_peer("peer1", "http://localhost:8000")
    await network_manager.connect_to_peer("peer1")
    
    message = {"type": "test", "data": "hello"}
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        assert await network_manager.send_message("peer1", message) is True

@pytest.mark.asyncio
async def test_get_peers(network_manager):
    """Test getting all peers."""
    await network_manager.add_peer("peer1", "http://localhost:8000")
    await network_manager.add_peer("peer2", "http://localhost:8001")
    
    peers = network_manager.get_peers()
    assert len(peers) == 2
    assert any(p["id"] == "peer1" for p in peers)
    assert any(p["id"] == "peer2" for p in peers)

@pytest.mark.asyncio
async def test_get_peer(network_manager):
    """Test getting a specific peer."""
    await network_manager.add_peer("peer1", "http://localhost:8000")
    peer = network_manager.get_peer("peer1")
    assert peer is not None
    assert peer["id"] == "peer1"
    assert peer["address"] == "http://localhost:8000"

@pytest.mark.asyncio
async def test_duplicate_peer(network_manager):
    """Test adding a duplicate peer."""
    await network_manager.add_peer("peer1", "http://localhost:8000")
    assert await network_manager.add_peer("peer1", "http://localhost:8001") is False
    assert "already exists" in network_manager.get_last_error()

@pytest.mark.asyncio
async def test_connect_nonexistent_peer(network_manager):
    """Test connecting to a non-existent peer."""
    assert await network_manager.connect_to_peer("nonexistent") is False
    assert "not found" in network_manager.get_last_error()

@pytest.mark.asyncio
async def test_send_message_nonexistent_peer(network_manager):
    """Test sending a message to a non-existent peer."""
    message = {"type": "test", "data": "hello"}
    assert await network_manager.send_message("nonexistent", message) is False
    assert "not found" in network_manager.get_last_error()

@pytest.mark.asyncio
async def test_send_message_unconnected_peer(network_manager):
    """Test sending a message to an unconnected peer."""
    await network_manager.add_peer("peer1", "http://localhost:8000")
    message = {"type": "test", "data": "hello"}
    assert await network_manager.send_message("peer1", message) is False
    assert "not connected" in network_manager.get_last_error()

@pytest.mark.asyncio
async def test_error_handling(network_manager):
    """Test error handling."""
    await network_manager.shutdown()
    assert await network_manager.add_peer("peer1", "http://localhost:8000") is False
    assert "not initialized" in network_manager.get_last_error()

@pytest.mark.asyncio
async def test_clear_error(network_manager):
    """Test clearing error message."""
    await network_manager.shutdown()
    await network_manager.add_peer("peer1", "http://localhost:8000")
    assert network_manager.get_last_error() != ""
    network_manager.clear_last_error()
    assert network_manager.get_last_error() == "" 