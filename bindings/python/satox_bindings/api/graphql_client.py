from typing import Dict, List, Optional, Any, Union
import json
import logging
import requests
from ..core.satox_error import SatoxError

class GraphQLClient:
    """Handles GraphQL queries."""
    
    def __init__(self, url: str, api_key: Optional[str] = None,
                 timeout: int = 30):
        """Initialize the GraphQL client.
        
        Args:
            url: GraphQL endpoint URL
            api_key: API key for authentication
            timeout: Request timeout in seconds
        """
        self.url = url
        self.api_key = api_key
        self.timeout = timeout
        self.session = requests.Session()
        self.logger = logging.getLogger(__name__)
        
        # Set default headers
        self.session.headers.update({
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        })
        
        if api_key:
            self.session.headers.update({
                'Authorization': f'Bearer {api_key}'
            })
            
    def query(self, query: str, variables: Optional[Dict[str, Any]] = None) -> Any:
        """Execute a GraphQL query.
        
        Args:
            query: GraphQL query
            variables: Query variables
            
        Returns:
            Query result
            
        Raises:
            SatoxError: If query fails
        """
        try:
            payload = {
                'query': query,
                'variables': variables or {}
            }
            
            response = self.session.post(
                self.url,
                json=payload,
                timeout=self.timeout
            )
            response.raise_for_status()
            
            result = response.json()
            
            if 'errors' in result:
                raise SatoxError(f"GraphQL query failed: {result['errors']}")
                
            return result.get('data')
            
        except requests.exceptions.RequestException as e:
            raise SatoxError(f"GraphQL query failed: {str(e)}")
            
    def mutation(self, mutation: str, variables: Optional[Dict[str, Any]] = None) -> Any:
        """Execute a GraphQL mutation.
        
        Args:
            mutation: GraphQL mutation
            variables: Mutation variables
            
        Returns:
            Mutation result
            
        Raises:
            SatoxError: If mutation fails
        """
        try:
            payload = {
                'query': mutation,
                'variables': variables or {}
            }
            
            response = self.session.post(
                self.url,
                json=payload,
                timeout=self.timeout
            )
            response.raise_for_status()
            
            result = response.json()
            
            if 'errors' in result:
                raise SatoxError(f"GraphQL mutation failed: {result['errors']}")
                
            return result.get('data')
            
        except requests.exceptions.RequestException as e:
            raise SatoxError(f"GraphQL mutation failed: {str(e)}")
            
    def subscription(self, subscription: str, 
                    variables: Optional[Dict[str, Any]] = None) -> Any:
        """Execute a GraphQL subscription.
        
        Args:
            subscription: GraphQL subscription
            variables: Subscription variables
            
        Returns:
            Subscription result
            
        Raises:
            SatoxError: If subscription fails
        """
        try:
            payload = {
                'query': subscription,
                'variables': variables or {}
            }
            
            response = self.session.post(
                self.url,
                json=payload,
                timeout=self.timeout
            )
            response.raise_for_status()
            
            result = response.json()
            
            if 'errors' in result:
                raise SatoxError(f"GraphQL subscription failed: {result['errors']}")
                
            return result.get('data')
            
        except requests.exceptions.RequestException as e:
            raise SatoxError(f"GraphQL subscription failed: {str(e)}")
            
    def set_api_key(self, api_key: str) -> None:
        """Set API key for authentication.
        
        Args:
            api_key: API key
        """
        self.api_key = api_key
        self.session.headers.update({
            'Authorization': f'Bearer {api_key}'
        })
        
    def set_timeout(self, timeout: int) -> None:
        """Set request timeout.
        
        Args:
            timeout: Timeout in seconds
        """
        self.timeout = timeout
        
    def close(self) -> None:
        """Close the session."""
        self.session.close() 