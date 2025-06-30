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
Event Monitoring Example

This script demonstrates how to:
1. Monitor blockchain events
2. Handle different event types
3. Process event data
4. Implement event filters
"""

from satox.blockchain import BlockchainClient
from satox.asset import Asset, NFT
from satox.wallet import Wallet
from satox.exceptions import BlockchainException
import time
from typing import Dict, Any
import json

class EventMonitor:
    def __init__(self):
        self.blockchain_client = BlockchainClient()
        self.wallet = Wallet.create()
        self.token = None
        self.nft = None

    def on_transaction(self, tx: Dict[str, Any]):
        """Handle transaction events"""
        print("\nTransaction Event:")
        print(f"Hash: {tx['hash']}")
        print(f"From: {tx['from']}")
        print(f"To: {tx['to']}")
        print(f"Value: {tx['value']} SATOX")
        print(f"Block Number: {tx['blockNumber']}")

    def on_token_transfer(self, event: Dict[str, Any]):
        """Handle token transfer events"""
        print("\nToken Transfer Event:")
        print(f"Token: {event['token']}")
        print(f"From: {event['from']}")
        print(f"To: {event['to']}")
        print(f"Amount: {event['amount']}")
        print(f"Transaction Hash: {event['transactionHash']}")

    def on_nft_transfer(self, event: Dict[str, Any]):
        """Handle NFT transfer events"""
        print("\nNFT Transfer Event:")
        print(f"Token ID: {event['tokenId']}")
        print(f"From: {event['from']}")
        print(f"To: {event['to']}")
        print(f"Transaction Hash: {event['transactionHash']}")

    def on_block(self, block: Dict[str, Any]):
        """Handle new block events"""
        print("\nNew Block Event:")
        print(f"Block Number: {block['number']}")
        print(f"Block Hash: {block['hash']}")
        print(f"Timestamp: {block['timestamp']}")
        print(f"Transaction Count: {len(block['transactions'])}")

    def setup_monitoring(self):
        """Set up event monitoring"""
        try:
            # Monitor transactions for wallet
            print("\nSetting up transaction monitoring...")
            self.blockchain_client.monitor_transactions(
                address=self.wallet.address,
                callback=self.on_transaction
            )

            # Create and monitor token
            print("\nSetting up token monitoring...")
            self.token = Asset(
                name="ExampleToken",
                symbol="EXT",
                decimals=18
            )
            self.token.monitor_transfers(callback=self.on_token_transfer)

            # Create and monitor NFT
            print("\nSetting up NFT monitoring...")
            self.nft = NFT(
                name="ExampleNFT",
                symbol="ENFT"
            )
            self.nft.monitor_transfers(callback=self.on_nft_transfer)

            # Monitor new blocks
            print("\nSetting up block monitoring...")
            self.blockchain_client.monitor_blocks(callback=self.on_block)

            # Set up event filters
            print("\nSetting up event filters...")
            filters = {
                "min_value": 1000000,  # 0.001 SATOX
                "max_value": 10000000,  # 0.01 SATOX
                "block_range": 100  # Last 100 blocks
            }
            self.blockchain_client.set_event_filters(filters)

        except BlockchainException as e:
            print(f"Blockchain error: {e}")
        except Exception as e:
            print(f"Unexpected error: {e}")

    def run(self):
        """Run the event monitor"""
        print("Starting event monitoring...")
        print("Press Ctrl+C to stop")
        
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\nStopping event monitoring...")
            self.cleanup()

    def cleanup(self):
        """Clean up monitoring resources"""
        try:
            self.blockchain_client.stop_monitoring()
            print("Monitoring stopped")
        except Exception as e:
            print(f"Error during cleanup: {e}")

def main():
    monitor = EventMonitor()
    monitor.setup_monitoring()
    monitor.run()

if __name__ == "__main__":
    main() 