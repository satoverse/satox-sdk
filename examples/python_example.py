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
Satox SDK Python Example Application
Demonstrates all major SDK features including blockchain operations, 
asset management, NFT operations, quantum security, and more.
"""

import asyncio
import json
import time
from typing import Dict, Any, List

# Import all SDK components
from satox_sdk import (
    CoreManager,
    BlockchainManager,
    SecurityManager,
    QuantumManager,
    WalletManager,
    AssetManager,
    NFTManager,
    TransactionManager,
    NetworkManager,
    DatabaseManager,
    APIManager,
    IPFSManager,
    SatoxError,
    AssetConfig,
    NFTConfig,
    TransactionConfig,
    BatchTransactionConfig,
    QuantumKeyPair
)


async def main():
    """Main example application demonstrating all SDK features."""
    print("🚀 Satox SDK Python Example Application")
    print("=======================================\n")

    try:
        # Initialize core manager
        print("1. Initializing Core Manager...")
        core_manager = CoreManager.get_instance()
        await core_manager.initialize()
        print("   ✅ Core Manager initialized successfully\n")

        # Initialize blockchain manager
        print("2. Initializing Blockchain Manager...")
        blockchain_manager = BlockchainManager.get_instance()
        await blockchain_manager.initialize()
        print("   ✅ Blockchain Manager initialized successfully\n")

        # Initialize security manager
        print("3. Initializing Security Manager...")
        security_manager = SecurityManager.get_instance()
        await security_manager.initialize()
        print("   ✅ Security Manager initialized successfully\n")

        # Initialize quantum manager
        print("4. Initializing Quantum Manager...")
        quantum_manager = QuantumManager.get_instance()
        await quantum_manager.initialize()
        print("   ✅ Quantum Manager initialized successfully\n")

        # Initialize wallet manager
        print("5. Initializing Wallet Manager...")
        wallet_manager = WalletManager.get_instance()
        await wallet_manager.initialize()
        print("   ✅ Wallet Manager initialized successfully\n")

        # Initialize asset manager
        print("6. Initializing Asset Manager...")
        asset_manager = AssetManager.get_instance()
        await asset_manager.initialize()
        print("   ✅ Asset Manager initialized successfully\n")

        # Initialize NFT manager
        print("7. Initializing NFT Manager...")
        nft_manager = NFTManager.get_instance()
        await nft_manager.initialize()
        print("   ✅ NFT Manager initialized successfully\n")

        # Initialize transaction manager
        print("8. Initializing Transaction Manager...")
        transaction_manager = TransactionManager.get_instance()
        await transaction_manager.initialize()
        print("   ✅ Transaction Manager initialized successfully\n")

        # Initialize network manager
        print("9. Initializing Network Manager...")
        network_manager = NetworkManager.get_instance()
        await network_manager.initialize()
        print("   ✅ Network Manager initialized successfully\n")

        # Initialize database manager
        print("10. Initializing Database Manager...")
        database_manager = DatabaseManager.get_instance()
        await database_manager.initialize()
        print("   ✅ Database Manager initialized successfully\n")

        # Initialize API manager
        print("11. Initializing API Manager...")
        api_manager = APIManager.get_instance()
        await api_manager.initialize()
        print("   ✅ API Manager initialized successfully\n")

        # Initialize IPFS manager
        print("12. Initializing IPFS Manager...")
        ipfs_manager = IPFSManager.get_instance()
        await ipfs_manager.initialize()
        print("   ✅ IPFS Manager initialized successfully\n")

        # Demonstrate wallet operations
        print("13. Wallet Operations Demo...")
        wallet_address = await wallet_manager.create_wallet()
        print(f"   ✅ Created wallet: {wallet_address}")

        balance = await wallet_manager.get_balance(wallet_address)
        print(f"   ✅ Wallet balance: {balance} SATOX")
        print()

        # Demonstrate quantum security
        print("14. Quantum Security Demo...")
        quantum_key_pair: QuantumKeyPair = await quantum_manager.generate_key_pair()
        print("   ✅ Generated quantum-resistant key pair")

        message = b"Hello Quantum World!"
        quantum_signature = await quantum_manager.sign(message, quantum_key_pair.private_key)
        print("   ✅ Created quantum-resistant signature")

        is_valid = await quantum_manager.verify(message, quantum_signature, quantum_key_pair.public_key)
        print(f"   ✅ Quantum signature verification: {is_valid}")
        print()

        # Demonstrate asset operations
        print("15. Asset Operations Demo...")
        asset_config = AssetConfig(
            name="Test Token",
            symbol="TEST",
            total_supply=1000000,
            decimals=8,
            description="A test token for demonstration",
            metadata={
                "website": "https://example.com",
                "category": "utility"
            }
        )

        asset_id = await asset_manager.create_asset(asset_config)
        print(f"   ✅ Created asset with ID: {asset_id}")

        asset_info = await asset_manager.get_asset_info(asset_id)
        print(f"   ✅ Asset info: {asset_info.name} ({asset_info.symbol})")
        print()

        # Demonstrate NFT operations
        print("16. NFT Operations Demo...")
        nft_config = NFTConfig(
            name="Test NFT",
            symbol="TNFT",
            description="A test NFT for demonstration",
            metadata={
                "image": "https://example.com/image.png",
                "attributes": {
                    "rarity": "common",
                    "power": 100
                }
            }
        )

        nft_id = await nft_manager.create_nft(nft_config)
        print(f"   ✅ Created NFT with ID: {nft_id}")

        nft_info = await nft_manager.get_nft_info(nft_id)
        print(f"   ✅ NFT info: {nft_info.name} ({nft_info.symbol})")
        print()

        # Demonstrate transaction operations
        print("17. Transaction Operations Demo...")
        tx_config = TransactionConfig(
            from_address=wallet_address,
            to_address="recipient_address",
            amount=1000,
            asset_id=asset_id,
            fee=100
        )

        tx_id = await transaction_manager.create_transaction(tx_config)
        print(f"   ✅ Created transaction with ID: {tx_id}")

        tx_status = await transaction_manager.get_transaction_status(tx_id)
        print(f"   ✅ Transaction status: {tx_status}")
        print()

        # Demonstrate blockchain operations
        print("18. Blockchain Operations Demo...")
        blockchain_info = await blockchain_manager.get_blockchain_info()
        print(f"   ✅ Blockchain: {blockchain_info.name} (Height: {blockchain_info.current_height})")

        latest_block = await blockchain_manager.get_latest_block()
        print(f"   ✅ Latest block: {latest_block.hash} with {len(latest_block.transactions)} transactions")
        print()

        # Demonstrate network operations
        print("19. Network Operations Demo...")
        network_info = await network_manager.get_network_info()
        print(f"   ✅ Network: {network_info.connections} connections")

        peers = await network_manager.get_peers()
        print(f"   ✅ Connected peers: {len(peers)}")
        print()

        # Demonstrate IPFS operations
        print("20. IPFS Operations Demo...")
        data = b"Hello IPFS World!"
        ipfs_hash = await ipfs_manager.upload_data(data)
        print(f"   ✅ Uploaded data to IPFS: {ipfs_hash}")

        retrieved_data = await ipfs_manager.download_data(ipfs_hash)
        print(f"   ✅ Retrieved data from IPFS: {len(retrieved_data)} bytes")
        print()

        # Demonstrate database operations
        print("21. Database Operations Demo...")
        db_info = await database_manager.get_database_info()
        print(f"   ✅ Database: {db_info.table_count} tables")

        db_stats = await database_manager.get_database_stats()
        print(f"   ✅ Database size: {db_stats.size_mb:.2f} MB")
        print()

        # Demonstrate API operations
        print("22. API Operations Demo...")
        api_info = await api_manager.get_api_info()
        print(f"   ✅ API: {api_info.endpoint_count} endpoints available")

        api_stats = await api_manager.get_api_stats()
        print(f"   ✅ API requests: {api_stats.total_requests} total")
        print()

        # Demonstrate security operations
        print("23. Security Operations Demo...")
        security_info = await security_manager.get_security_info()
        print(f"   ✅ Security: {security_info.algorithm_count} algorithms supported")

        encryption_key = await security_manager.generate_encryption_key()
        print(f"   ✅ Generated encryption key: {len(encryption_key)} bytes")
        print()

        # Demonstrate batch operations
        print("24. Batch Operations Demo...")
        batch_config = BatchTransactionConfig(
            transactions=[
                TransactionConfig(
                    from_address=wallet_address,
                    to_address="recipient1",
                    amount=100,
                    asset_id=asset_id,
                    fee=10
                ),
                TransactionConfig(
                    from_address=wallet_address,
                    to_address="recipient2",
                    amount=200,
                    asset_id=asset_id,
                    fee=10
                )
            ]
        )

        batch_id = await transaction_manager.create_batch_transaction(batch_config)
        print(f"   ✅ Created batch transaction with ID: {batch_id}")
        print()

        # Demonstrate error handling
        print("25. Error Handling Demo...")
        try:
            await transaction_manager.get_transaction_status("invalid_tx_id")
            print("   ❌ Unexpected success")
        except SatoxError as e:
            print(f"   ✅ Properly handled error: {e}")
        print()

        # Demonstrate concurrent operations
        print("26. Concurrent Operations Demo...")
        async def create_wallet():
            wm = WalletManager.get_instance()
            await wm.initialize()
            address = await wm.create_wallet()
            await wm.shutdown()
            return address

        wallet_tasks = [create_wallet() for _ in range(3)]
        addresses = await asyncio.gather(*wallet_tasks)
        print(f"   ✅ Concurrent wallets created: {', '.join(addresses)}")
        print()

        # Demonstrate event handling
        print("27. Event Handling Demo...")
        
        async def on_transaction_status_changed(event):
            print(f"   📡 Transaction {event.transaction_id} status changed to {event.status}")

        transaction_manager.on_transaction_status_changed(on_transaction_status_changed)
        print("   ✅ Event handler registered")
        print()

        # Demonstrate async context managers
        print("28. Async Context Managers Demo...")
        async with asset_manager as am:
            asset_list = await am.list_assets()
            print(f"   ✅ Listed {len(asset_list)} assets using context manager")

        async with nft_manager as nm:
            nft_list = await nm.list_nfts()
            print(f"   ✅ Listed {len(nft_list)} NFTs using context manager")
        print()

        # Demonstrate performance testing
        print("29. Performance Testing Demo...")
        start_time = time.time()
        
        # Create 100 wallets concurrently
        wallet_tasks = [create_wallet() for _ in range(100)]
        wallet_addresses = await asyncio.gather(*wallet_tasks)
        
        end_time = time.time()
        print(f"   ⚡ Created 100 wallets in {end_time - start_time:.2f} seconds")
        print()

        # Demonstrate cleanup
        print("30. Cleanup Operations...")
        await api_manager.shutdown()
        await database_manager.shutdown()
        await ipfs_manager.shutdown()
        await network_manager.shutdown()
        await transaction_manager.shutdown()
        await nft_manager.shutdown()
        await asset_manager.shutdown()
        await wallet_manager.shutdown()
        await quantum_manager.shutdown()
        await security_manager.shutdown()
        await blockchain_manager.shutdown()
        await core_manager.shutdown()
        print("   ✅ All managers shut down successfully\n")

        print("🎉 Python Example Application Completed Successfully!")
        print("All SDK features demonstrated and working correctly.")

    except Exception as e:
        print(f"❌ Error in Python example: {e}")
        if isinstance(e, SatoxError):
            print(f"Satox Error: {e}")
        import traceback
        traceback.print_exc()


# Test functions for demonstration
async def test_core_initialization():
    """Test core manager initialization."""
    core_manager = CoreManager.get_instance()
    await core_manager.initialize()
    await core_manager.shutdown()


async def test_quantum_operations():
    """Test quantum operations."""
    quantum_manager = QuantumManager.get_instance()
    await quantum_manager.initialize()

    key_pair = await quantum_manager.generate_key_pair()
    message = b"Test message"
    signature = await quantum_manager.sign(message, key_pair.private_key)
    is_valid = await quantum_manager.verify(message, signature, key_pair.public_key)

    if not is_valid:
        raise SatoxError("Quantum signature verification failed")

    await quantum_manager.shutdown()


async def test_asset_operations():
    """Test asset operations."""
    asset_manager = AssetManager.get_instance()
    await asset_manager.initialize()

    config = AssetConfig(
        name="Test Asset",
        symbol="TEST",
        total_supply=1000000,
        decimals=8,
        description="Test asset",
        metadata={}
    )

    asset_id = await asset_manager.create_asset(config)
    asset_info = await asset_manager.get_asset_info(asset_id)

    if asset_info.name != "Test Asset":
        raise SatoxError(f"Unexpected asset name: {asset_info.name}")

    await asset_manager.shutdown()


async def test_concurrent_operations():
    """Test concurrent operations."""
    async def create_wallet():
        wm = WalletManager.get_instance()
        await wm.initialize()
        address = await wm.create_wallet()
        await wm.shutdown()
        return address

    tasks = [create_wallet() for _ in range(5)]
    addresses = await asyncio.gather(*tasks)
    print(f"   ✅ Concurrent wallets created: {', '.join(addresses)}")


async def test_performance():
    """Test performance with multiple operations."""
    wallet_manager = WalletManager.get_instance()
    await wallet_manager.initialize()

    start_time = time.time()
    addresses = []

    for i in range(100):
        address = await wallet_manager.create_wallet()
        addresses.append(address)

    end_time = time.time()
    print(f"   ⚡ Created 100 wallets in {end_time - start_time:.2f} seconds")

    await wallet_manager.shutdown()


async def test_error_handling():
    """Test error handling scenarios."""
    try:
        # Test invalid transaction ID
        transaction_manager = TransactionManager.get_instance()
        await transaction_manager.initialize()
        
        await transaction_manager.get_transaction_status("invalid_tx_id")
        print("   ❌ Expected error but got success")
    except SatoxError as e:
        print(f"   ✅ Properly caught SatoxError: {e}")
    except Exception as e:
        print(f"   ❌ Unexpected error type: {type(e)}")
    finally:
        await transaction_manager.shutdown()


# Export for testing
__all__ = [
    'main',
    'test_core_initialization',
    'test_quantum_operations',
    'test_asset_operations',
    'test_concurrent_operations',
    'test_performance',
    'test_error_handling'
]


if __name__ == "__main__":
    # Run the main function
    asyncio.run(main()) 