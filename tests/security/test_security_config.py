import pytest
import os
import json
from typing import Dict, Any
from satox.core.satox_manager import SatoxManager, SatoxConfig
from satox.security.security_manager import SecurityManager

class TestSecurityConfig:
    """Security configuration verification tests."""

    def test_environment_variables(self):
        """Test that required environment variables are set and secure."""
        required_vars = [
            "SATOX_API_KEY",
            "SATOX_SECRET_KEY",
            "SATOX_NETWORK_ID"
        ]
        
        for var in required_vars:
            assert var in os.environ, f"Required environment variable {var} is not set"
            value = os.environ[var]
            assert len(value) >= 32, f"Environment variable {var} is too short"
            assert not value.startswith("test_"), f"Environment variable {var} appears to be a test value"

    def test_api_key_security(self, satox_config: SatoxConfig):
        """Test API key security configuration."""
        assert satox_config.api_key is not None, "API key should be configured"
        assert len(satox_config.api_key) >= 32, "API key should be sufficiently long"
        assert not satox_config.api_key.startswith("test_"), "API key should not be a test value"
        
        # Check API key format
        assert all(c in "0123456789abcdef" for c in satox_config.api_key.lower()), \
            "API key should be hexadecimal"

    def test_network_security(self, satox_config: SatoxConfig):
        """Test network security configuration."""
        assert satox_config.network_id in ["mainnet", "testnet"], \
            "Network ID should be either mainnet or testnet"
        
        if satox_config.network_id == "mainnet":
            assert satox_config.api_endpoint.startswith("https://"), \
                "Mainnet API endpoint should use HTTPS"
            assert "localhost" not in satox_config.api_endpoint, \
                "Mainnet should not use localhost"

    def test_encryption_config(self, security_manager: SecurityManager):
        """Test encryption configuration."""
        config = security_manager.get_encryption_config()
        
        assert config["algorithm"] in ["AES-256-GCM", "ChaCha20-Poly1305"], \
            "Should use strong encryption algorithm"
        assert config["key_size"] >= 256, "Key size should be at least 256 bits"
        assert config["iterations"] >= 100000, "Key derivation should use sufficient iterations"

    def test_authentication_config(self, security_manager: SecurityManager):
        """Test authentication configuration."""
        config = security_manager.get_authentication_config()
        
        assert config["jwt_secret"] is not None, "JWT secret should be configured"
        assert len(config["jwt_secret"]) >= 32, "JWT secret should be sufficiently long"
        assert config["token_expiry"] >= 3600, "Token expiry should be at least 1 hour"
        assert config["refresh_token_expiry"] >= 604800, "Refresh token expiry should be at least 1 week"

    def test_rate_limiting(self, security_manager: SecurityManager):
        """Test rate limiting configuration."""
        config = security_manager.get_rate_limit_config()
        
        assert config["requests_per_minute"] <= 60, "Rate limit should be reasonable"
        assert config["burst_limit"] <= 10, "Burst limit should be reasonable"
        assert config["ip_whitelist"] is not None, "IP whitelist should be configured"

    def test_audit_logging(self, security_manager: SecurityManager):
        """Test audit logging configuration."""
        config = security_manager.get_audit_config()
        
        assert config["enabled"] is True, "Audit logging should be enabled"
        assert config["log_level"] in ["INFO", "WARNING", "ERROR"], \
            "Log level should be appropriate"
        assert config["retention_days"] >= 90, "Log retention should be at least 90 days"

    def test_backup_security(self, security_manager: SecurityManager):
        """Test backup security configuration."""
        config = security_manager.get_backup_config()
        
        assert config["encryption_enabled"] is True, "Backup encryption should be enabled"
        assert config["compression_enabled"] is True, "Backup compression should be enabled"
        assert config["retention_days"] >= 30, "Backup retention should be at least 30 days"

    def test_network_security_policy(self, security_manager: SecurityManager):
        """Test network security policy configuration."""
        policy = security_manager.get_network_security_policy()
        
        assert policy["tls_version"] >= "1.2", "Should use TLS 1.2 or higher"
        assert policy["cipher_suites"] is not None, "Cipher suites should be configured"
        assert "RC4" not in policy["cipher_suites"], "Should not use weak ciphers"
        assert policy["certificate_validation"] is True, "Certificate validation should be enabled"

    def test_wallet_security(self, security_manager: SecurityManager):
        """Test wallet security configuration."""
        config = security_manager.get_wallet_security_config()
        
        assert config["encryption_enabled"] is True, "Wallet encryption should be enabled"
        assert config["key_derivation"] in ["PBKDF2", "Argon2"], \
            "Should use strong key derivation"
        assert config["iterations"] >= 100000, "Key derivation should use sufficient iterations"
        assert config["salt_length"] >= 16, "Salt should be sufficiently long" 