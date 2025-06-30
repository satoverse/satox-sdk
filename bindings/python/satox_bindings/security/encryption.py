from typing import Dict, Optional, Any
from ..core.satox_error import SatoxError
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives import serialization
import base64
import os
import logging

class Encryption:
    """Handles encryption utilities."""
    
    def __init__(self):
        """Initialize the encryption handler."""
        self.symmetric_key = None
        self.fernet = None
        self.private_key = None
        self.public_key = None
        self.logger = logging.getLogger(__name__)
        
    def generate_symmetric_key(self, password: str, salt: Optional[bytes] = None) -> bytes:
        """Generate a symmetric encryption key.
        
        Args:
            password: Password to derive key from
            salt: Salt for key derivation (generated if not provided)
            
        Returns:
            Generated key
            
        Raises:
            SatoxError: If key generation fails
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
            self.symmetric_key = key
            self.fernet = Fernet(key)
            
            return key
            
        except Exception as e:
            raise SatoxError(f"Failed to generate symmetric key: {str(e)}")
            
    def generate_key_pair(self) -> Dict[str, bytes]:
        """Generate RSA key pair.
        
        Returns:
            Dictionary containing private and public keys
            
        Raises:
            SatoxError: If key generation fails
        """
        try:
            # Generate private key
            private_key = rsa.generate_private_key(
                public_exponent=65537,
                key_size=2048
            )
            
            # Get public key
            public_key = private_key.public_key()
            
            # Serialize keys
            private_pem = private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption()
            )
            
            public_pem = public_key.public_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PublicFormat.SubjectPublicKeyInfo
            )
            
            # Store keys
            self.private_key = private_key
            self.public_key = public_key
            
            return {
                'private_key': private_pem,
                'public_key': public_pem
            }
            
        except Exception as e:
            raise SatoxError(f"Failed to generate key pair: {str(e)}")
            
    def load_private_key(self, private_key_data: bytes) -> None:
        """Load a private key.
        
        Args:
            private_key_data: Private key data in PEM format
            
        Raises:
            SatoxError: If key loading fails
        """
        try:
            self.private_key = serialization.load_pem_private_key(
                private_key_data,
                password=None
            )
            
        except Exception as e:
            raise SatoxError(f"Failed to load private key: {str(e)}")
            
    def load_public_key(self, public_key_data: bytes) -> None:
        """Load a public key.
        
        Args:
            public_key_data: Public key data in PEM format
            
        Raises:
            SatoxError: If key loading fails
        """
        try:
            self.public_key = serialization.load_pem_public_key(public_key_data)
            
        except Exception as e:
            raise SatoxError(f"Failed to load public key: {str(e)}")
            
    def encrypt_symmetric(self, data: bytes) -> bytes:
        """Encrypt data using symmetric encryption.
        
        Args:
            data: Data to encrypt
            
        Returns:
            Encrypted data
            
        Raises:
            SatoxError: If encryption is not set up or encryption fails
        """
        if not self.fernet:
            raise SatoxError("Symmetric encryption is not set up")
            
        try:
            return self.fernet.encrypt(data)
            
        except Exception as e:
            raise SatoxError(f"Failed to encrypt data: {str(e)}")
            
    def decrypt_symmetric(self, encrypted_data: bytes) -> bytes:
        """Decrypt data using symmetric encryption.
        
        Args:
            encrypted_data: Data to decrypt
            
        Returns:
            Decrypted data
            
        Raises:
            SatoxError: If encryption is not set up or decryption fails
        """
        if not self.fernet:
            raise SatoxError("Symmetric encryption is not set up")
            
        try:
            return self.fernet.decrypt(encrypted_data)
            
        except Exception as e:
            raise SatoxError(f"Failed to decrypt data: {str(e)}")
            
    def encrypt_asymmetric(self, data: bytes) -> bytes:
        """Encrypt data using asymmetric encryption.
        
        Args:
            data: Data to encrypt
            
        Returns:
            Encrypted data
            
        Raises:
            SatoxError: If public key is not loaded or encryption fails
        """
        if not self.public_key:
            raise SatoxError("Public key is not loaded")
            
        try:
            return self.public_key.encrypt(
                data,
                padding.OAEP(
                    mgf=padding.MGF1(algorithm=hashes.SHA256()),
                    algorithm=hashes.SHA256(),
                    label=None
                )
            )
            
        except Exception as e:
            raise SatoxError(f"Failed to encrypt data: {str(e)}")
            
    def decrypt_asymmetric(self, encrypted_data: bytes) -> bytes:
        """Decrypt data using asymmetric encryption.
        
        Args:
            encrypted_data: Data to decrypt
            
        Returns:
            Decrypted data
            
        Raises:
            SatoxError: If private key is not loaded or decryption fails
        """
        if not self.private_key:
            raise SatoxError("Private key is not loaded")
            
        try:
            return self.private_key.decrypt(
                encrypted_data,
                padding.OAEP(
                    mgf=padding.MGF1(algorithm=hashes.SHA256()),
                    algorithm=hashes.SHA256(),
                    label=None
                )
            )
            
        except Exception as e:
            raise SatoxError(f"Failed to decrypt data: {str(e)}")
            
    def sign_data(self, data: bytes) -> bytes:
        """Sign data using private key.
        
        Args:
            data: Data to sign
            
        Returns:
            Signature
            
        Raises:
            SatoxError: If private key is not loaded or signing fails
        """
        if not self.private_key:
            raise SatoxError("Private key is not loaded")
            
        try:
            return self.private_key.sign(
                data,
                padding.PSS(
                    mgf=padding.MGF1(hashes.SHA256()),
                    salt_length=padding.PSS.MAX_LENGTH
                ),
                hashes.SHA256()
            )
            
        except Exception as e:
            raise SatoxError(f"Failed to sign data: {str(e)}")
            
    def verify_signature(self, data: bytes, signature: bytes) -> bool:
        """Verify signature using public key.
        
        Args:
            data: Original data
            signature: Signature to verify
            
        Returns:
            True if signature is valid, False otherwise
            
        Raises:
            SatoxError: If public key is not loaded or verification fails
        """
        if not self.public_key:
            raise SatoxError("Public key is not loaded")
            
        try:
            self.public_key.verify(
                signature,
                data,
                padding.PSS(
                    mgf=padding.MGF1(hashes.SHA256()),
                    salt_length=padding.PSS.MAX_LENGTH
                ),
                hashes.SHA256()
            )
            return True
            
        except Exception as e:
            self.logger.error(f"Signature verification failed: {str(e)}")
            return False 