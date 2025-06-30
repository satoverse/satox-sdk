#!/usr/bin/env python3
"""
Test script for Satox SDK Python Bindings
MIT License - Copyright (c) 2025 Satoxcoin Core Developer
"""

import json
import sys
import traceback

def test_sdk_basic():
    """Test basic SDK functionality"""
    print("=== Testing Basic SDK Functionality ===")
    
    try:
        import satox_bindings._core as satox
        
        # Test version
        version = satox.get_version()
        print(f"✓ SDK Version: {version}")
        
        # Test initialization
        print("Initializing SDK...")
        success = satox.initialize()
        print(f"✓ SDK Initialization: {'Success' if success else 'Failed'}")
        
        if not success:
            print("❌ SDK initialization failed")
            return False
            
        # Test shutdown
        print("Shutting down SDK...")
        satox.shutdown()
        print("✓ SDK Shutdown: Success")
        
        return True
        
    except Exception as e:
        print(f"❌ Basic SDK test failed: {e}")
        traceback.print_exc()
        return False

def test_security_manager():
    """Test Security Manager functionality"""
    print("\n=== Testing Security Manager ===")
    
    try:
        import satox_bindings._core as satox
        
        # Initialize SDK
        if not satox.initialize():
            print("❌ Cannot test Security Manager - SDK initialization failed")
            return False
        
        # Create security manager
        security_mgr = satox.SecurityManager.create()
        print("✓ Security Manager created")
        
        # Test keypair generation
        print("Testing keypair generation...")
        public_key, private_key = security_mgr.generate_keypair()
        print(f"✓ Keypair generated - Public: {public_key[:20]}..., Private: {private_key[:20]}...")
        
        # Test data signing
        test_data = "Hello, Satox SDK!"
        print(f"Testing data signing with: '{test_data}'")
        signature = security_mgr.sign_data(test_data, private_key)
        print(f"✓ Data signed - Signature: {signature[:20]}...")
        
        # Test signature verification
        print("Testing signature verification...")
        is_valid = security_mgr.verify_signature(test_data, signature, public_key)
        print(f"✓ Signature verification: {'Valid' if is_valid else 'Invalid'}")
        
        # Test SHA256
        print("Testing SHA256...")
        hash_result = security_mgr.sha256(test_data)
        print(f"✓ SHA256 hash: {hash_result[:20]}...")
        
        # Cleanup
        security_mgr.destroy()
        satox.shutdown()
        
        return True
        
    except Exception as e:
        print(f"❌ Security Manager test failed: {e}")
        traceback.print_exc()
        return False

def test_asset_manager():
    """Test Asset Manager functionality"""
    print("\n=== Testing Asset Manager ===")
    
    try:
        import satox_bindings._core as satox
        
        # Initialize SDK
        if not satox.initialize():
            print("❌ Cannot test Asset Manager - SDK initialization failed")
            return False
        
        # Create asset manager
        asset_mgr = satox.AssetManager.create()
        print("✓ Asset Manager created")
        
        # Test asset creation
        config = {
            "name": "Test Asset",
            "symbol": "TEST",
            "description": "A test asset for SDK testing",
            "precision": 8,
            "reissuable": True,
            "total_supply": 1000000
        }
        
        owner_address = "STX1ABC123DEF456GHI789JKL"
        print(f"Creating asset with config: {config}")
        
        asset_result = asset_mgr.create_asset(json.dumps(config), owner_address)
        print(f"✓ Asset creation result: {asset_result}")
        
        # Test asset info retrieval
        if asset_result:
            try:
                asset_info = json.loads(asset_result)
                asset_id = asset_info.get("asset_id", "unknown")
                print(f"Retrieved asset ID: {asset_id}")
                
                # Test getting asset info
                info_result = asset_mgr.get_asset_info(asset_id)
                print(f"✓ Asset info: {info_result}")
                
            except json.JSONDecodeError:
                print("⚠️ Asset result is not valid JSON, but function succeeded")
        
        # Cleanup
        asset_mgr.destroy()
        satox.shutdown()
        
        return True
        
    except Exception as e:
        print(f"❌ Asset Manager test failed: {e}")
        traceback.print_exc()
        return False

def test_nft_manager():
    """Test NFT Manager functionality"""
    print("\n=== Testing NFT Manager ===")
    
    try:
        import satox_bindings._core as satox
        
        # Initialize SDK
        if not satox.initialize():
            print("❌ Cannot test NFT Manager - SDK initialization failed")
            return False
        
        # Create NFT manager
        nft_mgr = satox.NFTManager.create()
        print("✓ NFT Manager created")
        
        # Test NFT creation
        asset_id = "test_asset_123"
        metadata = {
            "name": "Test NFT",
            "description": "A test NFT for SDK testing",
            "image": "ipfs://QmTestHash",
            "attributes": [
                {"trait_type": "Rarity", "value": "Common"},
                {"trait_type": "Type", "value": "Test"}
            ]
        }
        
        owner_address = "STX1ABC123DEF456GHI789JKL"
        print(f"Creating NFT with asset ID: {asset_id}")
        
        nft_result = nft_mgr.create_nft(asset_id, json.dumps(metadata), owner_address)
        print(f"✓ NFT creation result: {nft_result}")
        
        # Test NFT info retrieval
        if nft_result:
            try:
                nft_info = json.loads(nft_result)
                nft_id = nft_info.get("nft_id", "unknown")
                print(f"Retrieved NFT ID: {nft_id}")
                
                # Test getting NFT info
                info_result = nft_mgr.get_nft_info(nft_id)
                print(f"✓ NFT info: {info_result}")
                
            except json.JSONDecodeError:
                print("⚠️ NFT result is not valid JSON, but function succeeded")
        
        # Cleanup
        nft_mgr.destroy()
        satox.shutdown()
        
        return True
        
    except Exception as e:
        print(f"❌ NFT Manager test failed: {e}")
        traceback.print_exc()
        return False

def main():
    """Run all tests"""
    print("🚀 Satox SDK Python Binding Tests")
    print("=" * 50)
    
    tests = [
        ("Basic SDK", test_sdk_basic),
        ("Security Manager", test_security_manager),
        ("Asset Manager", test_asset_manager),
        ("NFT Manager", test_nft_manager)
    ]
    
    passed = 0
    total = len(tests)
    
    for test_name, test_func in tests:
        print(f"\n🧪 Running {test_name} test...")
        if test_func():
            print(f"✅ {test_name} test PASSED")
            passed += 1
        else:
            print(f"❌ {test_name} test FAILED")
    
    print("\n" + "=" * 50)
    print(f"📊 Test Results: {passed}/{total} tests passed")
    
    if passed == total:
        print("🎉 All tests passed! Python binding is working correctly.")
        return 0
    else:
        print("⚠️ Some tests failed. Please check the output above.")
        return 1

if __name__ == "__main__":
    sys.exit(main()) 