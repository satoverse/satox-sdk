"""
MIT License

Copyright (c) 2025 Satoxcoin Core Developer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

"""
Satox SDK Python Bindings

This module provides Python bindings for the Satox SDK, allowing Python developers
to interact with the Satox blockchain and its various components.

Example usage:
    import satox_bindings as satox
    
    # Initialize the SDK
    sdk = satox.SDK.getInstance()
    sdk.initialize()
    
    # Use blockchain manager
    blockchain = sdk.getBlockchainManager()
    height = blockchain.getBlockHeight()
    
    # Use wallet manager
    wallet = sdk.getWalletManager()
    wallet.createWallet("my_wallet")
    
    # Shutdown
    sdk.shutdown()
"""

try:
    from ._core import (
        SDK,
        CoreManager,
        DatabaseManager,
        BlockchainManager,
        AssetManager,
        NFTManager,
        IPFSManager,
        NetworkManager,
        SecurityManager,
        WalletManager,
        # Configuration classes
        CoreConfig,
        DatabaseConfig,
        BlockchainConfig,
        AssetConfig,
        NFTConfig,
        IPFSConfig,
        NetworkConfig,
        SecurityConfig,
        WalletConfig,
        # Enums
        LogLevel,
        DatabaseType,
        NetworkType,
        SecurityLevel,
    )
except ImportError as e:
    raise ImportError(
        f"Failed to import Satox SDK native bindings: {e}\n"
        "Please ensure the native extension is built correctly."
    ) from e

# Version information
__version__ = "1.0.0"
__author__ = "Satoxcoin Core Developer"
__email__ = "team@satox.com"

# Export all public classes and enums
__all__ = [
    # Main SDK class
    "SDK",
    
    # Manager classes
    "CoreManager",
    "DatabaseManager", 
    "BlockchainManager",
    "AssetManager",
    "NFTManager",
    "IPFSManager",
    "NetworkManager",
    "SecurityManager",
    "WalletManager",
    
    # Configuration classes
    "CoreConfig",
    "DatabaseConfig",
    "BlockchainConfig",
    "AssetConfig",
    "NFTConfig",
    "IPFSConfig",
    "NetworkConfig",
    "SecurityConfig",
    "WalletConfig",
    
    # Enums
    "LogLevel",
    "DatabaseType",
    "NetworkType",
    "SecurityLevel",
]

# Convenience functions for common operations
def initialize_sdk(config=None):
    """
    Initialize the Satox SDK with optional configuration.
    
    Args:
        config (dict, optional): Configuration dictionary
        
    Returns:
        SDK: Initialized SDK instance
        
    Raises:
        RuntimeError: If initialization fails
    """
    sdk = SDK.getInstance()
    if not sdk.initialize(config or {}):
        raise RuntimeError("Failed to initialize Satox SDK")
    return sdk

def create_simple_wallet(wallet_name, sdk=None):
    """
    Create a simple wallet with default settings.
    
    Args:
        wallet_name (str): Name for the wallet
        sdk (SDK, optional): SDK instance. If None, will use getInstance()
        
    Returns:
        dict: Wallet information
        
    Raises:
        RuntimeError: If wallet creation fails
    """
    if sdk is None:
        sdk = SDK.getInstance()
    
    wallet = sdk.getWalletManager()
    if not wallet.createWallet(wallet_name):
        raise RuntimeError(f"Failed to create wallet: {wallet_name}")
    
    return wallet.getWalletInfo(wallet_name)

def get_blockchain_info(sdk=None):
    """
    Get basic blockchain information.
    
    Args:
        sdk (SDK, optional): SDK instance. If None, will use getInstance()
        
    Returns:
        dict: Blockchain information including height and best block hash
    """
    if sdk is None:
        sdk = SDK.getInstance()
    
    blockchain = sdk.getBlockchainManager()
    return {
        "height": blockchain.getBlockHeight(),
        "best_block_hash": blockchain.getBestBlockHash(),
        "is_healthy": blockchain.isHealthy(),
    }

def create_asset(asset_name, symbol, owner_address, sdk=None):
    """
    Create a new asset on the blockchain.
    
    Args:
        asset_name (str): Name of the asset
        symbol (str): Symbol/ticker for the asset
        owner_address (str): Owner's wallet address
        sdk (SDK, optional): SDK instance. If None, will use getInstance()
        
    Returns:
        dict: Asset information
        
    Raises:
        RuntimeError: If asset creation fails
    """
    if sdk is None:
        sdk = SDK.getInstance()
    
    asset = sdk.getAssetManager()
    
    # Create asset configuration
    config = {
        "name": asset_name,
        "symbol": symbol,
        "decimals": 8,
        "total_supply": 1000000,
        "description": f"Asset {asset_name} ({symbol})",
    }
    
    result = asset.createAsset(config, owner_address)
    if not result:
        raise RuntimeError(f"Failed to create asset: {asset_name}")
    
    return result

# Add convenience functions to __all__
__all__.extend([
    "initialize_sdk",
    "create_simple_wallet", 
    "get_blockchain_info",
    "create_asset",
])
