"""
Basic usage example for Satox SDK Python Bindings
"""

import asyncio
from satox_bindings import (
    CoreManager,
    DatabaseManager,
    BlockchainManager,
    AssetManager,
    WalletManager,
    DatabaseType,
    NetworkType,
    CoreConfig,
    DatabaseConfig,
    SatoxError
)

async def main():
    print("🚀 Satox SDK Python Example")
    print("============================")
    
    # Initialize core manager
    print("\n1. Initializing Core Manager...")
    core_manager = CoreManager()
    config = CoreConfig(
        name="satox_sdk_example",
        enable_logging=True,
        log_level="info",
        max_threads=4
    )
    
    try:
        await core_manager.initialize(config)
        print("✅ Core Manager initialized successfully")
        
        # Get status
        status = await core_manager.get_status()
        print(f"📊 SDK Status: {status}")
        
    except SatoxError as e:
        print(f"❌ Failed to initialize Core Manager: {e}")
        return
    
    # Initialize database manager
    print("\n2. Initializing Database Manager...")
    db_manager = DatabaseManager()
    db_config = DatabaseConfig(
        type=DatabaseType.SQLITE,
        connection_string=":memory:"
    )
    
    try:
        await db_manager.initialize(db_config)
        await db_manager.connect(DatabaseType.SQLITE, ":memory:")
        print("✅ Database Manager initialized and connected")
        
        # Execute a test query
        result = await db_manager.execute_query("SELECT 1 as test")
        print(f"📊 Test query result: {result}")
        
    except SatoxError as e:
        print(f"⚠️  Database Manager issue: {e}")
    
    # Initialize blockchain manager
    print("\n3. Initializing Blockchain Manager...")
    blockchain_manager = BlockchainManager()
    
    try:
        await blockchain_manager.initialize({})
        await blockchain_manager.connect(
            NetworkType.MAINNET,
            "http://localhost:7777",
            {"username": "user", "password": "pass"}
        )
        print("✅ Blockchain Manager initialized and connected")
        
        # Get block height
        height = await blockchain_manager.get_block_height()
        print(f"📊 Current block height: {height}")
        
    except SatoxError as e:
        print(f"⚠️  Blockchain Manager issue: {e}")
    
    # Initialize asset manager
    print("\n4. Initializing Asset Manager...")
    asset_manager = AssetManager()
    
    try:
        await asset_manager.initialize({})
        print("✅ Asset Manager initialized")
        
        # Create a test asset
        asset_config = {
            "name": "Test Token",
            "symbol": "TEST",
            "description": "A test token created with Satox SDK",
            "precision": 8,
            "reissuable": True,
            "total_supply": 1000000
        }
        
        asset = await asset_manager.create_asset(asset_config)
        print(f"✅ Asset created: {asset.name} ({asset.symbol})")
        
    except SatoxError as e:
        print(f"⚠️  Asset Manager issue: {e}")
    
    # Initialize wallet manager
    print("\n5. Initializing Wallet Manager...")
    wallet_manager = WalletManager()
    
    try:
        await wallet_manager.initialize({})
        print("✅ Wallet Manager initialized")
        
        # Create a wallet
        wallet = await wallet_manager.create_wallet()
        print(f"✅ Wallet created: {wallet.address}")
        
    except SatoxError as e:
        print(f"⚠️  Wallet Manager issue: {e}")
    
    # Shutdown
    print("\n6. Shutting down...")
    try:
        await core_manager.shutdown()
        print("✅ SDK shutdown successfully")
    except SatoxError as e:
        print(f"⚠️  Shutdown issue: {e}")
    
    print("\n🎉 Example completed successfully!")

if __name__ == "__main__":
    asyncio.run(main()) 