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
Custom Contract Interaction Example

This script demonstrates how to:
1. Deploy custom contracts
2. Interact with contract methods
3. Handle contract events
4. Manage contract state
"""

from satox.contract import Contract
from satox.wallet import Wallet
from satox.blockchain import BlockchainClient
from satox.exceptions import ContractException
import json
import time
from typing import Dict, Any, List

class CustomContract:
    def __init__(self, contract_path: str):
        self.wallet = Wallet.create()
        self.blockchain_client = BlockchainClient()
        self.contract_path = contract_path
        self.contract = None

    def load_contract(self):
        """Load contract ABI and bytecode"""
        try:
            with open(self.contract_path, 'r') as f:
                contract_data = json.load(f)
            return contract_data['abi'], contract_data['bytecode']
        except Exception as e:
            print(f"Error loading contract: {e}")
            return None, None

    def deploy_contract(self, constructor_args: List[Any] = None):
        """Deploy the contract"""
        try:
            print("\nDeploying contract...")
            abi, bytecode = self.load_contract()
            if not abi or not bytecode:
                raise ContractException("Failed to load contract data")

            self.contract = Contract(
                abi=abi,
                bytecode=bytecode
            )

            # Deploy contract
            tx_hash = self.contract.deploy(
                from_wallet=self.wallet,
                constructor_args=constructor_args
            )
            print(f"Deployment transaction hash: {tx_hash}")

            # Wait for deployment
            receipt = self.contract.wait_for_deployment(tx_hash)
            print(f"Contract deployed at: {receipt.contract_address}")

            return receipt.contract_address

        except ContractException as e:
            print(f"Contract error: {e}")
            return None
        except Exception as e:
            print(f"Unexpected error: {e}")
            return None

    def call_contract_method(self, method_name: str, args: List[Any] = None):
        """Call a contract method"""
        try:
            print(f"\nCalling method: {method_name}")
            result = self.contract.call(
                method=method_name,
                args=args,
                from_wallet=self.wallet
            )
            print(f"Result: {result}")
            return result
        except ContractException as e:
            print(f"Contract error: {e}")
            return None

    def send_contract_transaction(self, method_name: str, args: List[Any] = None):
        """Send a transaction to the contract"""
        try:
            print(f"\nSending transaction to method: {method_name}")
            tx_hash = self.contract.send(
                method=method_name,
                args=args,
                from_wallet=self.wallet
            )
            print(f"Transaction hash: {tx_hash}")

            # Wait for transaction confirmation
            receipt = self.contract.wait_for_transaction(tx_hash)
            print(f"Transaction status: {'Success' if receipt.status else 'Failed'}")
            return receipt
        except ContractException as e:
            print(f"Contract error: {e}")
            return None

    def monitor_contract_events(self, event_name: str, callback):
        """Monitor contract events"""
        try:
            print(f"\nMonitoring events: {event_name}")
            self.contract.monitor_events(
                event_name=event_name,
                callback=callback
            )
        except ContractException as e:
            print(f"Contract error: {e}")

def on_event(event: Dict[str, Any]):
    """Handle contract events"""
    print("\nContract Event:")
    print(f"Event Name: {event['event']}")
    print(f"Transaction Hash: {event['transactionHash']}")
    print(f"Block Number: {event['blockNumber']}")
    print(f"Event Data: {event['args']}")

def main():
    # Initialize contract
    contract = CustomContract("contracts/ExampleContract.json")

    try:
        # Deploy contract
        contract_address = contract.deploy_contract(["Initial Value"])
        if not contract_address:
            return

        # Call view method
        value = contract.call_contract_method("getValue")
        print(f"Current value: {value}")

        # Send transaction to update value
        contract.send_contract_transaction("setValue", [42])

        # Call view method again
        new_value = contract.call_contract_method("getValue")
        print(f"New value: {new_value}")

        # Monitor events
        contract.monitor_contract_events("ValueChanged", on_event)

        # Send another transaction to trigger event
        contract.send_contract_transaction("setValue", [100])

        # Wait for event
        time.sleep(5)

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main() 