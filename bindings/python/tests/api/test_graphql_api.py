"""Tests for the GraphQL API client."""

import pytest
import json
from unittest.mock import patch, MagicMock
from satox_bindings.api.graphql import GraphQLClient
from satox_bindings.core.error_handling import SatoxError

@pytest.fixture
async def graphql_client():
    """Create a GraphQL client instance for testing."""
    client = GraphQLClient("http://localhost:8000/graphql")
    await client.initialize()
    yield client
    await client.shutdown()

@pytest.fixture
def mock_response():
    """Create a mock response for testing."""
    response = MagicMock()
    response.status = 200
    response.json = MagicMock(return_value={
        "data": {"test": "value"},
        "errors": None
    })
    return response

@pytest.mark.asyncio
async def test_initialize(graphql_client):
    """Test GraphQL client initialization."""
    assert await graphql_client.initialize() is True
    assert graphql_client.get_last_error() == ""

@pytest.mark.asyncio
async def test_shutdown(graphql_client):
    """Test GraphQL client shutdown."""
    assert await graphql_client.shutdown() is True

@pytest.mark.asyncio
async def test_query(graphql_client, mock_response):
    """Test GraphQL query."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        query = """
        query {
            test {
                id
                name
            }
        }
        """
        response = await graphql_client.query(query)
        assert response == {"test": "value"}

@pytest.mark.asyncio
async def test_mutation(graphql_client, mock_response):
    """Test GraphQL mutation."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        mutation = """
        mutation {
            createTest(input: {name: "test"}) {
                id
                name
            }
        }
        """
        response = await graphql_client.mutate(mutation)
        assert response == {"test": "value"}

@pytest.mark.asyncio
async def test_subscription(graphql_client):
    """Test GraphQL subscription."""
    subscription = """
    subscription {
        testCreated {
            id
            name
        }
    }
    """
    with patch("websockets.connect") as mock_connect:
        mock_ws = MagicMock()
        mock_connect.return_value.__aenter__.return_value = mock_ws
        mock_ws.receive.return_value = json.dumps({
            "type": "data",
            "payload": {"data": {"testCreated": {"id": "1", "name": "test"}}}
        })
        
        async for message in graphql_client.subscribe(subscription):
            assert message == {"testCreated": {"id": "1", "name": "test"}}
            break

@pytest.mark.asyncio
async def test_error_handling(graphql_client):
    """Test GraphQL error handling."""
    error_response = MagicMock()
    error_response.status = 200
    error_response.json = MagicMock(return_value={
        "data": None,
        "errors": [{"message": "Test error"}]
    })
    
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = error_response
        query = "query { test { id } }"
        
        with pytest.raises(SatoxError) as exc_info:
            await graphql_client.query(query)
        assert "Test error" in str(exc_info.value)

@pytest.mark.asyncio
async def test_variables(graphql_client, mock_response):
    """Test GraphQL variables."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        query = """
        query($id: ID!) {
            test(id: $id) {
                id
                name
            }
        }
        """
        variables = {"id": "123"}
        await graphql_client.query(query, variables)
        
        # Verify variables were sent correctly
        request_data = json.loads(mock_post.call_args[1]["json"])
        assert request_data["variables"] == variables

@pytest.mark.asyncio
async def test_fragments(graphql_client, mock_response):
    """Test GraphQL fragments."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        query = """
        fragment TestFields on Test {
            id
            name
        }
        query {
            test {
                ...TestFields
            }
        }
        """
        response = await graphql_client.query(query)
        assert response == {"test": "value"}

@pytest.mark.asyncio
async def test_authentication(graphql_client, mock_response):
    """Test GraphQL authentication."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        graphql_client.set_auth_token("test-token")
        
        query = "query { test { id } }"
        await graphql_client.query(query)
        
        headers = mock_post.call_args[1]["headers"]
        assert headers["Authorization"] == "Bearer test-token"

@pytest.mark.asyncio
async def test_batch_queries(graphql_client, mock_response):
    """Test batch queries."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        queries = [
            "query { test1 { id } }",
            "query { test2 { id } }"
        ]
        responses = await graphql_client.batch_query(queries)
        assert len(responses) == 2

@pytest.mark.asyncio
async def test_introspection(graphql_client, mock_response):
    """Test schema introspection."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        schema = await graphql_client.introspect()
        assert schema is not None

@pytest.mark.asyncio
async def test_custom_headers(graphql_client, mock_response):
    """Test custom headers."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.return_value.__aenter__.return_value = mock_response
        headers = {"X-Custom-Header": "test-value"}
        
        query = "query { test { id } }"
        await graphql_client.query(query, headers=headers)
        
        request_headers = mock_post.call_args[1]["headers"]
        assert request_headers["X-Custom-Header"] == "test-value"

@pytest.mark.asyncio
async def test_timeout(graphql_client):
    """Test request timeout."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.side_effect = asyncio.TimeoutError()
        
        query = "query { test { id } }"
        with pytest.raises(SatoxError) as exc_info:
            await graphql_client.query(query)
        assert "Request timed out" in str(exc_info.value)

@pytest.mark.asyncio
async def test_retry_mechanism(graphql_client, mock_response):
    """Test retry mechanism."""
    with patch("aiohttp.ClientSession.post") as mock_post:
        mock_post.side_effect = [
            Exception("Connection error"),
            mock_response
        ]
        
        query = "query { test { id } }"
        response = await graphql_client.query(query, retries=1)
        assert response == {"test": "value"}
        assert mock_post.call_count == 2 