"""
Post-Quantum Algorithms Python Bindings
"""

from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
from satox_bindings.core.error_handling import SatoxError
from satox.quantum.native import satox

@dataclass
class AlgorithmInfo:
    """Information about a post-quantum algorithm."""
    name: str
    security_level: int
    key_size: int
    signature_size: int
    is_recommended: bool
    description: str

class PostQuantumAlgorithms:
    """Post-quantum cryptographic algorithms implementation."""

    def __init__(self):
        """Initialize a new PostQuantumAlgorithms instance."""
        self._algorithms = satox.PostQuantumAlgorithms()
        self._initialized = False

    def initialize(self) -> bool:
        """
        Initialize the post-quantum algorithms system.

        Returns:
            bool: True if initialization was successful

        Raises:
            SatoxError: If initialization fails
        """
        if not self._initialized:
            self._initialized = self._algorithms.initialize()
        return self._initialized

    def shutdown(self) -> bool:
        """
        Shutdown the post-quantum algorithms system.

        Returns:
            bool: True if shutdown was successful

        Raises:
            SatoxError: If shutdown fails
        """
        if self._initialized:
            self._initialized = not self._algorithms.shutdown()
        return not self._initialized

    def generate_key_pair(self, algorithm_name: str) -> Tuple[bytes, bytes]:
        """
        Generate a key pair using the specified algorithm.

        Args:
            algorithm_name (str): The name of the algorithm to use

        Returns:
            Tuple[bytes, bytes]: A tuple containing (public_key, private_key)

        Raises:
            SatoxError: If the system is not initialized or generation fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name:
            raise SatoxError("Invalid algorithm name", "INVALID_INPUT")
        return self._algorithms.generate_key_pair(algorithm_name)

    def sign(self, algorithm_name: str, private_key: bytes, data: bytes) -> bytes:
        """
        Sign data using the specified algorithm and private key.

        Args:
            algorithm_name (str): The name of the algorithm to use
            private_key (bytes): The private key to use for signing
            data (bytes): The data to sign

        Returns:
            bytes: The signature

        Raises:
            SatoxError: If the system is not initialized or signing fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name or not private_key or not data:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._algorithms.sign(algorithm_name, private_key, data)

    def verify(self, algorithm_name: str, public_key: bytes, data: bytes, signature: bytes) -> bool:
        """
        Verify a signature using the specified algorithm and public key.

        Args:
            algorithm_name (str): The name of the algorithm to use
            public_key (bytes): The public key to use for verification
            data (bytes): The data that was signed
            signature (bytes): The signature to verify

        Returns:
            bool: True if the signature is valid

        Raises:
            SatoxError: If the system is not initialized or verification fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name or not public_key or not data or not signature:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._algorithms.verify(algorithm_name, public_key, data, signature)

    def encrypt(self, algorithm_name: str, public_key: bytes, data: bytes) -> bytes:
        """
        Encrypt data using the specified algorithm and public key.

        Args:
            algorithm_name (str): The name of the algorithm to use
            public_key (bytes): The public key to use for encryption
            data (bytes): The data to encrypt

        Returns:
            bytes: The encrypted data

        Raises:
            SatoxError: If the system is not initialized or encryption fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name or not public_key or not data:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._algorithms.encrypt(algorithm_name, public_key, data)

    def decrypt(self, algorithm_name: str, private_key: bytes, encrypted_data: bytes) -> bytes:
        """
        Decrypt data using the specified algorithm and private key.

        Args:
            algorithm_name (str): The name of the algorithm to use
            private_key (bytes): The private key to use for decryption
            encrypted_data (bytes): The data to decrypt

        Returns:
            bytes: The decrypted data

        Raises:
            SatoxError: If the system is not initialized or decryption fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name or not private_key or not encrypted_data:
            raise SatoxError("Invalid input", "INVALID_INPUT")
        return self._algorithms.decrypt(algorithm_name, private_key, encrypted_data)

    def get_algorithm_info(self, algorithm_name: str) -> AlgorithmInfo:
        """
        Get information about a specific algorithm.

        Args:
            algorithm_name (str): The name of the algorithm

        Returns:
            AlgorithmInfo: Information about the algorithm

        Raises:
            SatoxError: If the system is not initialized or retrieval fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name:
            raise SatoxError("Invalid algorithm name", "INVALID_INPUT")
        return self._algorithms.get_algorithm_info(algorithm_name)

    def get_available_algorithms(self) -> List[str]:
        """
        Get a list of all available algorithms.

        Returns:
            List[str]: List of available algorithm names

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        return self._algorithms.get_available_algorithms()

    def get_recommended_algorithms(self) -> List[str]:
        """
        Get a list of recommended algorithms.

        Returns:
            List[str]: List of recommended algorithm names

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        return self._algorithms.get_recommended_algorithms()

    def is_algorithm_available(self, algorithm_name: str) -> bool:
        """
        Check if an algorithm is available.

        Args:
            algorithm_name (str): The name of the algorithm to check

        Returns:
            bool: True if the algorithm is available

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name:
            raise SatoxError("Invalid algorithm name", "INVALID_INPUT")
        return self._algorithms.is_algorithm_available(algorithm_name)

    def is_algorithm_recommended(self, algorithm_name: str) -> bool:
        """
        Check if an algorithm is recommended.

        Args:
            algorithm_name (str): The name of the algorithm to check

        Returns:
            bool: True if the algorithm is recommended

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name:
            raise SatoxError("Invalid algorithm name", "INVALID_INPUT")
        return self._algorithms.is_algorithm_recommended(algorithm_name)

    def get_algorithm_security_level(self, algorithm_name: str) -> int:
        """
        Get the security level of an algorithm.

        Args:
            algorithm_name (str): The name of the algorithm

        Returns:
            int: The security level

        Raises:
            SatoxError: If the system is not initialized or retrieval fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name:
            raise SatoxError("Invalid algorithm name", "INVALID_INPUT")
        return self._algorithms.get_algorithm_security_level(algorithm_name)

    def get_algorithm_key_size(self, algorithm_name: str) -> int:
        """
        Get the key size for an algorithm.

        Args:
            algorithm_name (str): The name of the algorithm

        Returns:
            int: The key size in bits

        Raises:
            SatoxError: If the system is not initialized or retrieval fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name:
            raise SatoxError("Invalid algorithm name", "INVALID_INPUT")
        return self._algorithms.get_algorithm_key_size(algorithm_name)

    def get_algorithm_signature_size(self, algorithm_name: str) -> int:
        """
        Get the signature size for an algorithm.

        Args:
            algorithm_name (str): The name of the algorithm

        Returns:
            int: The signature size in bytes

        Raises:
            SatoxError: If the system is not initialized or retrieval fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name:
            raise SatoxError("Invalid algorithm name", "INVALID_INPUT")
        return self._algorithms.get_algorithm_signature_size(algorithm_name)

    def is_initialized(self) -> bool:
        """
        Check if the post-quantum algorithms system is initialized.

        Returns:
            bool: True if initialized
        """
        return self._initialized

    def get_version(self) -> str:
        """
        Get the version of the post-quantum algorithms system.

        Returns:
            str: The version string
        """
        return self._algorithms.get_version()

    def get_default_algorithm(self) -> str:
        """
        Get the current default algorithm.

        Returns:
            str: The name of the default algorithm

        Raises:
            SatoxError: If the system is not initialized
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        return self._algorithms.get_default_algorithm()

    def set_default_algorithm(self, algorithm_name: str) -> bool:
        """
        Set the default algorithm.

        Args:
            algorithm_name (str): The name of the algorithm to set as default

        Returns:
            bool: True if setting was successful

        Raises:
            SatoxError: If the system is not initialized or setting fails
        """
        if not self._initialized:
            raise SatoxError("Post-Quantum Algorithms not initialized", "NOT_INITIALIZED")
        if not algorithm_name:
            raise SatoxError("Invalid algorithm name", "INVALID_INPUT")
        return self._algorithms.set_default_algorithm(algorithm_name) 