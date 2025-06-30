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
Basic example demonstrating asset management using the Satox SDK.
This example shows how to:
1. Create a new asset
2. Transfer assets
3. Query asset information
4. Manage asset metadata
"""

import asyncio
import logging
from pathlib import Path
from typing import Optional, Dict, Any

from satox.wallet import WalletManager
from satox.blockchain import BlockchainManager
from satox.assets import AssetManager
from satox.core.error_handler import ErrorHandler, handle_errors

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class AssetExample:
    def __init__(self):
        self.error_handler = ErrorHandler()
        self.wallet_manager = WalletManager()
        self.blockchain_manager = BlockchainManager()
        self.asset_manager = AssetManager()
        self.wallet_path = Path("wallet.json")
        self.wallet: Optional[WalletManager] = None

    @handle_errors
    async def setup(self):
        """Initialize the example environment."""
        logger.info("Setting up asset example...")
        
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
    async def create_asset(self, name: str, symbol: str, total_supply: float, metadata: Dict[str, Any]):
        """Create a new asset."""
        if not self.wallet:
            raise ValueError("Wallet not initialized")

        # Create asset
        asset = await self.asset_manager.create_asset(
            name=name,
            symbol=symbol,
            total_supply=total_supply,
            metadata=metadata,
            owner=self.wallet.address
        )

        logger.info(f"Created new asset: {asset.symbol} ({asset.asset_id})")
        return asset

    @handle_errors
    async def transfer_asset(self, asset_id: str, recipient: str, amount: float):
        """Transfer assets to another address."""
        if not self.wallet:
            raise ValueError("Wallet not initialized")

        # Create transfer transaction
        tx = await self.asset_manager.create_transfer(
            asset_id=asset_id,
            sender=self.wallet.address,
            recipient=recipient,
            amount=amount
        )

        # Sign transaction
        signed_tx = await self.wallet.sign_transaction(tx)

        # Broadcast transaction
        tx_hash = await self.blockchain_manager.broadcast_transaction(signed_tx)
        logger.info(f"Asset transfer sent: {tx_hash}")

        # Monitor transaction status
        while True:
            status = await self.blockchain_manager.get_transaction_status(tx_hash)
            if status == "confirmed":
                logger.info("Asset transfer confirmed!")
                break
            elif status == "failed":
                logger.error("Asset transfer failed!")
                break
            await asyncio.sleep(1)

        return tx_hash

    @handle_errors
    async def get_asset_info(self, asset_id: str):
        """Get asset information."""
        asset = await self.asset_manager.get_asset(asset_id)
        logger.info(f"Asset information: {asset}")
        return asset

    @handle_errors
    async def update_asset_metadata(self, asset_id: str, metadata: Dict[str, Any]):
        """Update asset metadata."""
        if not self.wallet:
            raise ValueError("Wallet not initialized")

        # Create update transaction
        tx = await self.asset_manager.create_metadata_update(
            asset_id=asset_id,
            owner=self.wallet.address,
            metadata=metadata
        )

        # Sign transaction
        signed_tx = await self.wallet.sign_transaction(tx)

        # Broadcast transaction
        tx_hash = await self.blockchain_manager.broadcast_transaction(signed_tx)
        logger.info(f"Metadata update sent: {tx_hash}")

        # Monitor transaction status
        while True:
            status = await self.blockchain_manager.get_transaction_status(tx_hash)
            if status == "confirmed":
                logger.info("Metadata update confirmed!")
                break
            elif status == "failed":
                logger.error("Metadata update failed!")
                break
            await asyncio.sleep(1)

        return tx_hash

    @handle_errors
    async def cleanup(self):
        """Clean up resources."""
        await self.blockchain_manager.disconnect()
        logger.info("Disconnected from blockchain network")

async def main():
    example = AssetExample()
    
    try:
        # Setup
        await example.setup()
        
        # Create a new asset
        asset = await example.create_asset(
            name="Example Token",
            symbol="EXT",
            total_supply=1000000.0,
            metadata={
                "description": "An example token for demonstration",
                "website": "https://example.com",
                "version": "1.0.0"
            }
        )
        
        # Get asset information
        asset_info = await example.get_asset_info(asset.asset_id)
        
        # Transfer assets (if you have a recipient address)
        # recipient = "recipient_address_here"
        # amount = 100.0
        # tx_hash = await example.transfer_asset(asset.asset_id, recipient, amount)
        
        # Update asset metadata
        new_metadata = {
            "description": "Updated example token description",
            "website": "https://example.com",
            "version": "1.0.1",
            "new_field": "New metadata field"
        }
        await example.update_asset_metadata(asset.asset_id, new_metadata)
        
    except Exception as e:
        logger.error(f"Error in example: {e}")
    finally:
        await example.cleanup()

if __name__ == "__main__":
    asyncio.run(main()) 