"""
Tests for the security module
"""

import pytest
from satox_bindings.security import SecurityManager

@pytest.fixture
def security_manager():
    """Create security manager instance"""
    manager = SecurityManager()
    manager.initialize()
    return manager

def test_initialize(security_manager):
    """Test security manager initialization"""
    assert security_manager.initialize() is True
    assert security_manager.get_last_error() == ""

def test_encrypt_decrypt(security_manager):
    """Test data encryption and decryption"""
    # Test with string
    original_str = "Test data"
    encrypted = security_manager.encrypt_data(original_str)
    assert encrypted is not None
    decrypted = security_manager.decrypt_data(encrypted)
    assert decrypted.decode() == original_str
    
    # Test with bytes
    original_bytes = b"Test data"
    encrypted = security_manager.encrypt_data(original_bytes)
    assert encrypted is not None
    decrypted = security_manager.decrypt_data(encrypted)
    assert decrypted == original_bytes

def test_password_hashing(security_manager):
    """Test password hashing and verification"""
    password = "test_password"
    
    # Hash password
    result = security_manager.hash_password(password)
    assert result is not None
    assert "hash" in result
    assert "salt" in result
    
    # Verify password
    assert security_manager.verify_password(
        password,
        result["hash"],
        result["salt"]
    ) is True
    
    # Verify wrong password
    assert security_manager.verify_password(
        "wrong_password",
        result["hash"],
        result["salt"]
    ) is False

def test_token_generation(security_manager):
    """Test token generation and verification"""
    data = {"user_id": 1, "timestamp": "2024-03-21"}
    secret = b"test_secret"
    
    # Generate token
    token = security_manager.generate_token(data, secret)
    assert token is not None
    
    # Verify token
    assert security_manager.verify_token(token, data, secret) is True
    
    # Verify with modified data
    modified_data = data.copy()
    modified_data["user_id"] = 2
    assert security_manager.verify_token(token, modified_data, secret) is False

def test_input_validation(security_manager):
    """Test input validation"""
    # Test email validation
    assert security_manager.validate_input(
        "test@example.com",
        r"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$"
    ) is True
    
    assert security_manager.validate_input(
        "invalid_email",
        r"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$"
    ) is False
    
    # Test phone number validation
    assert security_manager.validate_input(
        "+1234567890",
        r"^\+[1-9]\d{1,14}$"
    ) is True
    
    assert security_manager.validate_input(
        "invalid_phone",
        r"^\+[1-9]\d{1,14}$"
    ) is False

def test_input_sanitization(security_manager):
    """Test input sanitization"""
    # Test HTML removal
    input_str = "<script>alert('test')</script>Test"
    sanitized = security_manager.sanitize_input(input_str)
    assert "<script>" not in sanitized
    assert "Test" in sanitized
    
    # Test special character removal
    input_str = "Test!@#$%^&*()"
    sanitized = security_manager.sanitize_input(input_str)
    assert sanitized == "Test"
    
    # Test whitespace normalization
    input_str = "  Test   Data  "
    sanitized = security_manager.sanitize_input(input_str)
    assert sanitized == "Test Data"

def test_error_handling(security_manager):
    """Test error handling"""
    # Test with invalid encryption key
    security_manager._encryption_key = None
    encrypted = security_manager.encrypt_data("test")
    assert encrypted is None
    assert "not initialized" in security_manager.get_last_error()
    
    # Test with invalid token
    assert security_manager.verify_token(
        "invalid_token",
        {"test": "data"},
        b"secret"
    ) is False
    assert "Failed to verify token" in security_manager.get_last_error()

def test_clear_error(security_manager):
    """Test error clearing"""
    security_manager._last_error = "Test error"
    security_manager.clear_last_error()
    assert security_manager.get_last_error() == "" 