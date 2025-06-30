-- Satox SDK Lua Example
-- Basic usage demonstration

local satox = require("satox")

print("🚀 Satox SDK Lua Example")
print("========================")

-- Initialize the SDK
print("\n1. Initializing SDK...")
local config = {
    network = "mainnet",
    data_dir = "./data",
    enable_mining = false,
    enable_sync = true,
    rpc_endpoint = "http://localhost:7777",
    rpc_username = "user",
    rpc_password = "pass",
    timeout = 30000
}

local success, err = satox.initialize(config)
if not success then
    print("❌ Failed to initialize SDK: " .. tostring(err))
    os.exit(1)
end
print("✅ SDK initialized successfully")

-- Get SDK version
local version = satox.get_version()
print("📦 SDK Version: " .. tostring(version))

-- Get blockchain info
print("\n2. Getting Blockchain Info...")
local blockchain_info = satox.get_blockchain_info()
if blockchain_info then
    print("✅ Blockchain info retrieved")
    print("   Network: " .. tostring(blockchain_info.network or "unknown"))
    print("   Block height: " .. tostring(blockchain_info.block_height or "unknown"))
else
    print("⚠️  Could not get blockchain info")
end

-- Create a wallet
print("\n3. Creating Wallet...")
local wallet = satox.create_wallet()
if wallet then
    print("✅ Wallet created successfully")
    print("   Address: " .. tostring(wallet.address or "unknown"))
else
    print("⚠️  Could not create wallet")
end

-- Create an asset
print("\n4. Creating Asset...")
local asset_config = {
    name = "Test Token",
    symbol = "TEST",
    description = "A test token created with Satox SDK",
    precision = 8,
    reissuable = true,
    total_supply = 1000000
}

local asset = satox.create_asset(asset_config)
if asset then
    print("✅ Asset created successfully")
    print("   Asset ID: " .. tostring(asset.asset_id or "unknown"))
    print("   Name: " .. tostring(asset.name or "unknown"))
else
    print("⚠️  Could not create asset")
end

-- Shutdown the SDK
print("\n5. Shutting down SDK...")
success, err = satox.shutdown()
if not success then
    print("❌ Failed to shutdown SDK: " .. tostring(err))
else
    print("✅ SDK shutdown successfully")
end

print("\n🎉 Example completed successfully!") 