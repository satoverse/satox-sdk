import satox_sdk
print("✅ satox_sdk imported")
am = satox_sdk.AssetManager.getInstance()
print(f"✅ AssetManager instance: {am}")
print(f"✅ AssetType.MAIN_ASSET: {satox_sdk.AssetType.MAIN_ASSET}") 