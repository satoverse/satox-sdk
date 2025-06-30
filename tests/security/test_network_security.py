import unittest
import os
import tempfile
import ssl
import socket
from unittest.mock import Mock, patch
from satox.network.network_manager import NetworkManager
from satox.core.error_handler import SatoxError
from satox.security.security_manager import SecurityManager

class TestNetworkSecurity(unittest.TestCase):
    """Test cases for network security features."""
    
    def setUp(self):
        """Set up test environment."""
        self.temp_dir = tempfile.mkdtemp()
        self.config = {
            "network": {
                "ssl": {
                    "enabled": True,
                    "cert_file": "test.crt",
                    "key_file": "test.key",
                    "verify_peer": True
                },
                "authentication": {
                    "enabled": True,
                    "token_required": True
                },
                "rate_limiting": {
                    "enabled": True,
                    "max_requests": 100,
                    "window_seconds": 60
                },
                "ddos_protection": {
                    "enabled": True,
                    "max_connections": 1000,
                    "timeout_seconds": 30
                }
            }
        }
        self.security_manager = SecurityManager()
        self.network_manager = NetworkManager(self.config, self.security_manager)
        
    def tearDown(self):
        """Clean up test environment."""
        if os.path.exists("test.crt"):
            os.remove("test.crt")
        if os.path.exists("test.key"):
            os.remove("test.key")
        os.rmdir(self.temp_dir)
        
    def test_ssl_configuration(self):
        """Test SSL configuration and validation."""
        ssl_config = self.network_manager.get_ssl_config()
        self.assertTrue(ssl_config["enabled"])
        self.assertTrue(ssl_config["verify_peer"])
        self.assertEqual(ssl_config["cert_file"], "test.crt")
        self.assertEqual(ssl_config["key_file"], "test.key")
        
    def test_ssl_context_creation(self):
        """Test SSL context creation and configuration."""
        context = self.network_manager.create_ssl_context()
        self.assertIsInstance(context, ssl.SSLContext)
        self.assertEqual(context.verify_mode, ssl.CERT_REQUIRED)
        self.assertEqual(context.protocol, ssl.PROTOCOL_TLS)
        
    def test_authentication_configuration(self):
        """Test authentication configuration."""
        auth_config = self.network_manager.get_authentication_config()
        self.assertTrue(auth_config["enabled"])
        self.assertTrue(auth_config["token_required"])
        
    def test_rate_limiting_configuration(self):
        """Test rate limiting configuration."""
        rate_limit_config = self.network_manager.get_rate_limit_config()
        self.assertTrue(rate_limit_config["enabled"])
        self.assertEqual(rate_limit_config["max_requests"], 100)
        self.assertEqual(rate_limit_config["window_seconds"], 60)
        
    def test_ddos_protection_configuration(self):
        """Test DDoS protection configuration."""
        ddos_config = self.network_manager.get_ddos_protection_config()
        self.assertTrue(ddos_config["enabled"])
        self.assertEqual(ddos_config["max_connections"], 1000)
        self.assertEqual(ddos_config["timeout_seconds"], 30)
        
    def test_connection_validation(self):
        """Test connection validation."""
        mock_socket = Mock(spec=socket.socket)
        mock_socket.getpeername.return_value = ("127.0.0.1", 12345)
        self.assertTrue(self.network_manager.validate_connection(mock_socket))
        
    def test_connection_blacklist(self):
        """Test connection blacklisting."""
        ip = "192.168.1.1"
        self.network_manager.blacklist_ip(ip)
        self.assertTrue(self.network_manager.is_ip_blacklisted(ip))
        
    def test_connection_whitelist(self):
        """Test connection whitelisting."""
        ip = "192.168.1.2"
        self.network_manager.whitelist_ip(ip)
        self.assertTrue(self.network_manager.is_ip_whitelisted(ip))
        
    def test_rate_limit_enforcement(self):
        """Test rate limit enforcement."""
        ip = "192.168.1.3"
        for _ in range(100):
            self.assertTrue(self.network_manager.check_rate_limit(ip))
        self.assertFalse(self.network_manager.check_rate_limit(ip))
        
    def test_ssl_certificate_validation(self):
        """Test SSL certificate validation."""
        with self.assertRaises(SatoxError):
            self.network_manager.validate_ssl_certificate("invalid.crt")
            
    def test_authentication_token_validation(self):
        """Test authentication token validation."""
        token = self.security_manager.generate_session_token()
        self.assertTrue(self.network_manager.validate_auth_token(token))
        self.assertFalse(self.network_manager.validate_auth_token("invalid_token"))
        
    def test_connection_timeout(self):
        """Test connection timeout handling."""
        mock_socket = Mock(spec=socket.socket)
        mock_socket.getpeername.return_value = ("127.0.0.1", 12345)
        self.network_manager.set_connection_timeout(mock_socket, 30)
        mock_socket.settimeout.assert_called_with(30)
        
    def test_network_isolation(self):
        """Test network isolation configuration."""
        isolation_config = self.network_manager.get_network_isolation_config()
        self.assertTrue(isolation_config["enabled"])
        self.assertTrue(isolation_config["restrict_private_ips"])
        
    def test_protocol_security(self):
        """Test protocol security configuration."""
        protocol_config = self.network_manager.get_protocol_security_config()
        self.assertTrue(protocol_config["enabled"])
        self.assertTrue(protocol_config["require_encryption"])
        self.assertTrue(protocol_config["validate_headers"])
        
    def test_connection_pooling(self):
        """Test connection pooling configuration."""
        pool_config = self.network_manager.get_connection_pool_config()
        self.assertTrue(pool_config["enabled"])
        self.assertEqual(pool_config["max_connections"], 100)
        self.assertEqual(pool_config["timeout_seconds"], 30)
        
    def test_network_monitoring(self):
        """Test network monitoring configuration."""
        monitoring_config = self.network_manager.get_network_monitoring_config()
        self.assertTrue(monitoring_config["enabled"])
        self.assertTrue(monitoring_config["log_connections"])
        self.assertTrue(monitoring_config["log_disconnections"])
        
    def test_peer_validation(self):
        """Test peer validation configuration."""
        peer_config = self.network_manager.get_peer_validation_config()
        self.assertTrue(peer_config["enabled"])
        self.assertTrue(peer_config["verify_identity"])
        self.assertTrue(peer_config["check_reputation"])
        
    def test_security_headers(self):
        """Test security headers configuration."""
        headers_config = self.network_manager.get_security_headers_config()
        self.assertTrue(headers_config["enabled"])
        self.assertTrue(headers_config["require_ssl"])
        self.assertTrue(headers_config["prevent_mime_sniffing"])
        
    def test_network_audit(self):
        """Test network audit configuration."""
        audit_config = self.network_manager.get_network_audit_config()
        self.assertTrue(audit_config["enabled"])
        self.assertTrue(audit_config["log_connections"])
        self.assertTrue(audit_config["log_disconnections"])
        self.assertTrue(audit_config["log_errors"])
        
    def test_invalid_ssl_config(self):
        """Test handling of invalid SSL configuration."""
        invalid_config = {
            "network": {
                "ssl": {
                    "enabled": True,
                    "cert_file": "nonexistent.crt",
                    "key_file": "nonexistent.key"
                }
            }
        }
        with self.assertRaises(SatoxError):
            NetworkManager(invalid_config, self.security_manager)
            
    def test_invalid_auth_config(self):
        """Test handling of invalid authentication configuration."""
        invalid_config = {
            "network": {
                "authentication": {
                    "enabled": True,
                    "token_required": True,
                    "invalid_field": "value"
                }
            }
        }
        with self.assertRaises(SatoxError):
            NetworkManager(invalid_config, self.security_manager)
            
    def test_invalid_rate_limit_config(self):
        """Test handling of invalid rate limit configuration."""
        invalid_config = {
            "network": {
                "rate_limiting": {
                    "enabled": True,
                    "max_requests": -1
                }
            }
        }
        with self.assertRaises(SatoxError):
            NetworkManager(invalid_config, self.security_manager)
            
    def test_invalid_ddos_config(self):
        """Test handling of invalid DDoS protection configuration."""
        invalid_config = {
            "network": {
                "ddos_protection": {
                    "enabled": True,
                    "max_connections": 0
                }
            }
        }
        with self.assertRaises(SatoxError):
            NetworkManager(invalid_config, self.security_manager)
            
if __name__ == "__main__":
    unittest.main() 