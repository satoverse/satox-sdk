"""
Error handling for Satox SDK
MIT License - Copyright (c) 2025 Satoxcoin Core Developer
"""

class SatoxError(Exception):
    """Base exception for Satox SDK errors"""
    
    def __init__(self, message: str, error_code=None):
        super().__init__(message)
        self.message = message
        self.error_code = error_code

class InitializationError(SatoxError):
    """Raised when initialization fails"""
    pass

class ConnectionError(SatoxError):
    """Raised when connection fails"""
    pass

class OperationError(SatoxError):
    """Raised when an operation fails"""
    pass

class ValidationError(SatoxError):
    """Raised when validation fails"""
    pass

class TimeoutError(SatoxError):
    """Raised when operation times out"""
    pass

class ResourceError(SatoxError):
    """Raised when resource allocation fails"""
    pass 