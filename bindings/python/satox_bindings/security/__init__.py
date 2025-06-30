from .authentication import Authentication
from .authorization import Authorization
from .encryption import Encryption
from .key_manager import KeyManager

__all__ = [
    'Authentication',
    'Authorization',
    'Encryption',
    'KeyManager'
]

"""
Satox SDK Security Module
"""

from typing import Dict, List, Optional, Any, Union
from datetime import datetime
import json
import hashlib
import hmac
import base64
import re
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC

class SecurityManager:
    """Manages security operations"""
    
    def __init__(self):
        self._last_error = ""
        self._encryption_key: Optional[bytes] = None
    
    def initialize(self, encryption_key: Optional[bytes] = None) -> bool:
        """
        Initialize security manager
        
        Args:
            encryption_key: Optional encryption key
            
        Returns:
            bool: True if initialization successful, False otherwise
        """
        try:
            if encryption_key:
                self._encryption_key = encryption_key
            else:
                # Generate new encryption key
                self._encryption_key = Fernet.generate_key()
            
            return True
            
        except Exception as e:
            self._last_error = f"Failed to initialize security manager: {str(e)}"
            return False
    
    def encrypt_data(self, data: Union[str, bytes]) -> Optional[bytes]:
        """
        Encrypt data
        
        Args:
            data: Data to encrypt
            
        Returns:
            Optional[bytes]: Encrypted data
        """
        try:
            if not self._encryption_key:
                self._last_error = "Security manager not initialized"
                return None
            
            # Convert string to bytes if necessary
            if isinstance(data, str):
                data = data.encode()
            
            # Create Fernet instance
            f = Fernet(self._encryption_key)
            
            # Encrypt data
            return f.encrypt(data)
            
        except Exception as e:
            self._last_error = f"Failed to encrypt data: {str(e)}"
            return None
    
    def decrypt_data(self, encrypted_data: bytes) -> Optional[bytes]:
        """
        Decrypt data
        
        Args:
            encrypted_data: Data to decrypt
            
        Returns:
            Optional[bytes]: Decrypted data
        """
        try:
            if not self._encryption_key:
                self._last_error = "Security manager not initialized"
                return None
            
            # Create Fernet instance
            f = Fernet(self._encryption_key)
            
            # Decrypt data
            return f.decrypt(encrypted_data)
            
        except Exception as e:
            self._last_error = f"Failed to decrypt data: {str(e)}"
            return None
    
    def hash_password(self, password: str, salt: Optional[bytes] = None) -> Optional[Dict[str, bytes]]:
        """
        Hash password using PBKDF2
        
        Args:
            password: Password to hash
            salt: Optional salt
            
        Returns:
            Optional[Dict[str, bytes]]: Dictionary containing hash and salt
        """
        try:
            # Generate salt if not provided
            if not salt:
                salt = os.urandom(16)
            
            # Create PBKDF2 instance
            kdf = PBKDF2HMAC(
                algorithm=hashes.SHA256(),
                length=32,
                salt=salt,
                iterations=100000
            )
            
            # Hash password
            key = kdf.derive(password.encode())
            
            return {
                'hash': key,
                'salt': salt
            }
            
        except Exception as e:
            self._last_error = f"Failed to hash password: {str(e)}"
            return None
    
    def verify_password(self, password: str, stored_hash: bytes, salt: bytes) -> bool:
        """
        Verify password against stored hash
        
        Args:
            password: Password to verify
            stored_hash: Stored hash
            salt: Salt used for hashing
            
        Returns:
            bool: True if password matches, False otherwise
        """
        try:
            # Create PBKDF2 instance
            kdf = PBKDF2HMAC(
                algorithm=hashes.SHA256(),
                length=32,
                salt=salt,
                iterations=100000
            )
            
            # Hash password
            key = kdf.derive(password.encode())
            
            # Compare hashes
            return hmac.compare_digest(key, stored_hash)
            
        except Exception as e:
            self._last_error = f"Failed to verify password: {str(e)}"
            return False
    
    def generate_token(self, data: Dict[str, Any], secret: bytes) -> Optional[str]:
        """
        Generate HMAC token
        
        Args:
            data: Data to include in token
            secret: Secret key
            
        Returns:
            Optional[str]: Generated token
        """
        try:
            # Convert data to JSON
            data_json = json.dumps(data, sort_keys=True)
            
            # Create HMAC
            h = hmac.new(secret, data_json.encode(), hashlib.sha256)
            
            # Generate token
            token = base64.urlsafe_b64encode(h.digest()).decode()
            
            return token
            
        except Exception as e:
            self._last_error = f"Failed to generate token: {str(e)}"
            return None
    
    def verify_token(self, token: str, data: Dict[str, Any], secret: bytes) -> bool:
        """
        Verify HMAC token
        
        Args:
            token: Token to verify
            data: Data included in token
            secret: Secret key
            
        Returns:
            bool: True if token is valid, False otherwise
        """
        try:
            # Generate expected token
            expected_token = self.generate_token(data, secret)
            
            if not expected_token:
                return False
            
            # Compare tokens
            return hmac.compare_digest(token, expected_token)
            
        except Exception as e:
            self._last_error = f"Failed to verify token: {str(e)}"
            return False
    
    def validate_input(self, input_str: str, pattern: str) -> bool:
        """
        Validate input against pattern
        
        Args:
            input_str: Input to validate
            pattern: Regex pattern
            
        Returns:
            bool: True if input matches pattern, False otherwise
        """
        try:
            return bool(re.match(pattern, input_str))
            
        except Exception as e:
            self._last_error = f"Failed to validate input: {str(e)}"
            return False
    
    def sanitize_input(self, input_str: str) -> str:
        """
        Sanitize input string
        
        Args:
            input_str: Input to sanitize
            
        Returns:
            str: Sanitized input
        """
        try:
            # Remove HTML tags
            input_str = re.sub(r'<[^>]+>', '', input_str)
            
            # Remove special characters
            input_str = re.sub(r'[^\w\s-]', '', input_str)
            
            # Remove extra whitespace
            input_str = ' '.join(input_str.split())
            
            return input_str
            
        except Exception as e:
            self._last_error = f"Failed to sanitize input: {str(e)}"
            return input_str
    
    def get_last_error(self) -> str:
        """
        Get the last error message
        
        Returns:
            str: Last error message
        """
        return self._last_error
    
    def clear_last_error(self) -> None:
        """Clear the last error message"""
        self._last_error = ""

# Export the manager instance
security_manager = SecurityManager()
