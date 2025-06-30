"""
$(basename "$1")
$(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')

Copyright (c) 2025 Satoxcoin Core Developers
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

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

#!/usr/bin/env python3
"""
Advanced Error Handling Example

This script demonstrates how to:
1. Handle different types of exceptions
2. Implement retry mechanisms
3. Log errors properly
4. Provide user-friendly error messages
"""

from satox.blockchain import BlockchainClient
from satox.wallet import Wallet
from satox.asset import Asset
from satox.exceptions import (
    BlockchainException,
    WalletException,
    AssetException,
    TransactionException,
    NetworkException
)
import time
import logging
from typing import Optional, Any, Callable
from functools import wraps

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('error_handling.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

def retry(max_attempts: int = 3, delay: float = 1.0):
    """Decorator for retrying operations"""
    def decorator(func: Callable):
        @wraps(func)
        def wrapper(*args, **kwargs):
            attempts = 0
            while attempts < max_attempts:
                try:
                    return func(*args, **kwargs)
                except (NetworkException, BlockchainException) as e:
                    attempts += 1
                    if attempts == max_attempts:
                        logger.error(f"Operation failed after {max_attempts} attempts: {e}")
                        raise
                    logger.warning(f"Attempt {attempts} failed: {e}. Retrying in {delay} seconds...")
                    time.sleep(delay)
            return None
        return wrapper
    return decorator

class ErrorHandler:
    def __init__(self):
        self.blockchain_client = BlockchainClient()
        self.wallet = Wallet.create()
        self.token = None

    @retry(max_attempts=3, delay=2.0)
    def create_token(self, name: str, symbol: str) -> Optional[Asset]:
        """Create a new token with retry mechanism"""
        try:
            logger.info(f"Creating token {name} ({symbol})")
            self.token = Asset(
                name=name,
                symbol=symbol,
                decimals=18
            )
            return self.token
        except AssetException as e:
            logger.error(f"Failed to create token: {e}")
            raise

    def handle_transaction_error(self, error: Exception) -> str:
        """Handle transaction-related errors"""
        if isinstance(error, TransactionException):
            if "insufficient funds" in str(error).lower():
                return "Insufficient funds to complete the transaction"
            elif "gas price too low" in str(error).lower():
                return "Gas price is too low for the current network conditions"
            elif "nonce too low" in str(error).lower():
                return "Transaction nonce is invalid"
            else:
                return f"Transaction failed: {str(error)}"
        return f"Unexpected error: {str(error)}"

    def handle_wallet_error(self, error: Exception) -> str:
        """Handle wallet-related errors"""
        if isinstance(error, WalletException):
            if "invalid private key" in str(error).lower():
                return "Invalid private key format"
            elif "wallet not found" in str(error).lower():
                return "Wallet not found"
            else:
                return f"Wallet error: {str(error)}"
        return f"Unexpected error: {str(error)}"

    def handle_network_error(self, error: Exception) -> str:
        """Handle network-related errors"""
        if isinstance(error, NetworkException):
            if "connection refused" in str(error).lower():
                return "Unable to connect to the network"
            elif "timeout" in str(error).lower():
                return "Network request timed out"
            else:
                return f"Network error: {str(error)}"
        return f"Unexpected error: {str(error)}"

    def safe_transaction(self, operation: Callable, *args, **kwargs) -> Any:
        """Execute a transaction with comprehensive error handling"""
        try:
            return operation(*args, **kwargs)
        except TransactionException as e:
            error_msg = self.handle_transaction_error(e)
            logger.error(error_msg)
            raise TransactionException(error_msg)
        except WalletException as e:
            error_msg = self.handle_wallet_error(e)
            logger.error(error_msg)
            raise WalletException(error_msg)
        except NetworkException as e:
            error_msg = self.handle_network_error(e)
            logger.error(error_msg)
            raise NetworkException(error_msg)
        except Exception as e:
            logger.error(f"Unexpected error: {e}")
            raise

    def monitor_errors(self):
        """Monitor and log blockchain errors"""
        try:
            self.blockchain_client.monitor_errors(
                callback=lambda error: logger.error(f"Blockchain error: {error}")
            )
        except Exception as e:
            logger.error(f"Failed to set up error monitoring: {e}")

def main():
    handler = ErrorHandler()
    
    try:
        # Create token with retry mechanism
        token = handler.safe_transaction(
            handler.create_token,
            "ExampleToken",
            "EXT"
        )
        if not token:
            logger.error("Failed to create token")
            return

        # Monitor errors
        handler.monitor_errors()

        # Example of handling different error scenarios
        try:
            # Attempt invalid transaction
            handler.safe_transaction(
                token.transfer,
                "invalid_address",
                1000
            )
        except TransactionException as e:
            logger.error(f"Transaction error handled: {e}")

        try:
            # Attempt invalid wallet operation
            handler.safe_transaction(
                Wallet.import_from_private_key,
                "invalid_key"
            )
        except WalletException as e:
            logger.error(f"Wallet error handled: {e}")

    except Exception as e:
        logger.error(f"Main execution error: {e}")

if __name__ == "__main__":
    main() 