import unittest
from unittest.mock import Mock, patch
from satox.api.rest_api import RESTAPIClient, APIResponse, APIError

class TestRESTAPIClient(unittest.TestCase):
    def setUp(self):
        self.base_url = "http://api.satox.io"
        self.api_key = "test_api_key"
        self.client = RESTAPIClient(self.base_url, self.api_key)

    def test_initialization(self):
        """Test REST API client initialization"""
        self.assertEqual(self.client.base_url, self.base_url)
        self.assertEqual(self.client.api_key, self.api_key)
        self.assertIsNotNone(self.client.session)

    def test_request_headers(self):
        """Test request headers are properly set"""
        headers = self.client._get_headers()
        self.assertIn('Authorization', headers)
        self.assertIn('Content-Type', headers)
        self.assertEqual(headers['Content-Type'], 'application/json')

    @patch('requests.Session.request')
    def test_get_request(self, mock_request):
        """Test GET request handling"""
        mock_response = Mock()
        mock_response.status_code = 200
        mock_response.json.return_value = {'data': 'test'}
        mock_request.return_value = mock_response

        response = self.client.get('/test')
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.data, {'data': 'test'})

    @patch('requests.Session.request')
    def test_post_request(self, mock_request):
        """Test POST request handling"""
        mock_response = Mock()
        mock_response.status_code = 201
        mock_response.json.return_value = {'id': 1}
        mock_request.return_value = mock_response

        data = {'name': 'test'}
        response = self.client.post('/test', data)
        self.assertEqual(response.status_code, 201)
        self.assertEqual(response.data, {'id': 1})

    @patch('requests.Session.request')
    def test_error_handling(self, mock_request):
        """Test error handling"""
        mock_response = Mock()
        mock_response.status_code = 400
        mock_response.json.return_value = {'error': 'Bad Request'}
        mock_request.return_value = mock_response

        with self.assertRaises(APIError) as context:
            self.client.get('/test')
        self.assertEqual(context.exception.status_code, 400)
        self.assertEqual(context.exception.message, 'Bad Request')

    def test_rate_limiting(self):
        """Test rate limiting handling"""
        with patch('time.sleep') as mock_sleep:
            with patch('requests.Session.request') as mock_request:
                mock_response = Mock()
                mock_response.status_code = 429
                mock_response.headers = {'Retry-After': '1'}
                mock_request.return_value = mock_response

                with self.assertRaises(APIError) as context:
                    self.client.get('/test')
                self.assertEqual(context.exception.status_code, 429)
                mock_sleep.assert_called_once_with(1)

    def test_timeout_handling(self):
        """Test timeout handling"""
        with patch('requests.Session.request') as mock_request:
            mock_request.side_effect = TimeoutError()

            with self.assertRaises(APIError) as context:
                self.client.get('/test')
            self.assertEqual(context.exception.status_code, 408)

    def test_connection_error_handling(self):
        """Test connection error handling"""
        with patch('requests.Session.request') as mock_request:
            mock_request.side_effect = ConnectionError()

            with self.assertRaises(APIError) as context:
                self.client.get('/test')
            self.assertEqual(context.exception.status_code, 503)

if __name__ == '__main__':
    unittest.main() 