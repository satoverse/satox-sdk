"""Main API Client for Satox."""

from typing import Optional
from satox_bindings.core.config_manager import ConfigManager
from satox_bindings.core.error_handling import SatoxError
from .client_factory import APIClientFactory
from .rest_client import RESTClient
from .graphql_client import GraphQLClient
from .websocket_client import WebSocketClient

class APIClient:
    """Main API Client for interacting with Satox services."""

    def __init__(self, config_manager: ConfigManager):
        """Initialize the API client.
        
        Args:
            config_manager: Configuration manager instance
        """
        self.config = config_manager
        self.factory = APIClientFactory(config_manager)
        self._rest_client: Optional[RESTClient] = None
        self._graphql_client: Optional[GraphQLClient] = None
        self._websocket_client: Optional[WebSocketClient] = None

    async def initialize(self) -> bool:
        """Initialize the API client.
        
        Returns:
            bool: True if initialization was successful
        """
        try:
            self._rest_client = await self.factory.get_rest_client()
            self._graphql_client = await self.factory.get_graphql_client()
            self._websocket_client = await self.factory.get_websocket_client()
            return True
        except Exception as e:
            raise SatoxError(f"Failed to initialize API client: {str(e)}")

    async def shutdown(self) -> bool:
        """Shutdown the API client.
        
        Returns:
            bool: True if shutdown was successful
        """
        try:
            return await self.factory.shutdown()
        except Exception as e:
            raise SatoxError(f"Failed to shutdown API client: {str(e)}")

    @property
    def rest(self) -> RESTClient:
        """Get the REST client.
        
        Returns:
            RESTClient: REST client instance
            
        Raises:
            SatoxError: If the REST client is not initialized
        """
        if not self._rest_client:
            raise SatoxError("REST client not initialized")
        return self._rest_client

    @property
    def graphql(self) -> GraphQLClient:
        """Get the GraphQL client.
        
        Returns:
            GraphQLClient: GraphQL client instance
            
        Raises:
            SatoxError: If the GraphQL client is not initialized
        """
        if not self._graphql_client:
            raise SatoxError("GraphQL client not initialized")
        return self._graphql_client

    @property
    def websocket(self) -> WebSocketClient:
        """Get the WebSocket client.
        
        Returns:
            WebSocketClient: WebSocket client instance
            
        Raises:
            SatoxError: If the WebSocket client is not initialized
        """
        if not self._websocket_client:
            raise SatoxError("WebSocket client not initialized")
        return self._websocket_client 