"""
Quantum Manager Python Bindings
"""

from typing import Dict, List, Optional, Tuple, Union
from satox_bindings.core.error_handling import SatoxError
from satox.quantum.native import satox

class QuantumManager:
    """Quantum Manager for handling quantum-resistant operations."""

    def __init__(self):
        """Initialize a new QuantumManager instance."""
        self._manager = satox.QuantumManager()
        self._initialized = False

    def initialize(self) -> bool:
        """
        Initialize the quantum manager.

        Returns:
            bool: True if initialization was successful

        Raises:
            SatoxError: If initialization fails
        """
        if not self._initialized:
            self._initialized = self._manager.initialize()
        return self._initialized

    def shutdown(self) -> bool:
        """
        Shutdown the quantum manager.

        Returns:
            bool: True if shutdown was successful

        Raises:
            SatoxError: If shutdown fails
        """
        if self._initialized:
            self._initialized = not self._manager.shutdown()
        return not self._initialized

    def generate_key_pair(self) -> Tuple[bytes, bytes]:
        """
        Generate a quantum-resistant key pair.

        Returns:
            Tuple[bytes, bytes]: A tuple containing (public_key, private_key)

        Raises:
            SatoxError: If the manager is not initialized or generation fails
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        return self._manager.generate_key_pair()

    def encrypt(self, public_key: bytes, data: bytes) -> bytes:
        """
        Encrypt data using quantum-resistant encryption.

        Args:
            public_key (bytes): The public key to use for encryption
            data (bytes): The data to encrypt

        Returns:
            bytes: The encrypted data

        Raises:
            SatoxError: If the manager is not initialized, input is invalid, or encryption fails
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        if not public_key or not data:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._manager.encrypt(public_key, data)

    def decrypt(self, private_key: bytes, encrypted_data: bytes) -> bytes:
        """
        Decrypt data using quantum-resistant decryption.

        Args:
            private_key (bytes): The private key to use for decryption
            encrypted_data (bytes): The data to decrypt

        Returns:
            bytes: The decrypted data

        Raises:
            SatoxError: If the manager is not initialized, input is invalid, or decryption fails
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        if not private_key or not encrypted_data:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._manager.decrypt(private_key, encrypted_data)

    def sign(self, private_key: bytes, data: bytes) -> bytes:
        """
        Sign data using quantum-resistant signature.

        Args:
            private_key (bytes): The private key to use for signing
            data (bytes): The data to sign

        Returns:
            bytes: The signature

        Raises:
            SatoxError: If the manager is not initialized, input is invalid, or signing fails
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        if not private_key or not data:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._manager.sign(private_key, data)

    def verify(self, public_key: bytes, data: bytes, signature: bytes) -> bool:
        """
        Verify a quantum-resistant signature.

        Args:
            public_key (bytes): The public key to use for verification
            data (bytes): The data that was signed
            signature (bytes): The signature to verify

        Returns:
            bool: True if the signature is valid

        Raises:
            SatoxError: If the manager is not initialized, input is invalid, or verification fails
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        if not public_key or not data or not signature:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._manager.verify(public_key, data, signature)

    def generate_random_number(self, min_value: int, max_value: int) -> int:
        """
        Generate a quantum-resistant random number.

        Args:
            min_value (int): The minimum value (inclusive)
            max_value (int): The maximum value (inclusive)

        Returns:
            int: The generated random number

        Raises:
            SatoxError: If the manager is not initialized, input is invalid, or generation fails
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        if min_value >= max_value:
            raise SatoxError("Invalid range", "INVALID_INPUT")
        return self._manager.generate_random_number(min_value, max_value)

    def generate_random_bytes(self, length: int) -> bytes:
        """
        Generate quantum-resistant random bytes.

        Args:
            length (int): The number of bytes to generate

        Returns:
            bytes: The generated random bytes

        Raises:
            SatoxError: If the manager is not initialized, input is invalid, or generation fails
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        if length <= 0:
            raise SatoxError("Invalid length", "INVALID_INPUT")
        return self._manager.generate_random_bytes(length)

    def is_initialized(self) -> bool:
        """
        Check if the quantum manager is initialized.

        Returns:
            bool: True if initialized
        """
        return self._initialized

    def get_version(self) -> str:
        """
        Get the version of the quantum manager.

        Returns:
            str: The version string
        """
        return self._manager.get_version()

    def get_algorithm(self) -> str:
        """
        Get the current quantum algorithm.

        Returns:
            str: The algorithm name

        Raises:
            SatoxError: If the manager is not initialized
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        return self._manager.get_algorithm()

    def get_available_algorithms(self) -> List[str]:
        """
        Get a list of available quantum algorithms.

        Returns:
            List[str]: List of algorithm names

        Raises:
            SatoxError: If the manager is not initialized
        """
        if not self._initialized:
            raise SatoxError("Quantum Manager not initialized", "NOT_INITIALIZED")
        return self._manager.get_available_algorithms() 