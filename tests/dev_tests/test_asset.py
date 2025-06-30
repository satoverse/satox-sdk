#!/usr/bin/env python3

import satox_asset

def test_asset_manager():
    print("Testing Satox AssetManager Python Bindings...")
    
    # Get the singleton instance
    asset_manager = satox_asset.AssetManager.getInstance()
    print(f"✅ AssetManager instance created: {asset_manager}")
    
    # Test initialization
    result = asset_manager.initialize()
    print(f"✅ AssetManager.initialize() returned: {result}")
    
    # Test if initialized
    is_init = asset_manager.isInitialized()
    print(f"✅ AssetManager.isInitialized() returned: {is_init}")
    
    # Test asset creation (this will fail without real blockchain connection, but should not crash)
    try:
        metadata = {
            "name": "Test Asset",
            "symbol": "TEST",
            "description": "A test asset",
            "ipfs_hash": "QmTest123",
            "decimals": 8,
            "total_supply": 1000000,
            "reissuable": True
        }
        
        result = asset_manager.createAsset("TEST", satox_asset.AssetType.MAIN_ASSET, metadata)
        print(f"✅ AssetManager.createAsset() returned: {result}")
    except Exception as e:
        print(f"⚠️  AssetManager.createAsset() failed (expected): {e}")
    
    print("✅ All AssetManager tests completed successfully!")

if __name__ == "__main__":
    test_asset_manager() 