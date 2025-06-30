"""
Satox SDK Python Bindings
Copyright (c) 2025 Satoxcoin Core Developers
MIT License

High-performance Python bindings for the Satox SDK with quantum-resistant security.
"""

import os
import sys
from typing import Dict, Any, Optional

# Add the build directory to the path for the compiled extension
build_dir = os.path.join(os.path.dirname(__file__), '..', 'build')
if os.path.exists(build_dir):
    sys.path.insert(0, build_dir)

try:
    from ._satox_sdk_python import *
    from ._satox_sdk_python import SDK, WalletManager, SecurityManager, AssetManager, NFTManager
except ImportError:
    # Fallback for development or when extension is not built
    print("Warning: Satox SDK C++ extension not found. Using mock implementation.")
    from .mock import *

__version__ = "1.0.0"
__author__ = "Satoxcoin Core Developers"
__email__ = "dev@satoverse.io"
__license__ = "MIT"
__url__ = "https://github.com/satoverse/satox-sdk"

# Export main classes
__all__ = [
    'SDK',
    'WalletManager', 
    'SecurityManager',
    'AssetManager',
    'NFTManager',
    'DatabaseManager',
    'BlockchainManager',
    'IPFSManager',
    'NetworkManager',
    'CoreManager',
    'SatoxError',
    'SatoxConfigError',
    'SatoxSecurityError',
    'SatoxNetworkError',
    'SatoxDatabaseError',
    'SatoxWalletError',
    'SatoxAssetError',
    'SatoxNFTError',
    'SatoxIPFSError',
    'SatoxBlockchainError',
    'SatoxCoreError',
]

# Version info
def get_version() -> str:
    """Get the current version of the Satox SDK Python bindings."""
    return __version__

def get_core_version() -> str:
    """Get the version of the underlying C++ core library."""
    try:
        sdk = SDK()
        return sdk.get_version()
    except:
        return "unknown"

# Configuration helpers
def create_default_config() -> Dict[str, Any]:
    """Create a default configuration for the SDK."""
    return {
        "core": {
            "network": "testnet",
            "data_dir": os.path.expanduser("~/.satox"),
            "enable_mining": False,
            "enable_sync": True,
            "sync_interval_ms": 1000,
            "mining_threads": 1,
            "rpc_endpoint": "http://localhost:8332",
            "timeout_ms": 30000
        },
        "database": {
            "name": "satox_db",
            "enableLogging": True,
            "logPath": "logs/database",
            "maxConnections": 10,
            "connectionTimeout": 5000
        },
        "security": {
            "enablePQC": True,
            "enableInputValidation": True,
            "enableRateLimiting": True,
            "enableLogging": True,
            "logPath": "logs/security"
        },
        "wallet": {
            "enableLogging": True,
            "logPath": "logs/wallet"
        },
        "asset": {
            "enableLogging": True,
            "logPath": "logs/asset"
        },
        "nft": {
            "enableLogging": True,
            "logPath": "logs/nft"
        },
        "ipfs": {
            "enableLogging": True,
            "logPath": "logs/ipfs"
        },
        "blockchain": {
            "enableLogging": True,
            "logPath": "logs/blockchain"
        },
        "network": {
            "enableLogging": True,
            "logPath": "logs/network"
        }
    }

def create_mainnet_config() -> Dict[str, Any]:
    """Create a mainnet configuration for the SDK."""
    config = create_default_config()
    config["core"]["network"] = "mainnet"
    return config

def create_testnet_config() -> Dict[str, Any]:
    """Create a testnet configuration for the SDK."""
    config = create_default_config()
    config["core"]["network"] = "testnet"
    return config

# Utility functions
def is_initialized() -> bool:
    """Check if the SDK is currently initialized."""
    try:
        sdk = SDK()
        return sdk.is_initialized()
    except:
        return False

def get_sdk_instance() -> Optional['SDK']:
    """Get the current SDK instance if initialized."""
    try:
        return SDK.get_instance()
    except:
        return None

# Environment variable helpers
def load_config_from_env() -> Dict[str, Any]:
    """Load configuration from environment variables."""
    config = create_default_config()
    
    # Core settings
    if os.getenv('SATOX_NETWORK'):
        config['core']['network'] = os.getenv('SATOX_NETWORK')
    if os.getenv('SATOX_DATA_DIR'):
        config['core']['data_dir'] = os.getenv('SATOX_DATA_DIR')
    if os.getenv('SATOX_RPC_ENDPOINT'):
        config['core']['rpc_endpoint'] = os.getenv('SATOX_RPC_ENDPOINT')
    
    # Security settings
    if os.getenv('SATOX_ENABLE_PQC'):
        config['security']['enablePQC'] = os.getenv('SATOX_ENABLE_PQC').lower() == 'true'
    if os.getenv('SATOX_ENABLE_INPUT_VALIDATION'):
        config['security']['enableInputValidation'] = os.getenv('SATOX_ENABLE_INPUT_VALIDATION').lower() == 'true'
    
    return config

# Initialize logging directories
def _ensure_log_dirs():
    """Ensure log directories exist."""
    log_dirs = [
        "logs/components",
        "logs/bindings", 
        "logs/ci_cd",
        "logs/monitoring",
        "logs/database",
        "logs/security",
        "logs/wallet",
        "logs/asset",
        "logs/nft",
        "logs/ipfs",
        "logs/blockchain",
        "logs/network"
    ]
    
    for log_dir in log_dirs:
        os.makedirs(log_dir, exist_ok=True)

# Create log directories on import
_ensure_log_dirs()

# Export utility functions
__all__.extend([
    'get_version',
    'get_core_version', 
    'create_default_config',
    'create_mainnet_config',
    'create_testnet_config',
    'is_initialized',
    'get_sdk_instance',
    'load_config_from_env'
])
