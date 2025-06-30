"""
GraphQL API client for Satox.
"""

import json
from typing import Dict, Any, Optional, List
import requests
from requests.exceptions import RequestException

class GraphQLError(Exception):
    """Base class for GraphQL errors."""
    def __init__(self, message: str, locations: Optional[List[Dict[str, int]]] = None, path: Optional[List[str]] = None):
        self.message = message
        self.locations = locations
        self.path = path
        super().__init__(f"GraphQL Error: {message}")

class GraphQLResponse:
    """Represents a GraphQL response."""
    def __init__(self, data: Dict[str, Any], errors: Optional[List[Dict[str, Any]]] = None):
        self.data = data
        self.errors = errors
        if errors:
            raise GraphQLError(
                message=errors[0].get('message', 'Unknown error'),
                locations=errors[0].get('locations'),
                path=errors[0].get('path')
            )

class GraphQLClient:
    """GraphQL client for Satox."""
    
    def __init__(self, endpoint: str, api_key: str):
        self.endpoint = endpoint
        self.api_key = api_key
        self.session = requests.Session()
        self.session.headers.update(self._get_headers())

    def _get_headers(self) -> Dict[str, str]:
        """Get request headers."""
        return {
            'Authorization': f'Bearer {self.api_key}',
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        }

    def execute(self, query: str, variables: Optional[Dict[str, Any]] = None) -> GraphQLResponse:
        """Execute a GraphQL query."""
        payload = {
            'query': query,
            'variables': variables or {}
        }

        try:
            response = self.session.post(self.endpoint, json=payload)
            response.raise_for_status()
            result = response.json()
            return GraphQLResponse(
                data=result.get('data', {}),
                errors=result.get('errors')
            )
        except RequestException as e:
            raise GraphQLError(f"Request failed: {str(e)}")
        except json.JSONDecodeError as e:
            raise GraphQLError(f"Invalid response format: {str(e)}")

    def execute_mutation(self, mutation: str, variables: Optional[Dict[str, Any]] = None) -> GraphQLResponse:
        """Execute a GraphQL mutation."""
        return self.execute(mutation, variables)

    def execute_subscription(self, subscription: str, variables: Optional[Dict[str, Any]] = None) -> GraphQLResponse:
        """Execute a GraphQL subscription."""
        return self.execute(subscription, variables)

    def close(self):
        """Close the session."""
        self.session.close() 