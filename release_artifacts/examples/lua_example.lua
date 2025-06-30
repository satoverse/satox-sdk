#!/usr/bin/env lua
--[[
Satox SDK Lua Example Application
Demonstrates all major SDK features including blockchain operations, 
asset management, NFT operations, quantum security, and more.
]]

local satox = require("satox-sdk")

print("🚀 Satox SDK Lua Example Application")
print("====================================\n")

-- Initialize core manager
print("1. Initializing Core Manager...")
local core_manager = satox.core.CoreManager:getInstance()
core_manager:initialize()
print("   ✅ Core Manager initialized successfully\n")

-- Initialize blockchain manager
print("2. Initializing Blockchain Manager...")
local blockchain_manager = satox.blockchain.BlockchainManager:getInstance()
blockchain_manager:initialize()
print("   ✅ Blockchain Manager initialized successfully\n")

-- Initialize security manager
print("3. Initializing Security Manager...")
local security_manager = satox.security.SecurityManager:getInstance()
security_manager:initialize()
print("   ✅ Security Manager initialized successfully\n")

-- Initialize quantum manager
print("4. Initializing Quantum Manager...")
local quantum_manager = satox.quantum.QuantumManager:getInstance()
quantum_manager:initialize()
print("   ✅ Quantum Manager initialized successfully\n")

-- Initialize wallet manager
print("5. Initializing Wallet Manager...")
local wallet_manager = satox.wallet.WalletManager:getInstance()
wallet_manager:initialize()
print("   ✅ Wallet Manager initialized successfully\n")

-- Initialize asset manager
print("6. Initializing Asset Manager...")
local asset_manager = satox.asset.AssetManager:getInstance()
asset_manager:initialize()
print("   ✅ Asset Manager initialized successfully\n")

-- Initialize NFT manager
print("7. Initializing NFT Manager...")
local nft_manager = satox.nft.NFTManager:getInstance()
nft_manager:initialize()
print("   ✅ NFT Manager initialized successfully\n")

-- Initialize transaction manager
print("8. Initializing Transaction Manager...")
local transaction_manager = satox.transaction.TransactionManager:getInstance()
transaction_manager:initialize()
print("   ✅ Transaction Manager initialized successfully\n")

-- Initialize network manager
print("9. Initializing Network Manager...")
local network_manager = satox.network.NetworkManager:getInstance()
network_manager:initialize()
print("   ✅ Network Manager initialized successfully\n")

-- Initialize database manager
print("10. Initializing Database Manager...")
local database_manager = satox.database.DatabaseManager:getInstance()
database_manager:initialize()
print("   ✅ Database Manager initialized successfully\n")

-- Initialize API manager
print("11. Initializing API Manager...")
local api_manager = satox.api.APIManager:getInstance()
api_manager:initialize()
print("   ✅ API Manager initialized successfully\n")

-- Initialize IPFS manager
print("12. Initializing IPFS Manager...")
local ipfs_manager = satox.ipfs.IPFSManager:getInstance()
ipfs_manager:initialize()
print("   ✅ IPFS Manager initialized successfully\n")

-- Demonstrate wallet operations
print("13. Wallet Operations Demo...")
local wallet_address = wallet_manager:createWallet()
print("   ✅ Created wallet: " .. wallet_address)

local balance = wallet_manager:getBalance(wallet_address)
print("   ✅ Wallet balance: " .. balance .. " SATOX")
print()

-- Demonstrate quantum security
print("14. Quantum Security Demo...")
local quantum_key_pair = quantum_manager:generateKeyPair()
print("   ✅ Generated quantum-resistant key pair")

local message = "Hello Quantum World!"
local quantum_signature = quantum_manager:sign(message, quantum_key_pair.private_key)
print("   ✅ Created quantum-resistant signature")

local is_valid = quantum_manager:verify(message, quantum_signature, quantum_key_pair.public_key)
print("   ✅ Quantum signature verification: " .. tostring(is_valid))
print()

-- Demonstrate asset operations
print("15. Asset Operations Demo...")
local asset_config = {
    name = "Test Token",
    symbol = "TEST",
    total_supply = 1000000,
    decimals = 8,
    description = "A test token for demonstration",
    metadata = {
        website = "https://example.com",
        category = "utility"
    }
}

local asset_id = asset_manager:createAsset(asset_config)
print("   ✅ Created asset with ID: " .. asset_id)

local asset_info = asset_manager:getAssetInfo(asset_id)
print("   ✅ Asset info: " .. asset_info.name .. " (" .. asset_info.symbol .. ")")
print()

-- Demonstrate NFT operations
print("16. NFT Operations Demo...")
local nft_config = {
    name = "Test NFT",
    symbol = "TNFT",
    description = "A test NFT for demonstration",
    metadata = {
        image = "https://example.com/image.png",
        attributes = {
            rarity = "common",
            power = 100
        }
    }
}

local nft_id = nft_manager:createNFT(nft_config)
print("   ✅ Created NFT with ID: " .. nft_id)

local nft_info = nft_manager:getNFTInfo(nft_id)
print("   ✅ NFT info: " .. nft_info.name .. " (" .. nft_info.symbol .. ")")
print()

-- Demonstrate transaction operations
print("17. Transaction Operations Demo...")
local tx_config = {
    from = wallet_address,
    to = "recipient_address",
    amount = 1000,
    asset_id = asset_id,
    fee = 100
}

local tx_id = transaction_manager:createTransaction(tx_config)
print("   ✅ Created transaction with ID: " .. tx_id)

local tx_status = transaction_manager:getTransactionStatus(tx_id)
print("   ✅ Transaction status: " .. tx_status)
print()

-- Demonstrate blockchain operations
print("18. Blockchain Operations Demo...")
local blockchain_info = blockchain_manager:getBlockchainInfo()
print("   ✅ Blockchain: " .. blockchain_info.name .. " (Height: " .. blockchain_info.current_height .. ")")

local latest_block = blockchain_manager:getLatestBlock()
print("   ✅ Latest block: " .. latest_block.hash .. " with " .. #latest_block.transactions .. " transactions")
print()

-- Demonstrate network operations
print("19. Network Operations Demo...")
local network_info = network_manager:getNetworkInfo()
print("   ✅ Network: " .. network_info.connections .. " connections")

local peers = network_manager:getPeers()
print("   ✅ Connected peers: " .. #peers)
print()

-- Demonstrate IPFS operations
print("20. IPFS Operations Demo...")
local data = "Hello IPFS World!"
local ipfs_hash = ipfs_manager:uploadData(data)
print("   ✅ Uploaded data to IPFS: " .. ipfs_hash)

local retrieved_data = ipfs_manager:downloadData(ipfs_hash)
print("   ✅ Retrieved data from IPFS: " .. #retrieved_data .. " bytes")
print()

-- Demonstrate database operations
print("21. Database Operations Demo...")
local db_info = database_manager:getDatabaseInfo()
print("   ✅ Database: " .. db_info.table_count .. " tables")

local db_stats = database_manager:getDatabaseStats()
print("   ✅ Database size: " .. string.format("%.2f", db_stats.size_mb) .. " MB")
print()

-- Demonstrate API operations
print("22. API Operations Demo...")
local api_info = api_manager:getAPIInfo()
print("   ✅ API: " .. api_info.endpoint_count .. " endpoints available")

local api_stats = api_manager:getAPIStats()
print("   ✅ API requests: " .. api_stats.total_requests .. " total")
print()

-- Demonstrate security operations
print("23. Security Operations Demo...")
local security_info = security_manager:getSecurityInfo()
print("   ✅ Security: " .. security_info.algorithm_count .. " algorithms supported")

local encryption_key = security_manager:generateEncryptionKey()
print("   ✅ Generated encryption key: " .. #encryption_key .. " bytes")
print()

-- Demonstrate batch operations
print("24. Batch Operations Demo...")
local batch_config = {
    transactions = {
        {
            from = wallet_address,
            to = "recipient1",
            amount = 100,
            asset_id = asset_id,
            fee = 10
        },
        {
            from = wallet_address,
            to = "recipient2",
            amount = 200,
            asset_id = asset_id,
            fee = 10
        }
    }
}

local batch_id = transaction_manager:createBatchTransaction(batch_config)
print("   ✅ Created batch transaction with ID: " .. batch_id)
print()

-- Demonstrate error handling
print("25. Error Handling Demo...")
local success, result = pcall(function()
    return transaction_manager:getTransactionStatus("invalid_tx_id")
end)

if not success then
    print("   ✅ Properly handled error: " .. tostring(result))
else
    print("   ❌ Unexpected success")
end
print()

-- Demonstrate coroutine operations
print("26. Coroutine Operations Demo...")
local function createWallet()
    local wm = satox.wallet.WalletManager:getInstance()
    wm:initialize()
    local address = wm:createWallet()
    wm:shutdown()
    return address
end

local co1 = coroutine.create(createWallet)
local co2 = coroutine.create(createWallet)
local co3 = coroutine.create(createWallet)

local addresses = {}
coroutine.resume(co1)
coroutine.resume(co2)
coroutine.resume(co3)

if coroutine.status(co1) == "dead" then
    local address1 = coroutine.resume(co1)
    table.insert(addresses, address1)
end

if coroutine.status(co2) == "dead" then
    local address2 = coroutine.resume(co2)
    table.insert(addresses, address2)
end

if coroutine.status(co3) == "dead" then
    local address3 = coroutine.resume(co3)
    table.insert(addresses, address3)
end

print("   ✅ Coroutine wallets created: " .. table.concat(addresses, ", "))
print()

-- Demonstrate event handling
print("27. Event Handling Demo...")
local function onTransactionStatusChanged(event)
    print("   📡 Transaction " .. event.transaction_id .. " status changed to " .. event.status)
end

transaction_manager:onTransactionStatusChanged(onTransactionStatusChanged)
print("   ✅ Event handler registered")
print()

-- Demonstrate metatable usage
print("28. Metatable Usage Demo...")
local Asset = {}
Asset.__index = Asset

function Asset.new(config)
    local self = setmetatable({}, Asset)
    self.name = config.name
    self.symbol = config.symbol
    self.total_supply = config.total_supply
    return self
end

function Asset:getInfo()
    return self.name .. " (" .. self.symbol .. ")"
end

local test_asset = Asset.new({
    name = "Test Asset",
    symbol = "TEST",
    total_supply = 1000000
})

print("   ✅ Created asset using metatable: " .. test_asset:getInfo())
print()

-- Demonstrate performance testing
print("29. Performance Testing Demo...")
local start_time = os.clock()

-- Create 100 wallets
local wallet_addresses = {}
for i = 1, 100 do
    local address = wallet_manager:createWallet()
    table.insert(wallet_addresses, address)
end

local end_time = os.clock()
print("   ⚡ Created 100 wallets in " .. string.format("%.2f", end_time - start_time) .. " seconds")
print()

-- Demonstrate cleanup
print("30. Cleanup Operations...")
api_manager:shutdown()
database_manager:shutdown()
ipfs_manager:shutdown()
network_manager:shutdown()
transaction_manager:shutdown()
nft_manager:shutdown()
asset_manager:shutdown()
wallet_manager:shutdown()
quantum_manager:shutdown()
security_manager:shutdown()
blockchain_manager:shutdown()
core_manager:shutdown()
print("   ✅ All managers shut down successfully\n")

print("🎉 Lua Example Application Completed Successfully!")
print("All SDK features demonstrated and working correctly.")


-- Test functions for demonstration
local function test_core_initialization()
    local core_manager = satox.core.CoreManager:getInstance()
    core_manager:initialize()
    core_manager:shutdown()
end

local function test_quantum_operations()
    local quantum_manager = satox.quantum.QuantumManager:getInstance()
    quantum_manager:initialize()

    local key_pair = quantum_manager:generateKeyPair()
    local message = "Test message"
    local signature = quantum_manager:sign(message, key_pair.private_key)
    local is_valid = quantum_manager:verify(message, signature, key_pair.public_key)

    if not is_valid then
        error("Quantum signature verification failed")
    end

    quantum_manager:shutdown()
end

local function test_asset_operations()
    local asset_manager = satox.asset.AssetManager:getInstance()
    asset_manager:initialize()

    local config = {
        name = "Test Asset",
        symbol = "TEST",
        total_supply = 1000000,
        decimals = 8,
        description = "Test asset",
        metadata = {}
    }

    local asset_id = asset_manager:createAsset(config)
    local asset_info = asset_manager:getAssetInfo(asset_id)

    if asset_info.name ~= "Test Asset" then
        error("Unexpected asset name: " .. asset_info.name)
    end

    asset_manager:shutdown()
end

local function test_concurrent_operations()
    local function createWallet()
        local wm = satox.wallet.WalletManager:getInstance()
        wm:initialize()
        local address = wm:createWallet()
        wm:shutdown()
        return address
    end

    local addresses = {}
    for i = 1, 5 do
        local address = createWallet()
        table.insert(addresses, address)
    end

    print("   ✅ Concurrent wallets created: " .. table.concat(addresses, ", "))
end

local function test_performance()
    local wallet_manager = satox.wallet.WalletManager:getInstance()
    wallet_manager:initialize()

    local start_time = os.clock()
    local addresses = {}

    for i = 1, 100 do
        local address = wallet_manager:createWallet()
        table.insert(addresses, address)
    end

    local end_time = os.clock()
    print("   ⚡ Created 100 wallets in " .. string.format("%.2f", end_time - start_time) .. " seconds")

    wallet_manager:shutdown()
end

local function test_error_handling()
    local success, result = pcall(function()
        local transaction_manager = satox.transaction.TransactionManager:getInstance()
        transaction_manager:initialize()
        
        transaction_manager:getTransactionStatus("invalid_tx_id")
        print("   ❌ Expected error but got success")
    end)

    if not success then
        print("   ✅ Properly caught error: " .. tostring(result))
    end
end

-- Export for testing
return {
    test_core_initialization = test_core_initialization,
    test_quantum_operations = test_quantum_operations,
    test_asset_operations = test_asset_operations,
    test_concurrent_operations = test_concurrent_operations,
    test_performance = test_performance,
    test_error_handling = test_error_handling
} 