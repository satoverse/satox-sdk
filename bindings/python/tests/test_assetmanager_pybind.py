import importlib.util
import sys
import os

# Try to import the built module from satox-asset/build if not installed
module_name = 'satox_asset'
found = False
for path in [os.path.join(os.path.dirname(__file__), '../../satox-asset/build'), os.getcwd()]:
    try:
        sys.path.insert(0, os.path.abspath(path))
        import satox_asset
        found = True
        break
    except ImportError:
        continue
if not found:
    raise ImportError('satox_asset module not found. Build it with CMake in satox-asset/build.')

def test_assetmanager_singleton():
    mgr = satox_asset.AssetManager.getInstance()
    assert mgr is not None
    assert hasattr(mgr, 'isInitialized')
    assert hasattr(mgr, 'getLastError') 