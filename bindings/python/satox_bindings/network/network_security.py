from typing import Dict, List, Optional, Any
from ..core.satox_error import SatoxError
import ssl
import logging
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
import base64
import os

class NetworkSecurity:
    """Handles network security features."""
    
    def __init__(self):
        """Initialize the network security handler."""
        self.ssl_context = None
        self.encryption_key = None
        self.fernet = None
        self.logger = logging.getLogger(__name__)
        
    def setup_ssl(self, cert_file: str, key_file: str) -> None:
        """Set up SSL context.
        
        Args:
            cert_file: Path to certificate file
            key_file: Path to private key file
            
        Raises:
            SatoxError: If SSL setup fails
        """
        try:
            self.ssl_context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
            self.ssl_context.load_cert_chain(cert_file, key_file)
            self.ssl_context.verify_mode = ssl.CERT_REQUIRED
            self.ssl_context.check_hostname = True
            
        except Exception as e:
            raise SatoxError(f"Failed to set up SSL: {str(e)}")
            
    def setup_encryption(self, password: str, salt: Optional[bytes] = None) -> None:
        """Set up encryption.
        
        Args:
            password: Encryption password
            salt: Encryption salt (generated if not provided)
            
        Raises:
            SatoxError: If encryption setup fails
        """
        try:
            # Generate salt if not provided
            if not salt:
                salt = os.urandom(16)
                
            # Generate key
            kdf = PBKDF2HMAC(
                algorithm=hashes.SHA256(),
                length=32,
                salt=salt,
                iterations=100000
            )
            
            key = base64.urlsafe_b64encode(kdf.derive(password.encode()))
            self.encryption_key = key
            self.fernet = Fernet(key)
            
        except Exception as e:
            raise SatoxError(f"Failed to set up encryption: {str(e)}")
            
    def encrypt_data(self, data: bytes) -> bytes:
        """Encrypt data.
        
        Args:
            data: Data to encrypt
            
        Returns:
            Encrypted data
            
        Raises:
            SatoxError: If encryption is not set up or encryption fails
        """
        if not self.fernet:
            raise SatoxError("Encryption is not set up")
            
        try:
            return self.fernet.encrypt(data)
            
        except Exception as e:
            raise SatoxError(f"Failed to encrypt data: {str(e)}")
            
    def decrypt_data(self, encrypted_data: bytes) -> bytes:
        """Decrypt data.
        
        Args:
            encrypted_data: Data to decrypt
            
        Returns:
            Decrypted data
            
        Raises:
            SatoxError: If encryption is not set up or decryption fails
        """
        if not self.fernet:
            raise SatoxError("Encryption is not set up")
            
        try:
            return self.fernet.decrypt(encrypted_data)
            
        except Exception as e:
            raise SatoxError(f"Failed to decrypt data: {str(e)}")
            
    def get_ssl_context(self) -> Optional[ssl.SSLContext]:
        """Get SSL context.
        
        Returns:
            SSL context if set up, None otherwise
        """
        return self.ssl_context
        
    def is_encryption_setup(self) -> bool:
        """Check if encryption is set up.
        
        Returns:
            True if encryption is set up, False otherwise
        """
        return self.fernet is not None
        
    def rotate_encryption_key(self, new_password: str) -> None:
        """Rotate encryption key.
        
        Args:
            new_password: New encryption password
            
        Raises:
            SatoxError: If encryption is not set up or key rotation fails
        """
        if not self.fernet:
            raise SatoxError("Encryption is not set up")
            
        try:
            # Generate new salt
            salt = os.urandom(16)
            
            # Generate new key
            kdf = PBKDF2HMAC(
                algorithm=hashes.SHA256(),
                length=32,
                salt=salt,
                iterations=100000
            )
            
            new_key = base64.urlsafe_b64encode(kdf.derive(new_password.encode()))
            
            # Update encryption
            self.encryption_key = new_key
            self.fernet = Fernet(new_key)
            
        except Exception as e:
            raise SatoxError(f"Failed to rotate encryption key: {str(e)}")
            
    def verify_ssl_certificate(self, cert_data: bytes) -> bool:
        """Verify SSL certificate.
        
        Args:
            cert_data: Certificate data
            
        Returns:
            True if certificate is valid, False otherwise
            
        Raises:
            SatoxError: If SSL is not set up or verification fails
        """
        if not self.ssl_context:
            raise SatoxError("SSL is not set up")
            
        try:
            # Load certificate
            cert = ssl.PEM_cert_to_DER_cert(cert_data.decode())
            
            # Verify certificate
            self.ssl_context.verify_mode = ssl.CERT_REQUIRED
            self.ssl_context.check_hostname = True
            
            return True
            
        except Exception as e:
            self.logger.error(f"Certificate verification failed: {str(e)}")
            return False 