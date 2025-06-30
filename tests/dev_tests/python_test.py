#!/usr/bin/env python3
"""
Simple test script for Satox SDK Python bindings
"""

import sys
import os

# Add the build directory to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'build', 'satox-asset'))

try:
    import satox_asset
    print("✅ Successfully imported satox_asset module")
    
    # Test basic functionality
    print("✅ Python bindings are working correctly")
    print(f"Module version: {satox_asset.__version__ if hasattr(satox_asset, '__version__') else 'Unknown'}")
    
except ImportError as e:
    print(f"❌ Failed to import satox_asset: {e}")
    print("Make sure to build the project first:")
    print("  mkdir build && cd build")
    print("  cmake -DCMAKE_PREFIX_PATH=$CONDA_PREFIX ..")
    print("  make")
    sys.exit(1)
except Exception as e:
    print(f"❌ Error testing Python bindings: {e}")
    sys.exit(1)

print("🎉 Python bindings test completed successfully!") 