"""Exception classes for Satox SDK.
Defines custom exceptions used throughout the SDK.
"""

class SatoxError(Exception):
    """Base exception class for all Satox SDK errors."""
    
    def __init__(self, message: str = "", error_code: int = 0):
        """Initialize a new SatoxError.
        
        Args:
            message: Error message
            error_code: Numeric error code
        """
        self.error_code = error_code
        super().__init__(message)


class SatoxCryptoError(SatoxError):
    """Exception raised for cryptographic errors."""
    
    def __init__(self, message: str = "", error_code: int = 1000):
        """Initialize a new SatoxCryptoError.
        
        Args:
            message: Error message
            error_code: Numeric error code (default: 1000)
        """
        super().__init__(message, error_code)


class SatoxNetworkError(SatoxError):
    """Exception raised for network-related errors."""
    
    def __init__(self, message: str = "", error_code: int = 2000):
        """Initialize a new SatoxNetworkError.
        
        Args:
            message: Error message
            error_code: Numeric error code (default: 2000)
        """
        super().__init__(message, error_code)


class SatoxBlockchainError(SatoxError):
    """Exception raised for blockchain-related errors."""
    
    def __init__(self, message: str = "", error_code: int = 3000):
        """Initialize a new SatoxBlockchainError.
        
        Args:
            message: Error message
            error_code: Numeric error code (default: 3000)
        """
        super().__init__(message, error_code)


class SatoxTransactionError(SatoxError):
    """Exception raised for transaction-related errors."""
    
    def __init__(self, message: str = "", error_code: int = 4000):
        """Initialize a new SatoxTransactionError.
        
        Args:
            message: Error message
            error_code: Numeric error code (default: 4000)
        """
        super().__init__(message, error_code)


class SatoxIOError(SatoxError):
    """Exception raised for I/O errors."""
    
    def __init__(self, message: str = "", error_code: int = 5000):
        """Initialize a new SatoxIOError.
        
        Args:
            message: Error message
            error_code: Numeric error code (default: 5000)
        """
        super().__init__(message, error_code) 