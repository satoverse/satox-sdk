"""Tests for the REST API client."""

import pytest
import aiohttp
from unittest.mock import patch, MagicMock
from satox_bindings.api.rest import RestApiClient
from satox_bindings.core.error_handling import SatoxError

@pytest.fixture
async def api_client():
    """Create a REST API client instance for testing."""
    client = RestApiClient("http://localhost:8000")
    await client.initialize()
    yield client
    await client.shutdown()

@pytest.fixture
def mock_response():
    """Create a mock response for testing."""
    response = MagicMock()
    response.status = 200
    response.json = MagicMock(return_value={"data": "test"})
    return response

@pytest.mark.asyncio
async def test_initialize(api_client):
    """Test API client initialization."""
    assert await api_client.initialize() is True
    assert api_client.get_last_error() == ""

@pytest.mark.asyncio
async def test_shutdown(api_client):
    """Test API client shutdown."""
    assert await api_client.shutdown() is True

@pytest.mark.asyncio
async def test_get_request(api_client, mock_response):
    """Test GET request."""
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.return_value.__aenter__.return_value = mock_response
        response = await api_client.get("/test")
        assert response == {"data": "test"}

@pytest.mark.asyncio
async def test_post_request(api_client, mock_response):
    """Test POST request."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        response = await api_client.post("/test", {"key": "value"})
        assert response == {"data": "test"}

@pytest.mark.asyncio
async def test_put_request(api_client, mock_response):
    """Test PUT request."""
    with patch("aiohttp.ClientSession.put") as mock_put:
        mock_put.return_value.__aenter__.return_value = mock_response
        response = await api_client.put("/test", {"key": "value"})
        assert response == {"data": "test"}

@pytest.mark.asyncio
async def test_delete_request(api_client, mock_response):
    """Test DELETE request."""
    with patch("aiohttp.ClientSession.delete") as mock_delete:
        mock_delete.return_value.__aenter__.return_value = mock_response
        response = await api_client.delete("/test")
        assert response == {"data": "test"}

@pytest.mark.asyncio
async def test_error_handling(api_client):
    """Test error handling."""
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.side_effect = aiohttp.ClientError("Connection error")
        with pytest.raises(SatoxError) as exc_info:
            await api_client.get("/test")
        assert "Connection error" in str(exc_info.value)

@pytest.mark.asyncio
async def test_timeout_handling(api_client):
    """Test timeout handling."""
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.side_effect = asyncio.TimeoutError()
        with pytest.raises(SatoxError) as exc_info:
            await api_client.get("/test")
        assert "Request timed out" in str(exc_info.value)

@pytest.mark.asyncio
async def test_invalid_response(api_client):
    """Test handling of invalid response."""
    mock_response = MagicMock()
    mock_response.status = 400
    mock_response.json = MagicMock(return_value={"error": "Bad Request"})
    
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.return_value.__aenter__.return_value = mock_response
        with pytest.raises(SatoxError) as exc_info:
            await api_client.get("/test")
        assert "Bad Request" in str(exc_info.value)

@pytest.mark.asyncio
async def test_authentication(api_client, mock_response):
    """Test authentication handling."""
    api_client.set_auth_token("test-token")
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.return_value.__aenter__.return_value = mock_response
        await api_client.get("/test")
        mock_get.assert_called_once()
        headers = mock_get.call_args[1]["headers"]
        assert headers["Authorization"] == "Bearer test-token"

@pytest.mark.asyncio
async def test_rate_limiting(api_client, mock_response):
    """Test rate limiting handling."""
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_response.status = 429
        mock_response.json = MagicMock(return_value={"error": "Too Many Requests"})
        mock_get.return_value.__aenter__.return_value = mock_response
        
        with pytest.raises(SatoxError) as exc_info:
            await api_client.get("/test")
        assert "Too Many Requests" in str(exc_info.value)

@pytest.mark.asyncio
async def test_retry_mechanism(api_client, mock_response):
    """Test retry mechanism."""
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.side_effect = [
            aiohttp.ClientError("Connection error"),
            mock_response
        ]
        response = await api_client.get("/test", retries=1)
        assert response == {"data": "test"}
        assert mock_get.call_count == 2

@pytest.mark.asyncio
async def test_request_timeout(api_client):
    """Test request timeout configuration."""
    api_client.set_timeout(5)
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.side_effect = asyncio.TimeoutError()
        with pytest.raises(SatoxError) as exc_info:
            await api_client.get("/test")
        assert "Request timed out" in str(exc_info.value)

@pytest.mark.asyncio
async def test_custom_headers(api_client, mock_response):
    """Test custom headers handling."""
    custom_headers = {"X-Custom-Header": "test-value"}
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.return_value.__aenter__.return_value = mock_response
        await api_client.get("/test", headers=custom_headers)
        headers = mock_get.call_args[1]["headers"]
        assert headers["X-Custom-Header"] == "test-value"

@pytest.mark.asyncio
async def test_query_parameters(api_client, mock_response):
    """Test query parameters handling."""
    params = {"key": "value", "filter": "test"}
    with patch("aiohttp.ClientSession.get") as mock_get:
        mock_get.return_value.__aenter__.return_value = mock_response
        await api_client.get("/test", params=params)
        called_params = mock_get.call_args[1]["params"]
        assert called_params == params 