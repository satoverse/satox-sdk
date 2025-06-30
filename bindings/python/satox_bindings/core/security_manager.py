"""Security Manager for Satox SDK.
Handles security-related operations and key management.
"""

from typing import Optional, Tuple, Dict
import json
import hashlib
import hmac
import os
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives import serialization

class SecurityManager:
    """Manages security operations and cryptographic functions."""
    
    def __init__(self):
        """Initialize the security manager."""
        self._initialized = False
        self._last_error = ""
        self._fernet = None
        self._private_key = None
        self._public_key = None

    def initialize(self) -> bool:
        """Initialize the security manager."""
        try:
            # Generate encryption key
            salt = os.urandom(16)
            kdf = PBKDF2HMAC(
                algorithm=hashes.SHA256(),
                length=32,
                salt=salt,
                iterations=100000,
            )
            key = base64.urlsafe_b64encode(kdf.derive(os.urandom(32)))
            self._fernet = Fernet(key)

            # Generate RSA key pair
            self._private_key = rsa.generate_private_key(
                public_exponent=65537,
                key_size=2048
            )
            self._public_key = self._private_key.public_key()

            self._initialized = True
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def shutdown(self) -> bool:
        """Shutdown the security manager."""
        try:
            self._initialized = False
            self._fernet = None
            self._private_key = None
            self._public_key = None
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def encrypt_data(self, data: bytes) -> Optional[bytes]:
        """Encrypt data using Fernet symmetric encryption."""
        if not self._initialized:
            self._last_error = "Security manager not initialized"
            return None
        try:
            return self._fernet.encrypt(data)
        except Exception as e:
            self._last_error = str(e)
            return None

    def decrypt_data(self, encrypted_data: bytes) -> Optional[bytes]:
        """Decrypt data using Fernet symmetric encryption."""
        if not self._initialized:
            self._last_error = "Security manager not initialized"
            return None
        try:
            return self._fernet.decrypt(encrypted_data)
        except Exception as e:
            self._last_error = str(e)
            return None

    def sign_data(self, data: bytes) -> Optional[bytes]:
        """Sign data using RSA private key."""
        if not self._initialized:
            self._last_error = "Security manager not initialized"
            return None
        try:
            signature = self._private_key.sign(
                data,
                padding.PSS(
                    mgf=padding.MGF1(hashes.SHA256()),
                    salt_length=padding.PSS.MAX_LENGTH
                ),
                hashes.SHA256()
            )
            return signature
        except Exception as e:
            self._last_error = str(e)
            return None

    def verify_signature(self, data: bytes, signature: bytes) -> bool:
        """Verify signature using RSA public key."""
        if not self._initialized:
            self._last_error = "Security manager not initialized"
            return False
        try:
            self._public_key.verify(
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
            self._last_error = str(e)
            return False

    def generate_key_pair(self) -> Optional[Tuple[bytes, bytes]]:
        """Generate a new RSA key pair."""
        if not self._initialized:
            self._last_error = "Security manager not initialized"
            return None
        try:
            private_key = rsa.generate_private_key(
                public_exponent=65537,
                key_size=2048
            )
            public_key = private_key.public_key()

            private_pem = private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption()
            )
            public_pem = public_key.public_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PublicFormat.SubjectPublicKeyInfo
            )

            return private_pem, public_pem
        except Exception as e:
            self._last_error = str(e)
            return None

    def hash_data(self, data: bytes) -> Optional[bytes]:
        """Hash data using SHA-256."""
        if not self._initialized:
            self._last_error = "Security manager not initialized"
            return None
        try:
            return hashlib.sha256(data).digest()
        except Exception as e:
            self._last_error = str(e)
            return None

    def get_public_key(self) -> Optional[bytes]:
        """Get the public key in PEM format."""
        if not self._initialized:
            self._last_error = "Security manager not initialized"
            return None
        try:
            return self._public_key.public_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PublicFormat.SubjectPublicKeyInfo
            )
        except Exception as e:
            self._last_error = str(e)
            return None

    def get_last_error(self) -> str:
        """Get the last error message."""
        return self._last_error

    def clear_last_error(self) -> None:
        """Clear the last error message."""
        self._last_error = ""
