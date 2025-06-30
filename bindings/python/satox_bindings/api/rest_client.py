from typing import Dict, List, Optional, Any, Union
import json
import logging
import requests
from urllib.parse import urljoin
from ..core.satox_error import SatoxError

class RESTClient:
    """Handles REST API interactions."""
    
    def __init__(self, base_url: str, api_key: Optional[str] = None,
                 timeout: int = 30):
        """Initialize the REST client.
        
        Args:
            base_url: Base URL for API
            api_key: API key for authentication
            timeout: Request timeout in seconds
        """
        self.base_url = base_url.rstrip('/')
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
            
    def get(self, endpoint: str, params: Optional[Dict[str, Any]] = None) -> Any:
        """Send a GET request.
        
        Args:
            endpoint: API endpoint
            params: Query parameters
            
        Returns:
            Response data
            
        Raises:
            SatoxError: If request fails
        """
        try:
            url = urljoin(self.base_url, endpoint)
            response = self.session.get(url, params=params, timeout=self.timeout)
            response.raise_for_status()
            return response.json()
            
        except requests.exceptions.RequestException as e:
            raise SatoxError(f"GET request failed: {str(e)}")
            
    def post(self, endpoint: str, data: Optional[Dict[str, Any]] = None) -> Any:
        """Send a POST request.
        
        Args:
            endpoint: API endpoint
            data: Request data
            
        Returns:
            Response data
            
        Raises:
            SatoxError: If request fails
        """
        try:
            url = urljoin(self.base_url, endpoint)
            response = self.session.post(url, json=data, timeout=self.timeout)
            response.raise_for_status()
            return response.json()
            
        except requests.exceptions.RequestException as e:
            raise SatoxError(f"POST request failed: {str(e)}")
            
    def put(self, endpoint: str, data: Optional[Dict[str, Any]] = None) -> Any:
        """Send a PUT request.
        
        Args:
            endpoint: API endpoint
            data: Request data
            
        Returns:
            Response data
            
        Raises:
            SatoxError: If request fails
        """
        try:
            url = urljoin(self.base_url, endpoint)
            response = self.session.put(url, json=data, timeout=self.timeout)
            response.raise_for_status()
            return response.json()
            
        except requests.exceptions.RequestException as e:
            raise SatoxError(f"PUT request failed: {str(e)}")
            
    def delete(self, endpoint: str) -> Any:
        """Send a DELETE request.
        
        Args:
            endpoint: API endpoint
            
        Returns:
            Response data
            
        Raises:
            SatoxError: If request fails
        """
        try:
            url = urljoin(self.base_url, endpoint)
            response = self.session.delete(url, timeout=self.timeout)
            response.raise_for_status()
            return response.json()
            
        except requests.exceptions.RequestException as e:
            raise SatoxError(f"DELETE request failed: {str(e)}")
            
    def patch(self, endpoint: str, data: Optional[Dict[str, Any]] = None) -> Any:
        """Send a PATCH request.
        
        Args:
            endpoint: API endpoint
            data: Request data
            
        Returns:
            Response data
            
        Raises:
            SatoxError: If request fails
        """
        try:
            url = urljoin(self.base_url, endpoint)
            response = self.session.patch(url, json=data, timeout=self.timeout)
            response.raise_for_status()
            return response.json()
            
        except requests.exceptions.RequestException as e:
            raise SatoxError(f"PATCH request failed: {str(e)}")
            
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