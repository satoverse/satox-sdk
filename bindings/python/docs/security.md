# Security Module Documentation

The security module provides comprehensive security features for data encryption, password hashing, token generation, and input validation.

## Installation

```bash
pip install bindings
```

## Basic Usage

### Initialization

```python
from satox_bindings.security import SecurityManager

# Create and initialize security manager
security_manager = SecurityManager()
security_manager.initialize()

# Initialize with custom encryption key
encryption_key = b"your-secret-key"
security_manager.initialize(encryption_key)
```

### Data Encryption

```python
# Encrypt data
data = "Sensitive information"
encrypted = security_manager.encrypt_data(data)
print(f"Encrypted: {encrypted}")

# Decrypt data
decrypted = security_manager.decrypt_data(encrypted)
print(f"Decrypted: {decrypted.decode()}")
```

### Password Hashing

```python
# Hash password
password = "user_password"
result = security_manager.hash_password(password)
print(f"Hash: {result['hash']}")
print(f"Salt: {result['salt']}")

# Verify password
is_valid = security_manager.verify_password(
    password,
    result["hash"],
    result["salt"]
)
print(f"Password valid: {is_valid}")
```

### Token Generation

```python
# Generate token
data = {
    "user_id": 1,
    "timestamp": "2024-03-21"
}
secret = b"your-secret-key"
token = security_manager.generate_token(data, secret)
print(f"Token: {token}")

# Verify token
is_valid = security_manager.verify_token(token, data, secret)
print(f"Token valid: {is_valid}")
```

### Input Validation

```python
# Validate email
email = "user@example.com"
is_valid = security_manager.validate_input(
    email,
    r"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$"
)
print(f"Email valid: {is_valid}")

# Validate phone number
phone = "+1234567890"
is_valid = security_manager.validate_input(
    phone,
    r"^\+[1-9]\d{1,14}$"
)
print(f"Phone valid: {is_valid}")
```

### Input Sanitization

```python
# Sanitize input
input_str = "<script>alert('test')</script>Test!@#$%^&*()  Data  "
sanitized = security_manager.sanitize_input(input_str)
print(f"Sanitized: {sanitized}")
```

## Advanced Usage

### Custom Validation Patterns

```python
# Validate URL
url = "https://example.com"
is_valid = security_manager.validate_input(
    url,
    r"^https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*)$"
)

# Validate date
date = "2024-03-21"
is_valid = security_manager.validate_input(
    date,
    r"^\d{4}-\d{2}-\d{2}$"
)
```

### Error Handling

```python
# Check for errors
encrypted = security_manager.encrypt_data("test")
if encrypted is None:
    print(f"Error: {security_manager.get_last_error()}")

# Clear error
security_manager.clear_last_error()
```

## Best Practices

1. Never store encryption keys in code
2. Use strong, unique salts for password hashing
3. Implement proper token expiration
4. Validate all user input
5. Sanitize data before storage
6. Use HTTPS for all communications
7. Implement rate limiting
8. Keep security libraries updated

## API Reference

### SecurityManager

#### Methods

- `initialize(encryption_key: Optional[bytes] = None) -> bool`: Initialize the security manager
- `encrypt_data(data: Union[str, bytes]) -> Optional[bytes]`: Encrypt data
- `decrypt_data(encrypted_data: bytes) -> Optional[bytes]`: Decrypt data
- `hash_password(password: str, salt: Optional[bytes] = None) -> Optional[Dict[str, bytes]]`: Hash password
- `verify_password(password: str, stored_hash: bytes, salt: bytes) -> bool`: Verify password
- `generate_token(data: Dict[str, Any], secret: bytes) -> Optional[str]`: Generate token
- `verify_token(token: str, data: Dict[str, Any], secret: bytes) -> bool`: Verify token
- `validate_input(input_str: str, pattern: str) -> bool`: Validate input
- `sanitize_input(input_str: str) -> str`: Sanitize input
- `get_last_error() -> str`: Get the last error message
- `clear_last_error() -> None`: Clear the last error message 