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
Batch Processing Example

This script demonstrates how to:
1. Process multiple transactions in batch
2. Monitor batch progress
3. Handle batch failures
4. Verify batch results
"""

from satox.transaction import Transaction, TransactionManager
from satox.wallet import Wallet
from satox.blockchain import BlockchainClient
from satox.exceptions import TransactionException
import time
from typing import List, Dict

def create_batch_transactions(
    wallet: Wallet,
    recipients: List[Dict[str, str]],
    amount: int
) -> List[Transaction]:
    """Create a batch of transactions"""
    return [
        Transaction(
            from_address=wallet.address,
            to_address=recipient["address"],
            value=amount,
            gas_limit=21000
        )
        for recipient in recipients
    ]

def main():
    # Initialize components
    transaction_manager = TransactionManager()
    blockchain_client = BlockchainClient()
    wallet = Wallet.create()

    try:
        # Create multiple recipient wallets
        print("\nCreating recipient wallets...")
        recipients = []
        for i in range(3):
            recipient = Wallet.create()
            recipients.append({
                "address": recipient.address,
                "name": f"Recipient {i+1}"
            })
            print(f"{recipients[i]['name']}: {recipients[i]['address']}")

        # Create batch of transactions
        print("\nCreating batch transactions...")
        amount = 1000000  # 0.001 SATOX
        transactions = create_batch_transactions(wallet, recipients, amount)
        print(f"Created {len(transactions)} transactions")

        # Sign all transactions
        print("\nSigning transactions...")
        signed_transactions = [tx.sign(wallet) for tx in transactions]

        # Send batch of transactions
        print("\nSending batch transactions...")
        tx_hashes = transaction_manager.send_batch(signed_transactions)
        print(f"Sent {len(tx_hashes)} transactions")

        # Monitor batch progress
        print("\nMonitoring batch progress...")
        for i, tx_hash in enumerate(tx_hashes):
            print(f"\nTransaction {i+1}:")
            print(f"Hash: {tx_hash}")
            print(f"Recipient: {recipients[i]['name']}")
            
            # Wait for confirmation
            receipt = transaction_manager.wait_for_transaction(tx_hash)
            print(f"Status: {'Success' if receipt.status else 'Failed'}")
            print(f"Block Number: {receipt.block_number}")

        # Verify final balances
        print("\nVerifying final balances...")
        for recipient in recipients:
            balance = blockchain_client.get_balance(recipient["address"])
            print(f"{recipient['name']}: {balance} SATOX")

        # Handle failed transactions
        print("\nHandling failed transactions...")
        failed_txs = transaction_manager.get_failed_transactions(tx_hashes)
        if failed_txs:
            print(f"Found {len(failed_txs)} failed transactions")
            for tx_hash in failed_txs:
                print(f"Failed transaction: {tx_hash}")
                # Retry failed transaction
                retry_tx = transaction_manager.retry_transaction(tx_hash)
                print(f"Retry transaction hash: {retry_tx}")

    except TransactionException as e:
        print(f"Transaction error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    main() 