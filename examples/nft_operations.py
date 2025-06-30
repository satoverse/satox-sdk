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
NFT Operations Example

This script demonstrates how to:
1. Create and deploy NFT contracts
2. Mint NFTs
3. Transfer NFTs
4. Query NFT metadata
"""

from satox.asset import NFT
from satox.wallet import Wallet
from satox.blockchain import BlockchainClient
from satox.exceptions import AssetException
import json
import time

def create_metadata(name, description, image_url):
    """Create NFT metadata"""
    return {
        "name": name,
        "description": description,
        "image": image_url,
        "attributes": [
            {
                "trait_type": "Rarity",
                "value": "Common"
            }
        ]
    }

def main():
    # Initialize components
    blockchain_client = BlockchainClient()
    wallet = Wallet.create()

    try:
        # Create NFT contract
        print("\nCreating NFT contract...")
        nft = NFT(
            name="ExampleNFT",
            symbol="ENFT",
            metadata_uri="ipfs://..."
        )
        print(f"NFT contract deployed at: {nft.address}")

        # Create metadata for NFTs
        metadata1 = create_metadata(
            "NFT #1",
            "This is the first NFT",
            "ipfs://..."
        )
        metadata2 = create_metadata(
            "NFT #2",
            "This is the second NFT",
            "ipfs://..."
        )

        # Mint NFTs
        print("\nMinting NFTs...")
        token_id1 = nft.mint(
            to_address=wallet.address,
            metadata_uri=json.dumps(metadata1)
        )
        print(f"Minted NFT #1 with ID: {token_id1}")

        token_id2 = nft.mint(
            to_address=wallet.address,
            metadata_uri=json.dumps(metadata2)
        )
        print(f"Minted NFT #2 with ID: {token_id2}")

        # Create another wallet for transfers
        recipient = Wallet.create()
        print(f"\nRecipient Address: {recipient.address}")

        # Get NFT owner
        owner = nft.owner_of(token_id1)
        print(f"\nOwner of NFT #1: {owner}")

        # Transfer NFT
        print("\nTransferring NFT #1...")
        tx_hash = nft.transfer(
            to_address=recipient.address,
            token_id=token_id1,
            from_wallet=wallet
        )
        print(f"Transfer transaction hash: {tx_hash}")

        # Wait for transaction confirmation
        print("\nWaiting for transaction confirmation...")
        time.sleep(5)  # Wait for block confirmation

        # Check new owner
        new_owner = nft.owner_of(token_id1)
        print(f"\nNew owner of NFT #1: {new_owner}")

        # Get NFT metadata
        print("\nNFT Metadata:")
        metadata = nft.get_metadata(token_id1)
        print(json.dumps(metadata, indent=2))

        # Get all NFTs owned by an address
        print("\nNFTs owned by recipient:")
        nfts = nft.get_tokens_by_owner(recipient.address)
        for token_id in nfts:
            print(f"Token ID: {token_id}")

    except AssetException as e:
        print(f"Asset error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    main() 