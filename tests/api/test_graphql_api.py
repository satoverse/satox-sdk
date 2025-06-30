import unittest
from unittest.mock import Mock, patch
import json
from satox.api.graphql_api import GraphQLClient, GraphQLResponse, GraphQLError

class TestGraphQLClient(unittest.TestCase):
    def setUp(self):
        self.endpoint = "http://api.satox.io/graphql"
        self.api_key = "test_api_key"
        self.client = GraphQLClient(self.endpoint, self.api_key)

    def test_initialization(self):
        """Test GraphQL client initialization"""
        self.assertEqual(self.client.endpoint, self.endpoint)
        self.assertEqual(self.client.api_key, self.api_key)
        self.assertIsNotNone(self.client.session)

    def test_request_headers(self):
        """Test request headers are properly set"""
        headers = self.client._get_headers()
        self.assertIn('Authorization', headers)
        self.assertIn('Content-Type', headers)
        self.assertEqual(headers['Content-Type'], 'application/json')

    @patch('requests.Session.post')
    def test_query_execution(self, mock_post):
        """Test GraphQL query execution"""
        mock_response = Mock()
        mock_response.status_code = 200
        mock_response.json.return_value = {
            'data': {
                'transactions': [
                    {'id': '1', 'amount': 100}
                ]
            }
        }
        mock_post.return_value = mock_response

        query = """
        query {
            transactions {
                id
                amount
            }
        }
        """
        response = self.client.execute(query)
        
        self.assertEqual(response.status_code, 200)
        self.assertEqual(len(response.data['transactions']), 1)
        self.assertEqual(response.data['transactions'][0]['id'], '1')

    @patch('requests.Session.post')
    def test_mutation_execution(self, mock_post):
        """Test GraphQL mutation execution"""
        mock_response = Mock()
        mock_response.status_code = 200
        mock_response.json.return_value = {
            'data': {
                'createTransaction': {
                    'id': '1',
                    'status': 'success'
                }
            }
        }
        mock_post.return_value = mock_response

        mutation = """
        mutation CreateTransaction($input: TransactionInput!) {
            createTransaction(input: $input) {
                id
                status
            }
        }
        """
        variables = {
            'input': {
                'amount': 100,
                'recipient': '0x123'
            }
        }
        response = self.client.execute(mutation, variables)
        
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.data['createTransaction']['id'], '1')
        self.assertEqual(response.data['createTransaction']['status'], 'success')

    @patch('requests.Session.post')
    def test_error_handling(self, mock_post):
        """Test GraphQL error handling"""
        mock_response = Mock()
        mock_response.status_code = 200
        mock_response.json.return_value = {
            'errors': [
                {
                    'message': 'Invalid input',
                    'locations': [{'line': 2, 'column': 3}]
                }
            ]
        }
        mock_post.return_value = mock_response

        query = "query { invalid }"
        with self.assertRaises(GraphQLError) as context:
            self.client.execute(query)
        self.assertEqual(context.exception.message, 'Invalid input')

    @patch('requests.Session.post')
    def test_network_error_handling(self, mock_post):
        """Test network error handling"""
        mock_post.side_effect = Exception("Network error")

        query = "query { transactions { id } }"
        with self.assertRaises(GraphQLError) as context:
            self.client.execute(query)
        self.assertEqual(context.exception.status_code, 503)

    def test_query_validation(self):
        """Test query validation"""
        invalid_query = "invalid query"
        with self.assertRaises(GraphQLError) as context:
            self.client.execute(invalid_query)
        self.assertEqual(context.exception.message, 'Invalid GraphQL query')

    @patch('requests.Session.post')
    def test_subscription_handling(self, mock_post):
        """Test subscription handling"""
        mock_response = Mock()
        mock_response.status_code = 200
        mock_response.json.return_value = {
            'data': {
                'transactionCreated': {
                    'id': '1',
                    'amount': 100
                }
            }
        }
        mock_post.return_value = mock_response

        subscription = """
        subscription {
            transactionCreated {
                id
                amount
            }
        }
        """
        response = self.client.execute(subscription)
        
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.data['transactionCreated']['id'], '1')
        self.assertEqual(response.data['transactionCreated']['amount'], 100)

if __name__ == '__main__':
    unittest.main() 