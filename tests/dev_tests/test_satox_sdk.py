#!/usr/bin/env python3
"""
Comprehensive test script for Satox SDK Python bindings
"""

import sys
import os

# Add the build directory to the path so we can import the modules
sys.path.insert(0, '../build/satox-asset')

def test_asset_manager():
    """Test AssetManager functionality"""
    print("🔧 Testing AssetManager...")
    
    try:
        import satox_asset
        
        # Get the singleton instance
        asset_manager = satox_asset.AssetManager.getInstance()
        print(f"   ✅ AssetManager instance created: {asset_manager}")
        
        # Test initialization
        result = asset_manager.initialize()
        print(f"   ✅ AssetManager.initialize() returned: {result}")
        
        # Test if initialized
        is_init = asset_manager.isInitialized()
        print(f"   ✅ AssetManager.isInitialized() returned: {is_init}")
        
        # Test asset types
        print(f"   ✅ AssetType.MAIN_ASSET: {satox_asset.AssetType.MAIN_ASSET}")
        print(f"   ✅ AssetType.SUB_ASSET: {satox_asset.AssetType.SUB_ASSET}")
        print(f"   ✅ AssetType.UNIQUE_ASSET: {satox_asset.AssetType.UNIQUE_ASSET}")
        print(f"   ✅ AssetType.RESTRICTED_ASSET: {satox_asset.AssetType.RESTRICTED_ASSET}")
        
        return True
        
    except ImportError as e:
        print(f"   ❌ AssetManager not available: {e}")
        return False
    except Exception as e:
        print(f"   ❌ AssetManager test failed: {e}")
        return False

def test_blockchain_manager():
    """Test BlockchainManager functionality"""
    print("🔧 Testing BlockchainManager...")
    
    try:
        import satox_blockchain
        
        # Create instance
        blockchain_manager = satox_blockchain.BlockchainManager()
        print(f"   ✅ BlockchainManager instance created: {blockchain_manager}")
        
        # Test initialization
        result = blockchain_manager.initialize()
        print(f"   ✅ BlockchainManager.initialize() returned: {result}")
        
        return True
        
    except ImportError as e:
        print(f"   ❌ BlockchainManager not available: {e}")
        return False
    except Exception as e:
        print(f"   ❌ BlockchainManager test failed: {e}")
        return False

def test_security_manager():
    """Test SecurityManager functionality"""
    print("🔧 Testing SecurityManager...")
    
    try:
        import satox_security
        
        # Create instance
        security_manager = satox_security.SecurityManager()
        print(f"   ✅ SecurityManager instance created: {security_manager}")
        
        # Test initialization
        result = security_manager.initialize()
        print(f"   ✅ SecurityManager.initialize() returned: {result}")
        
        return True
        
    except ImportError as e:
        print(f"   ❌ SecurityManager not available: {e}")
        return False
    except Exception as e:
        print(f"   ❌ SecurityManager test failed: {e}")
        return False

def test_nft_manager():
    """Test NFTManager functionality"""
    print("🔧 Testing NFTManager...")
    
    try:
        import satox_nft
        
        # Create instance
        nft_manager = satox_nft.NFTManager()
        print(f"   ✅ NFTManager instance created: {nft_manager}")
        
        # Test initialization
        result = nft_manager.initialize()
        print(f"   ✅ NFTManager.initialize() returned: {result}")
        
        return True
        
    except ImportError as e:
        print(f"   ❌ NFTManager not available: {e}")
        return False
    except Exception as e:
        print(f"   ❌ NFTManager test failed: {e}")
        return False

def test_ipfs_manager():
    """Test IPFSManager functionality"""
    print("🔧 Testing IPFSManager...")
    
    try:
        import satox_ipfs
        
        # Create instance
        ipfs_manager = satox_ipfs.IPFSManager()
        print(f"   ✅ IPFSManager instance created: {ipfs_manager}")
        
        # Test initialization
        result = ipfs_manager.initialize()
        print(f"   ✅ IPFSManager.initialize() returned: {result}")
        
        return True
        
    except ImportError as e:
        print(f"   ❌ IPFSManager not available: {e}")
        return False
    except Exception as e:
        print(f"   ❌ IPFSManager test failed: {e}")
        return False

def test_main_package():
    """Test the main satox_sdk package"""
    print("🔧 Testing main satox_sdk package...")
    
    try:
        import satox_sdk
        
        print(f"   ✅ satox_sdk imported successfully")
        print(f"   ✅ Version: {satox_sdk.__version__}")
        print(f"   ✅ AssetManager available: {satox_sdk.ASSET_AVAILABLE}")
        print(f"   ✅ BlockchainManager available: {satox_sdk.BLOCKCHAIN_AVAILABLE}")
        print(f"   ✅ SecurityManager available: {satox_sdk.SECURITY_AVAILABLE}")
        print(f"   ✅ NFTManager available: {satox_sdk.NFT_AVAILABLE}")
        print(f"   ✅ IPFSManager available: {satox_sdk.IPFS_AVAILABLE}")
        
        return True
        
    except ImportError as e:
        print(f"   ❌ satox_sdk package not available: {e}")
        return False
    except Exception as e:
        print(f"   ❌ satox_sdk package test failed: {e}")
        return False

def main():
    """Run all tests"""
    print("🚀 Starting Satox SDK Python Bindings Test Suite")
    print("=" * 60)
    
    results = []
    
    # Test main package
    results.append(test_main_package())
    print()
    
    # Test individual components
    results.append(test_asset_manager())
    print()
    
    results.append(test_blockchain_manager())
    print()
    
    results.append(test_security_manager())
    print()
    
    results.append(test_nft_manager())
    print()
    
    results.append(test_ipfs_manager())
    print()
    
    # Summary
    print("=" * 60)
    print("📊 Test Results Summary:")
    print(f"   Total tests: {len(results)}")
    print(f"   Passed: {sum(results)}")
    print(f"   Failed: {len(results) - sum(results)}")
    
    if sum(results) == len(results):
        print("🎉 All tests passed!")
        return 0
    else:
        print("⚠️  Some tests failed. Check the output above for details.")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 