from typing import Dict, Any, Optional, Tuple
import os
import json
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives import serialization
from ..core.error_handling import SatoxError

class KeyManager:
    def __init__(self, storage_path: str):
        """
        Initialize the KeyManager with a storage path.
        
        Args:
            storage_path: Path to store keys
        """
        self._storage_path = storage_path
        self._ensure_storage_directory()
        self._keys: Dict[str, Dict[str, Any]] = {}
        self._load_keys()

    def _ensure_storage_directory(self):
        """Ensure the storage directory exists."""
        try:
            os.makedirs(self._storage_path, exist_ok=True)
        except Exception as e:
            raise SatoxError(f"Failed to create storage directory: {str(e)}")

    def _get_key_path(self, key_id: str) -> str:
        """Get the full path for a key file."""
        return os.path.join(self._storage_path, f"{key_id}.json")

    def _load_keys(self):
        """Load all keys from storage."""
        try:
            for filename in os.listdir(self._storage_path):
                if filename.endswith('.json'):
                    key_id = filename[:-5]
                    with open(os.path.join(self._storage_path, filename), 'r') as f:
                        self._keys[key_id] = json.load(f)
        except Exception as e:
            raise SatoxError(f"Failed to load keys: {str(e)}")

    def generate_key_pair(self, key_id: str, password: str) -> Tuple[str, str]:
        """
        Generate a new RSA key pair.
        
        Args:
            key_id: Unique identifier for the key pair
            password: Password to encrypt the private key
            
        Returns:
            Tuple[str, str]: Public and private key strings
            
        Raises:
            SatoxError: If key generation fails
        """
        try:
            # Generate RSA key pair
            private_key = rsa.generate_private_key(
                public_exponent=65537,
                key_size=2048
            )
            public_key = private_key.public_key()

            # Serialize keys
            private_pem = private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.BestAvailableEncryption(password.encode())
            )
            public_pem = public_key.public_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PublicFormat.SubjectPublicKeyInfo
            )

            # Store keys
            key_data = {
                'public_key': public_pem.decode(),
                'private_key': private_pem.decode(),
                'type': 'rsa'
            }
            self._keys[key_id] = key_data
            self._save_key(key_id, key_data)

            return public_pem.decode(), private_pem.decode()
        except Exception as e:
            raise SatoxError(f"Failed to generate key pair: {str(e)}")

    def _save_key(self, key_id: str, key_data: Dict[str, Any]):
        """
        Save key data to storage.
        
        Args:
            key_id: ID of the key
            key_data: Key data to save
        """
        try:
            with open(self._get_key_path(key_id), 'w') as f:
                json.dump(key_data, f)
        except Exception as e:
            raise SatoxError(f"Failed to save key: {str(e)}")

    def get_public_key(self, key_id: str) -> Optional[str]:
        """
        Get a public key by ID.
        
        Args:
            key_id: ID of the key
            
        Returns:
            Optional[str]: Public key if found
        """
        key_data = self._keys.get(key_id)
        return key_data.get('public_key') if key_data else None

    def get_private_key(self, key_id: str, password: str) -> Optional[str]:
        """
        Get a private key by ID.
        
        Args:
            key_id: ID of the key
            password: Password to decrypt the private key
            
        Returns:
            Optional[str]: Private key if found
            
        Raises:
            SatoxError: If decryption fails
        """
        try:
            key_data = self._keys.get(key_id)
            if not key_data:
                return None

            private_key = key_data.get('private_key')
            if not private_key:
                return None

            # Load and decrypt private key
            private_key = serialization.load_pem_private_key(
                private_key.encode(),
                password=password.encode()
            )
            return private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.PKCS8,
                encryption_algorithm=serialization.NoEncryption()
            ).decode()
        except Exception as e:
            raise SatoxError(f"Failed to get private key: {str(e)}")

    def delete_key(self, key_id: str) -> bool:
        """
        Delete a key pair.
        
        Args:
            key_id: ID of the key to delete
            
        Returns:
            bool: True if deletion was successful
            
        Raises:
            SatoxError: If deletion fails
        """
        try:
            if key_id in self._keys:
                del self._keys[key_id]
                os.remove(self._get_key_path(key_id))
            return True
        except Exception as e:
            raise SatoxError(f"Failed to delete key: {str(e)}")

    def list_keys(self) -> List[str]:
        """
        Get a list of all key IDs.
        
        Returns:
            List[str]: List of key IDs
        """
        return list(self._keys.keys())

    def sign_data(self, key_id: str, data: bytes, password: str) -> bytes:
        """
        Sign data using a private key.
        
        Args:
            key_id: ID of the key to use
            data: Data to sign
            password: Password to decrypt the private key
            
        Returns:
            bytes: Signature
            
        Raises:
            SatoxError: If signing fails
        """
        try:
            private_key = self.get_private_key(key_id, password)
            if not private_key:
                raise SatoxError(f"Private key not found for ID {key_id}")

            private_key = serialization.load_pem_private_key(
                private_key.encode(),
                password=None
            )
            
            signature = private_key.sign(
                data,
                padding.PSS(
                    mgf=padding.MGF1(hashes.SHA256()),
                    salt_length=padding.PSS.MAX_LENGTH
                ),
                hashes.SHA256()
            )
            
            return signature
        except Exception as e:
            raise SatoxError(f"Failed to sign data: {str(e)}")

    def verify_signature(self, key_id: str, data: bytes, signature: bytes) -> bool:
        """
        Verify a signature using a public key.
        
        Args:
            key_id: ID of the key to use
            data: Original data
            signature: Signature to verify
            
        Returns:
            bool: True if signature is valid
            
        Raises:
            SatoxError: If verification fails
        """
        try:
            public_key = self.get_public_key(key_id)
            if not public_key:
                raise SatoxError(f"Public key not found for ID {key_id}")

            public_key = serialization.load_pem_public_key(public_key.encode())
            
            try:
                public_key.verify(
                    signature,
                    data,
                    padding.PSS(
                        mgf=padding.MGF1(hashes.SHA256()),
                        salt_length=padding.PSS.MAX_LENGTH
                    ),
                    hashes.SHA256()
                )
                return True
            except Exception:
                return False
        except Exception as e:
            raise SatoxError(f"Failed to verify signature: {str(e)}")

    def encrypt_data(self, key_id: str, data: bytes) -> bytes:
        """
        Encrypt data using a public key.
        
        Args:
            key_id: ID of the key to use
            data: Data to encrypt
            
        Returns:
            bytes: Encrypted data
            
        Raises:
            SatoxError: If encryption fails
        """
        try:
            public_key = self.get_public_key(key_id)
            if not public_key:
                raise SatoxError(f"Public key not found for ID {key_id}")

            public_key = serialization.load_pem_public_key(public_key.encode())
            
            encrypted_data = public_key.encrypt(
                data,
                padding.OAEP(
                    mgf=padding.MGF1(algorithm=hashes.SHA256()),
                    algorithm=hashes.SHA256(),
                    label=None
                )
            )
            
            return encrypted_data
        except Exception as e:
            raise SatoxError(f"Failed to encrypt data: {str(e)}")

    def decrypt_data(self, key_id: str, encrypted_data: bytes, password: str) -> bytes:
        """
        Decrypt data using a private key.
        
        Args:
            key_id: ID of the key to use
            encrypted_data: Data to decrypt
            password: Password to decrypt the private key
            
        Returns:
            bytes: Decrypted data
            
        Raises:
            SatoxError: If decryption fails
        """
        try:
            private_key = self.get_private_key(key_id, password)
            if not private_key:
                raise SatoxError(f"Private key not found for ID {key_id}")

            private_key = serialization.load_pem_private_key(
                private_key.encode(),
                password=None
            )
            
            decrypted_data = private_key.decrypt(
                encrypted_data,
                padding.OAEP(
                    mgf=padding.MGF1(algorithm=hashes.SHA256()),
                    algorithm=hashes.SHA256(),
                    label=None
                )
            )
            
            return decrypted_data
        except Exception as e:
            raise SatoxError(f"Failed to decrypt data: {str(e)}") 