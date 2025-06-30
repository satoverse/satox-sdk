"""
Satox SDK - A Python library for interacting with the Satoxcoin blockchain network.
"""

from satox.core.satox_manager import SatoxManager
from satox.blockchain.blockchain_manager import BlockchainManager
from satox.transaction.transaction_manager import TransactionManager
from satox.asset.asset_manager import AssetManager
from satox.api.rest_api import RESTAPIClient
from satox.api.websocket_api import WebSocketClient
from satox.api.graphql_api import GraphQLClient
from satox.wallet.wallet import Wallet
from satox.wallet.wallet_manager import WalletManager
from satox.security.security_manager import SecurityManager

__version__ = "0.1.0"
__author__ = "Satox Team"
__license__ = "MIT"

# Export main classes
__all__ = [
    'SatoxManager',
    'BlockchainManager',
    'TransactionManager',
    'AssetManager',
    'RESTAPIClient',
    'WebSocketClient',
    'GraphQLClient',
    'Wallet',
    'WalletManager',
    'SecurityManager',
]
