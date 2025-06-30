"""
Key Storage Python Bindings
"""

from typing import Dict, List, Optional, Tuple, Union
from dataclasses import dataclass
from satox_bindings.core.error_handling import SatoxError
from satox.quantum.native import satox

@dataclass
class KeyMetadata:
    """Metadata for a stored key."""
    algorithm: str
    created: int
    expires: Optional[int] = None
    access: List[str] = None
    tags: List[str] = None

class KeyStorage:
    """Secure key storage system."""

    def __init__(self):
        """Initialize a new KeyStorage instance."""
        self._storage = satox.KeyStorage()
        self._initialized = False

    def initialize(self) -> bool:
        """
        Initialize the key storage system.

        Returns:
            bool: True if initialization was successful

        Raises:
            SatoxError: If initialization fails
        """
        if not self._initialized:
            self._initialized = self._storage.initialize()
        return self._initialized

    def shutdown(self) -> bool:
        """
        Shutdown the key storage system.

        Returns:
            bool: True if shutdown was successful

        Raises:
            SatoxError: If shutdown fails
        """
        if self._initialized:
            self._initialized = not self._storage.shutdown()
        return not self._initialized

    def store_key(self, key_id: str, key: bytes, metadata: Optional[KeyMetadata] = None) -> bool:
        """
        Store a key in the storage system.

        Args:
            key_id (str): The unique identifier for the key
            key (bytes): The key data to store
            metadata (Optional[KeyMetadata]): Optional metadata for the key

        Returns:
            bool: True if storage was successful

        Raises:
            SatoxError: If the system is not initialized or storage fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id or not key:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._storage.store_key(key_id, key, metadata)

    def retrieve_key(self, key_id: str) -> bytes:
        """
        Retrieve a key from the storage system.

        Args:
            key_id (str): The unique identifier of the key to retrieve

        Returns:
            bytes: The retrieved key data

        Raises:
            SatoxError: If the system is not initialized or retrieval fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id:
            raise SatoxError("Invalid key ID", "INVALID_INPUT")
        return self._storage.retrieve_key(key_id)

    def delete_key(self, key_id: str) -> bool:
        """
        Delete a key from the storage system.

        Args:
            key_id (str): The unique identifier of the key to delete

        Returns:
            bool: True if deletion was successful

        Raises:
            SatoxError: If the system is not initialized or deletion fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id:
            raise SatoxError("Invalid key ID", "INVALID_INPUT")
        return self._storage.delete_key(key_id)

    def update_key(self, key_id: str, key: bytes, metadata: Optional[KeyMetadata] = None) -> bool:
        """
        Update a key in the storage system.

        Args:
            key_id (str): The unique identifier of the key to update
            key (bytes): The new key data
            metadata (Optional[KeyMetadata]): Optional new metadata for the key

        Returns:
            bool: True if update was successful

        Raises:
            SatoxError: If the system is not initialized or update fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id or not key:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._storage.update_key(key_id, key, metadata)

    def set_key_metadata(self, key_id: str, metadata: KeyMetadata) -> bool:
        """
        Set metadata for a key.

        Args:
            key_id (str): The unique identifier of the key
            metadata (KeyMetadata): The metadata to set

        Returns:
            bool: True if setting was successful

        Raises:
            SatoxError: If the system is not initialized or setting fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id or not metadata:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._storage.set_key_metadata(key_id, metadata)

    def get_key_metadata(self, key_id: str) -> KeyMetadata:
        """
        Get metadata for a key.

        Args:
            key_id (str): The unique identifier of the key

        Returns:
            KeyMetadata: The key metadata

        Raises:
            SatoxError: If the system is not initialized or retrieval fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id:
            raise SatoxError("Invalid key ID", "INVALID_INPUT")
        return self._storage.get_key_metadata(key_id)

    def rotate_key(self, key_id: str) -> bool:
        """
        Rotate a key in the storage system.

        Args:
            key_id (str): The unique identifier of the key to rotate

        Returns:
            bool: True if rotation was successful

        Raises:
            SatoxError: If the system is not initialized or rotation fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id:
            raise SatoxError("Invalid key ID", "INVALID_INPUT")
        return self._storage.rotate_key(key_id)

    def reencrypt_key(self, key_id: str, new_key: bytes) -> bool:
        """
        Re-encrypt a key with a new encryption key.

        Args:
            key_id (str): The unique identifier of the key to re-encrypt
            new_key (bytes): The new encryption key

        Returns:
            bool: True if re-encryption was successful

        Raises:
            SatoxError: If the system is not initialized or re-encryption fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id or not new_key:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._storage.reencrypt_key(key_id, new_key)

    def validate_key(self, key_id: str) -> bool:
        """
        Validate a key in the storage system.

        Args:
            key_id (str): The unique identifier of the key to validate

        Returns:
            bool: True if the key is valid

        Raises:
            SatoxError: If the system is not initialized or validation fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id:
            raise SatoxError("Invalid key ID", "INVALID_INPUT")
        return self._storage.validate_key(key_id)

    def validate_all_keys(self) -> bool:
        """
        Validate all keys in the storage system.

        Returns:
            bool: True if all keys are valid

        Raises:
            SatoxError: If the system is not initialized or validation fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        return self._storage.validate_all_keys()

    def set_key_expiration(self, key_id: str, expiration_time: int) -> bool:
        """
        Set expiration time for a key.

        Args:
            key_id (str): The unique identifier of the key
            expiration_time (int): The expiration timestamp

        Returns:
            bool: True if setting was successful

        Raises:
            SatoxError: If the system is not initialized or setting fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id or expiration_time <= 0:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._storage.set_key_expiration(key_id, expiration_time)

    def get_key_expiration(self, key_id: str) -> int:
        """
        Get expiration time for a key.

        Args:
            key_id (str): The unique identifier of the key

        Returns:
            int: The expiration timestamp

        Raises:
            SatoxError: If the system is not initialized or retrieval fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id:
            raise SatoxError("Invalid key ID", "INVALID_INPUT")
        return self._storage.get_key_expiration(key_id)

    def is_key_expired(self, key_id: str) -> bool:
        """
        Check if a key has expired.

        Args:
            key_id (str): The unique identifier of the key

        Returns:
            bool: True if the key has expired

        Raises:
            SatoxError: If the system is not initialized or check fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id:
            raise SatoxError("Invalid key ID", "INVALID_INPUT")
        return self._storage.is_key_expired(key_id)

    def set_key_access(self, key_id: str, access: List[str]) -> bool:
        """
        Set access level for a key.

        Args:
            key_id (str): The unique identifier of the key
            access (List[str]): The access level to set

        Returns:
            bool: True if setting was successful

        Raises:
            SatoxError: If the system is not initialized or setting fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id or not access:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._storage.set_key_access(key_id, access)

    def get_key_access(self, key_id: str) -> List[str]:
        """
        Get access level for a key.

        Args:
            key_id (str): The unique identifier of the key

        Returns:
            List[str]: The access levels

        Raises:
            SatoxError: If the system is not initialized or retrieval fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id:
            raise SatoxError("Invalid key ID", "INVALID_INPUT")
        return self._storage.get_key_access(key_id)

    def check_key_access(self, key_id: str, access: str) -> bool:
        """
        Check if a key has a specific access level.

        Args:
            key_id (str): The unique identifier of the key
            access (str): The access level to check

        Returns:
            bool: True if the key has the access level

        Raises:
            SatoxError: If the system is not initialized or check fails
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        if not key_id or not access:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._storage.check_key_access(key_id, access)

    def is_initialized(self) -> bool:
        """
        Check if the key storage system is initialized.

        Returns:
            bool: True if initialized
        """
        return self._initialized

    def get_version(self) -> str:
        """
        Get the version of the key storage system.

        Returns:
            str: The version string
        """
        return self._storage.get_version()

    def get_algorithm(self) -> str:
        """
        Get the current algorithm used by the key storage system.

        Returns:
            str: The algorithm name

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        return self._storage.get_algorithm()

    def count_keys(self) -> int:
        """
        Get the count of stored keys.

        Returns:
            int: The number of stored keys

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        return self._storage.count_keys()

    def get_all_key_ids(self) -> List[str]:
        """
        Get all key IDs stored in the system.

        Returns:
            List[str]: Array of key IDs

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Key Storage not initialized", "NOT_INITIALIZED")
        return self._storage.get_all_key_ids() 