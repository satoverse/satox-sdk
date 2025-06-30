"""
Hybrid Encryption Python Bindings
"""

from typing import Dict, List, Optional, Tuple, Union
from satox_bindings.core.error_handling import SatoxError
from satox.quantum.native import satox

class HybridEncryption:
    """Hybrid Encryption for combining classical and quantum-resistant encryption."""

    def __init__(self):
        """Initialize a new HybridEncryption instance."""
        self._encryption = satox.HybridEncryption()
        self._initialized = False

    def initialize(self) -> bool:
        """
        Initialize the hybrid encryption system.

        Returns:
            bool: True if initialization was successful

        Raises:
            SatoxError: If initialization fails
        """
        if not self._initialized:
            self._initialized = self._encryption.initialize()
        return self._initialized

    def shutdown(self) -> bool:
        """
        Shutdown the hybrid encryption system.

        Returns:
            bool: True if shutdown was successful

        Raises:
            SatoxError: If shutdown fails
        """
        if self._initialized:
            self._initialized = not self._encryption.shutdown()
        return not self._initialized

    def generate_key_pair(self) -> Tuple[bytes, bytes]:
        """
        Generate a new key pair for hybrid encryption.

        Returns:
            Tuple[bytes, bytes]: A tuple containing (public_key, private_key)

        Raises:
            SatoxError: If the system is not initialized or generation fails
        """
        if not self._initialized:
            raise SatoxError("Hybrid Encryption not initialized", "NOT_INITIALIZED")
        return self._encryption.generate_key_pair()

    def encrypt(self, public_key: bytes, data: bytes) -> bytes:
        """
        Encrypt data using hybrid encryption.

        Args:
            public_key (bytes): The public key to use for encryption
            data (bytes): The data to encrypt

        Returns:
            bytes: The encrypted data

        Raises:
            SatoxError: If the system is not initialized, input is invalid, or encryption fails
        """
        if not self._initialized:
            raise SatoxError("Hybrid Encryption not initialized", "NOT_INITIALIZED")
        if not public_key or not data:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._encryption.encrypt(public_key, data)

    def decrypt(self, private_key: bytes, encrypted_data: bytes) -> bytes:
        """
        Decrypt data using hybrid encryption.

        Args:
            private_key (bytes): The private key to use for decryption
            encrypted_data (bytes): The data to decrypt

        Returns:
            bytes: The decrypted data

        Raises:
            SatoxError: If the system is not initialized, input is invalid, or decryption fails
        """
        if not self._initialized:
            raise SatoxError("Hybrid Encryption not initialized", "NOT_INITIALIZED")
        if not private_key or not encrypted_data:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._encryption.decrypt(private_key, encrypted_data)

    def get_session_key(self) -> bytes:
        """
        Get the current session key.

        Returns:
            bytes: The current session key

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Hybrid Encryption not initialized", "NOT_INITIALIZED")
        return self._encryption.get_session_key()

    def rotate_session_key(self) -> bool:
        """
        Rotate the session key.

        Returns:
            bool: True if rotation was successful

        Raises:
            SatoxError: If the system is not initialized or rotation fails
        """
        if not self._initialized:
            raise SatoxError("Hybrid Encryption not initialized", "NOT_INITIALIZED")
        return self._encryption.rotate_session_key()

    def is_initialized(self) -> bool:
        """
        Check if the hybrid encryption system is initialized.

        Returns:
            bool: True if initialized
        """
        return self._initialized

    def get_version(self) -> str:
        """
        Get the version of the hybrid encryption system.

        Returns:
            str: The version string
        """
        return self._encryption.get_version() 