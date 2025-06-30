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
Basic example demonstrating wallet creation and management using the Satox SDK.
This example shows how to:
1. Create a new wallet
2. Connect to the blockchain
3. Check wallet balance
4. Create and send a transaction
5. Monitor transaction status
"""

import asyncio
import logging
from pathlib import Path
from typing import Optional

from satox.wallet import WalletManager
from satox.blockchain import BlockchainManager
from satox.core.error_handler import ErrorHandler, handle_errors

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class WalletExample:
    def __init__(self):
        self.error_handler = ErrorHandler()
        self.wallet_manager = WalletManager()
        self.blockchain_manager = BlockchainManager()
        self.wallet_path = Path("wallet.json")
        self.wallet: Optional[WalletManager] = None

    @handle_errors
    async def setup(self):
        """Initialize the example environment."""
        logger.info("Setting up wallet example...")
        
        # Initialize blockchain connection
        await self.blockchain_manager.connect()
        logger.info("Connected to blockchain network")

        # Create or load wallet
        if self.wallet_path.exists():
            self.wallet = await self.wallet_manager.load_wallet(str(self.wallet_path))
            logger.info(f"Loaded existing wallet: {self.wallet.address}")
        else:
            self.wallet = await self.wallet_manager.create_wallet()
            await self.wallet.save(str(self.wallet_path))
            logger.info(f"Created new wallet: {self.wallet.address}")

    @handle_errors
    async def check_balance(self):
        """Check wallet balance."""
        if not self.wallet:
            raise ValueError("Wallet not initialized")

        balance = await self.blockchain_manager.get_balance(self.wallet.address)
        logger.info(f"Wallet balance: {balance} SATOX")
        return balance

    @handle_errors
    async def create_transaction(self, recipient: str, amount: float):
        """Create and send a transaction."""
        if not self.wallet:
            raise ValueError("Wallet not initialized")

        # Create transaction
        tx = await self.wallet.create_transaction(
            recipient=recipient,
            amount=amount,
            fee=0.001  # 0.001 SATOX fee
        )

        # Sign transaction
        signed_tx = await self.wallet.sign_transaction(tx)

        # Broadcast transaction
        tx_hash = await self.blockchain_manager.broadcast_transaction(signed_tx)
        logger.info(f"Transaction sent: {tx_hash}")

        # Monitor transaction status
        while True:
            status = await self.blockchain_manager.get_transaction_status(tx_hash)
            if status == "confirmed":
                logger.info("Transaction confirmed!")
                break
            elif status == "failed":
                logger.error("Transaction failed!")
                break
            await asyncio.sleep(1)

        return tx_hash

    @handle_errors
    async def cleanup(self):
        """Clean up resources."""
        await self.blockchain_manager.disconnect()
        logger.info("Disconnected from blockchain network")

async def main():
    example = WalletExample()
    
    try:
        # Setup
        await example.setup()
        
        # Check initial balance
        initial_balance = await example.check_balance()
        
        # Create and send transaction (if you have a recipient address)
        # recipient = "recipient_address_here"
        # amount = 1.0
        # tx_hash = await example.create_transaction(recipient, amount)
        
        # Check final balance
        final_balance = await example.check_balance()
        
    except Exception as e:
        logger.error(f"Error in example: {e}")
    finally:
        await example.cleanup()

if __name__ == "__main__":
    asyncio.run(main()) 