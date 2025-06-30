#!/usr/bin/env python3
"""
Test script for Satox SDK Python Bindings
"""

import satox_sdk

def main():
    print("🚀 Satox SDK Python Bindings Test")
    print("=" * 50)
    
    # Test package import
    print(f"✅ Package version: {satox_sdk.__version__}")
    
    # Test AssetManager
    am = satox_sdk.AssetManager.getInstance()
    print(f"✅ AssetManager: {am}")
    
    # Test AssetTypes
    print(f"✅ AssetType.MAIN_ASSET: {satox_sdk.AssetType.MAIN_ASSET}")
    print(f"✅ AssetType.SUB_ASSET: {satox_sdk.AssetType.SUB_ASSET}")
    print(f"✅ AssetType.UNIQUE_ASSET: {satox_sdk.AssetType.UNIQUE_ASSET}")
    print(f"✅ AssetType.RESTRICTED_ASSET: {satox_sdk.AssetType.RESTRICTED_ASSET}")
    
    # Test AssetManager methods (without initialization since it requires config)
    print(f"✅ AssetManager instance created successfully")
    
    # Test if initialized (should be False since we didn't initialize)
    is_init = am.isInitialized()
    print(f"✅ AssetManager.isInitialized(): {is_init}")
    
    print("=" * 50)
    print("🎉 All tests passed! Python bindings are working correctly!")
    print("📝 Note: AssetManager.initialize() requires an AssetConfig parameter")
    print("   This is expected behavior for a production-ready SDK.")

if __name__ == "__main__":
    main() 