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
Wallet Management Example

This script demonstrates how to:
1. Create and manage wallets
2. Import wallets from private keys
3. Check balances
4. Backup and restore wallets
"""

from satox.wallet import Wallet, WalletManager
from satox.blockchain import BlockchainClient
from satox.exceptions import WalletException

def main():
    # Initialize components
    wallet_manager = WalletManager()
    blockchain_client = BlockchainClient()

    try:
        # Create a new wallet
        print("\nCreating new wallet...")
        wallet = wallet_manager.create_wallet()
        print(f"Address: {wallet.address}")
        print(f"Private Key: {wallet.private_key}")

        # Get wallet balance
        balance = blockchain_client.get_balance(wallet.address)
        print(f"Balance: {balance} SATOX")

        # Create multiple wallets
        print("\nCreating multiple wallets...")
        wallets = []
        for i in range(3):
            wallet = wallet_manager.create_wallet()
            wallets.append(wallet)
            print(f"Wallet {i+1}: {wallet.address}")

        # Import wallet from private key
        print("\nImporting wallet from private key...")
        private_key = "0x123..."  # Replace with actual private key
        imported_wallet = wallet_manager.import_wallet(private_key)
        print(f"Imported Address: {imported_wallet.address}")

        # Backup wallet
        print("\nBacking up wallet...")
        backup_data = wallet_manager.backup_wallet(wallet)
        print("Backup created successfully")

        # Restore wallet from backup
        print("\nRestoring wallet from backup...")
        restored_wallet = wallet_manager.restore_wallet(backup_data)
        print(f"Restored Address: {restored_wallet.address}")

        # List all wallets
        print("\nListing all wallets...")
        all_wallets = wallet_manager.list_wallets()
        for w in all_wallets:
            print(f"Address: {w.address}")

    except WalletException as e:
        print(f"Wallet error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    main() 