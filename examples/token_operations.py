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
Token Operations Example

This script demonstrates how to:
1. Create and deploy tokens
2. Transfer tokens
3. Check token balances
4. Monitor token transfers
"""

from satox.asset import Asset, AssetManager
from satox.wallet import Wallet
from satox.blockchain import BlockchainClient
from satox.exceptions import AssetException
import time

def on_transfer(event):
    """Callback function for token transfer events"""
    print(f"\nToken Transfer Event:")
    print(f"From: {event.from_address}")
    print(f"To: {event.to_address}")
    print(f"Amount: {event.amount}")

def main():
    # Initialize components
    asset_manager = AssetManager()
    blockchain_client = BlockchainClient()
    wallet = Wallet.create()

    try:
        # Create and deploy a new token
        print("\nCreating new token...")
        token = asset_manager.create_asset(
            name="ExampleToken",
            symbol="EXT",
            decimals=18,
            total_supply=1000000
        )
        print(f"Token deployed at: {token.address}")

        # Get token information
        print("\nToken Information:")
        print(f"Name: {token.name}")
        print(f"Symbol: {token.symbol}")
        print(f"Decimals: {token.decimals}")
        print(f"Total Supply: {token.total_supply}")

        # Create another wallet for transfers
        recipient = Wallet.create()
        print(f"\nRecipient Address: {recipient.address}")

        # Transfer tokens
        amount = 1000
        print(f"\nTransferring {amount} tokens...")
        tx_hash = token.transfer(
            to_address=recipient.address,
            amount=amount,
            from_wallet=wallet
        )
        print(f"Transfer transaction hash: {tx_hash}")

        # Wait for transaction confirmation
        print("\nWaiting for transaction confirmation...")
        time.sleep(5)  # Wait for block confirmation

        # Check balances
        sender_balance = token.get_balance(wallet.address)
        recipient_balance = token.get_balance(recipient.address)
        print("\nBalances after transfer:")
        print(f"Sender: {sender_balance} {token.symbol}")
        print(f"Recipient: {recipient_balance} {token.symbol}")

        # Start monitoring token transfers
        print("\nStarting token transfer monitoring...")
        token.monitor_transfers(callback=on_transfer)

        # Perform another transfer to demonstrate monitoring
        print("\nPerforming another transfer...")
        token.transfer(
            to_address=recipient.address,
            amount=500,
            from_wallet=wallet
        )

        # Wait for monitoring to catch the transfer
        time.sleep(5)

    except AssetException as e:
        print(f"Asset error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    main() 