"""API Client Factory for Satox."""

from typing import Optional, Type
from satox_bindings.core.config_manager import ConfigManager
from satox_bindings.core.error_handling import SatoxError
from .rest_client import RESTClient
from .graphql_client import GraphQLClient
from .websocket_client import WebSocketClient

class APIClientFactory:
    """Factory for creating API clients."""

    def __init__(self, config_manager: ConfigManager):
        """Initialize the API client factory.
        
        Args:
            config_manager: Configuration manager instance
        """
        self.config = config_manager
        self._rest_client: Optional[RESTClient] = None
        self._graphql_client: Optional[GraphQLClient] = None
        self._websocket_client: Optional[WebSocketClient] = None

    async def get_rest_client(self) -> RESTClient:
        """Get or create a REST client.
        
        Returns:
            RESTClient: REST client instance
        """
        if not self._rest_client:
            self._rest_client = RESTClient(self.config)
            await self._rest_client.initialize()
        return self._rest_client

    async def get_graphql_client(self) -> GraphQLClient:
        """Get or create a GraphQL client.
        
        Returns:
            GraphQLClient: GraphQL client instance
        """
        if not self._graphql_client:
            self._graphql_client = GraphQLClient(self.config)
            await self._graphql_client.initialize()
        return self._graphql_client

    async def get_websocket_client(self) -> WebSocketClient:
        """Get or create a WebSocket client.
        
        Returns:
            WebSocketClient: WebSocket client instance
        """
        if not self._websocket_client:
            self._websocket_client = WebSocketClient(self.config)
            await self._websocket_client.initialize()
        return self._websocket_client

    async def shutdown(self) -> bool:
        """Shutdown all API clients.
        
        Returns:
            bool: True if shutdown was successful
        """
        success = True
        if self._rest_client:
            try:
                await self._rest_client.shutdown()
            except Exception as e:
                success = False
                raise SatoxError(f"Failed to shutdown REST client: {str(e)}")

        if self._graphql_client:
            try:
                await self._graphql_client.shutdown()
            except Exception as e:
                success = False
                raise SatoxError(f"Failed to shutdown GraphQL client: {str(e)}")

        if self._websocket_client:
            try:
                await self._websocket_client.shutdown()
            except Exception as e:
                success = False
                raise SatoxError(f"Failed to shutdown WebSocket client: {str(e)}")

        return success 