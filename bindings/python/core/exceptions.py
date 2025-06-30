"""
Core exceptions for the Satox SDK.
"""

class SatoxError(Exception):
    """Base exception for all Satox SDK errors."""
    pass

class ConfigurationError(SatoxError):
    """Raised when there is a configuration error."""
    pass

class NetworkError(SatoxError):
    """Raised when there is a network-related error."""
    pass

class WalletError(SatoxError):
    """Base exception for wallet-related errors."""
    pass

class WalletNotFoundError(WalletError):
    """Raised when a wallet is not found."""
    pass

class WalletAlreadyExistsError(WalletError):
    """Raised when attempting to create a wallet that already exists."""
    pass

class InvalidWalletError(WalletError):
    """Raised when a wallet is invalid."""
    pass

class TransactionError(SatoxError):
    """Base exception for transaction-related errors."""
    pass

class InvalidTransactionError(TransactionError):
    """Raised when a transaction is invalid."""
    pass

class TransactionNotFoundError(TransactionError):
    """Raised when a transaction is not found."""
    pass

class TransactionFailedError(TransactionError):
    """Raised when a transaction fails."""
    pass

class BlockchainError(SatoxError):
    """Base exception for blockchain-related errors."""
    pass

class BlockNotFoundError(BlockchainError):
    """Raised when a block is not found."""
    pass

class InvalidBlockError(BlockchainError):
    """Raised when a block is invalid."""
    pass

class AssetError(SatoxError):
    """Base exception for asset-related errors."""
    pass

class AssetNotFoundError(AssetError):
    """Raised when an asset is not found."""
    pass

class InvalidAssetError(AssetError):
    """Raised when an asset is invalid."""
    pass

class SecurityError(SatoxError):
    """Base exception for security-related errors."""
    pass

class EncryptionError(SecurityError):
    """Raised when there is an encryption error."""
    pass

class DecryptionError(SecurityError):
    """Raised when there is a decryption error."""
    pass

class AuthenticationError(SecurityError):
    """Raised when there is an authentication error."""
    pass

class AuthorizationError(SecurityError):
    """Raised when there is an authorization error."""
    pass

class ValidationError(SatoxError):
    """Raised when validation fails."""
    pass

class ResourceError(SatoxError):
    """Base exception for resource-related errors."""
    pass

class ResourceNotFoundError(ResourceError):
    """Raised when a resource is not found."""
    pass

class ResourceAlreadyExistsError(ResourceError):
    """Raised when a resource already exists."""
    pass

class ResourceLimitError(ResourceError):
    """Raised when a resource limit is exceeded."""
    pass

class TimeoutError(SatoxError):
    """Raised when an operation times out."""
    pass

class RetryError(SatoxError):
    """Raised when all retry attempts fail."""
    pass

class CacheError(SatoxError):
    """Base exception for cache-related errors."""
    pass

class CacheMissError(CacheError):
    """Raised when a cache lookup fails."""
    pass

class CacheFullError(CacheError):
    """Raised when the cache is full."""
    pass

class SyncError(SatoxError):
    """Base exception for synchronization-related errors."""
    pass

class SyncTimeoutError(SyncError):
    """Raised when synchronization times out."""
    pass

class SyncConflictError(SyncError):
    """Raised when there is a synchronization conflict."""
    pass

class SyncFailedError(SyncError):
    """Raised when synchronization fails."""
    pass 