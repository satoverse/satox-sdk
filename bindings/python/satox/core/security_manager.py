"""
Security Manager Python Bindings
"""

from typing import Dict, List, Optional, Any, Tuple
import satox_cpp

class SecurityManager:
    """Python wrapper for the C++ SecurityManager class."""
    
    def __init__(self):
        """Initialize the Security Manager."""
        self._manager = satox_cpp.SecurityManager()
        self._initialized = False
    
    def initialize(self) -> bool:
        """Initialize the security manager.
        
        Returns:
            bool: True if initialization was successful, False otherwise.
        """
        if not self._initialized:
            self._initialized = self._manager.initialize()
        return self._initialized
    
    def shutdown(self) -> bool:
        """Shutdown the security manager.
        
        Returns:
            bool: True if shutdown was successful, False otherwise.
        """
        if self._initialized:
            self._initialized = not self._manager.shutdown()
        return not self._initialized
    
    def generate_key_pair(self) -> Tuple[bytes, bytes]:
        """Generate a new key pair.
        
        Returns:
            Tuple[bytes, bytes]: A tuple containing (private_key, public_key)
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        return self._manager.generate_key_pair()
    
    def sign_message(self, private_key: bytes, message: bytes) -> bytes:
        """Sign a message with a private key.
        
        Args:
            private_key (bytes): The private key to use for signing
            message (bytes): The message to sign
            
        Returns:
            bytes: The signature
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        if not private_key or not message:
            raise ValueError("Invalid input")
        
        return self._manager.sign_message(private_key, message)
    
    def verify_signature(self, public_key: bytes, message: bytes, signature: bytes) -> bool:
        """Verify a message signature.
        
        Args:
            public_key (bytes): The public key to use for verification
            message (bytes): The original message
            signature (bytes): The signature to verify
            
        Returns:
            bool: True if the signature is valid, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        if not public_key or not message or not signature:
            raise ValueError("Invalid input")
        
        return self._manager.verify_signature(public_key, message, signature)
    
    def encrypt_data(self, public_key: bytes, data: bytes) -> bytes:
        """Encrypt data using a public key.
        
        Args:
            public_key (bytes): The public key to use for encryption
            data (bytes): The data to encrypt
            
        Returns:
            bytes: The encrypted data
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        if not public_key or not data:
            raise ValueError("Invalid input")
        
        return self._manager.encrypt_data(public_key, data)
    
    def decrypt_data(self, private_key: bytes, encrypted_data: bytes) -> bytes:
        """Decrypt data using a private key.
        
        Args:
            private_key (bytes): The private key to use for decryption
            encrypted_data (bytes): The data to decrypt
            
        Returns:
            bytes: The decrypted data
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        if not private_key or not encrypted_data:
            raise ValueError("Invalid input")
        
        return self._manager.decrypt_data(private_key, encrypted_data)
    
    def hash_data(self, data: bytes) -> bytes:
        """Hash data using the configured hash algorithm.
        
        Args:
            data (bytes): The data to hash
            
        Returns:
            bytes: The hash of the data
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        if not data:
            raise ValueError("Invalid input")
        
        return self._manager.hash_data(data)
    
    def verify_hash(self, data: bytes, hash_value: bytes) -> bool:
        """Verify a hash value against data.
        
        Args:
            data (bytes): The original data
            hash_value (bytes): The hash value to verify
            
        Returns:
            bool: True if the hash is valid, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        if not data or not hash_value:
            raise ValueError("Invalid input")
        
        return self._manager.verify_hash(data, hash_value)
    
    def generate_random_bytes(self, length: int) -> bytes:
        """Generate random bytes.
        
        Args:
            length (int): The number of bytes to generate
            
        Returns:
            bytes: The generated random bytes
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        if length <= 0:
            raise ValueError("Invalid length")
        
        return self._manager.generate_random_bytes(length)
    
    def is_initialized(self) -> bool:
        """Check if the security manager is initialized.
        
        Returns:
            bool: True if initialized, False otherwise.
        """
        return self._initialized
    
    def get_version(self) -> str:
        """Get the version of the security manager.
        
        Returns:
            str: The version string
        """
        return self._manager.get_version()
    
    def get_security_level(self) -> str:
        """Get the current security level.
        
        Returns:
            str: The security level
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        return self._manager.get_security_level()
    
    def get_available_algorithms(self) -> List[str]:
        """Get the list of available security algorithms.
        
        Returns:
            List[str]: List of algorithm names
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        return self._manager.get_available_algorithms()
    
    def get_algorithm_info(self, algorithm: str) -> Optional[Dict[str, Any]]:
        """Get information about a specific algorithm.
        
        Args:
            algorithm (str): The name of the algorithm
            
        Returns:
            Optional[Dict[str, Any]]: Algorithm information, or None if not found
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Security Manager not initialized")
        
        if not algorithm:
            raise ValueError("Invalid algorithm name")
        
        return self._manager.get_algorithm_info(algorithm) 