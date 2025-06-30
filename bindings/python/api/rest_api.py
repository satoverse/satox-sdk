"""
REST API client for Satox.
"""

import json
import time
from typing import Dict, Any, Optional
import requests
from requests.exceptions import RequestException

class APIError(Exception):
    """Base class for API errors."""
    def __init__(self, message: str, status_code: Optional[int] = None):
        self.message = message
        self.status_code = status_code
        super().__init__(f"API Error: {message} (Status: {status_code})")

class APIResponse:
    """Represents an API response."""
    def __init__(self, response: requests.Response):
        self.status_code = response.status_code
        self.headers = response.headers
        self.data = response.json() if response.content else None

class RESTAPIClient:
    """REST API client for Satox."""
    
    def __init__(self, base_url: str, api_key: str):
        self.base_url = base_url.rstrip('/')
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

    def _handle_response(self, response: requests.Response) -> APIResponse:
        """Handle API response."""
        if response.status_code >= 400:
            try:
                error_data = response.json()
                error_message = error_data.get('error', 'Unknown error')
            except ValueError:
                error_message = response.text or 'Unknown error'
            raise APIError(error_message, response.status_code)
        return APIResponse(response)

    def get(self, endpoint: str, params: Optional[Dict[str, Any]] = None) -> APIResponse:
        """Make a GET request."""
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        try:
            response = self.session.get(url, params=params)
            return self._handle_response(response)
        except RequestException as e:
            raise APIError(str(e), 503)

    def post(self, endpoint: str, data: Dict[str, Any]) -> APIResponse:
        """Make a POST request."""
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        try:
            response = self.session.post(url, json=data)
            return self._handle_response(response)
        except RequestException as e:
            raise APIError(str(e), 503)

    def put(self, endpoint: str, data: Dict[str, Any]) -> APIResponse:
        """Make a PUT request."""
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        try:
            response = self.session.put(url, json=data)
            return self._handle_response(response)
        except RequestException as e:
            raise APIError(str(e), 503)

    def delete(self, endpoint: str) -> APIResponse:
        """Make a DELETE request."""
        url = f"{self.base_url}/{endpoint.lstrip('/')}"
        try:
            response = self.session.delete(url)
            return self._handle_response(response)
        except RequestException as e:
            raise APIError(str(e), 503)

    def close(self):
        """Close the session."""
        self.session.close() 