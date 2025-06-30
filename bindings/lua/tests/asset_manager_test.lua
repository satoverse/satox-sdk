-- Test file for Asset Manager
local AssetManager = require("satox.asset.asset_manager")

-- Helper function to create test asset metadata
local function createTestMetadata()
    return {
        name = "Test Asset",
        symbol = "TEST",
        totalSupply = 1000000,
        decimals = 8,
        creator = "0x123",
        metadata = {
            description = "Test asset description",
            website = "https://example.com"
        }
    }
end

-- Test initialization
local function testInitialization()
    print("Testing AssetManager initialization...")
    
    local manager = AssetManager.new()
    assert(manager ~= nil, "Manager should not be nil")
    
    local success, err = manager:initialize()
    assert(success, "Initialization should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test double initialization
    success, err = manager:initialize()
    assert(not success, "Double initialization should fail")
    assert(err == "AssetManager already initialized", "Should return initialization error")
    
    print("Initialization tests passed!")
end

-- Test asset creation
local function testAssetCreation()
    print("Testing asset creation...")
    
    local manager = AssetManager.new()
    manager:initialize()
    
    -- Test successful creation
    local metadata = createTestMetadata()
    local assetId, err = manager:createAsset(metadata)
    assert(assetId ~= nil, "Asset ID should not be nil")
    assert(err == nil, "No error should be returned")
    
    -- Test duplicate creation
    local duplicateId, duplicateErr = manager:createAsset(metadata)
    assert(duplicateId == nil, "Duplicate asset should not be created")
    assert(duplicateErr == "Asset already exists", "Should return duplicate error")
    
    -- Test invalid creation
    local invalidMetadata = createTestMetadata()
    invalidMetadata.name = ""
    local invalidId, invalidErr = manager:createAsset(invalidMetadata)
    assert(invalidId == nil, "Invalid asset should not be created")
    assert(invalidErr == "Asset name is required", "Should return validation error")
    
    print("Asset creation tests passed!")
end

-- Test asset retrieval
local function testAssetRetrieval()
    print("Testing asset retrieval...")
    
    local manager = AssetManager.new()
    manager:initialize()
    
    -- Create test asset
    local metadata = createTestMetadata()
    local assetId = manager:createAsset(metadata)
    
    -- Test successful retrieval
    local asset, err = manager:getAsset(assetId)
    assert(asset ~= nil, "Asset should not be nil")
    assert(err == nil, "No error should be returned")
    assert(asset.name == metadata.name, "Asset name should match")
    assert(asset.symbol == metadata.symbol, "Asset symbol should match")
    
    -- Test non-existent asset
    local nonExistentAsset, nonExistentErr = manager:getAsset("non_existent")
    assert(nonExistentAsset == nil, "Non-existent asset should return nil")
    assert(nonExistentErr == "Asset not found", "Should return not found error")
    
    print("Asset retrieval tests passed!")
end

-- Test asset transfer
local function testAssetTransfer()
    print("Testing asset transfer...")
    
    local manager = AssetManager.new()
    manager:initialize()
    
    -- Create test asset
    local metadata = createTestMetadata()
    local assetId = manager:createAsset(metadata)
    
    -- Test successful transfer
    local success, err = manager:transferAsset(assetId, "0x123", "0x456", 1000)
    assert(success, "Transfer should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Verify balances
    local fromBalance = manager:getBalance("0x123", assetId)
    local toBalance = manager:getBalance("0x456", assetId)
    assert(fromBalance == metadata.totalSupply - 1000, "From balance should be reduced")
    assert(toBalance == 1000, "To balance should be increased")
    
    -- Test insufficient balance
    local insufficientSuccess, insufficientErr = manager:transferAsset(assetId, "0x456", "0x789", 2000)
    assert(not insufficientSuccess, "Transfer should fail")
    assert(insufficientErr == "Insufficient balance", "Should return insufficient balance error")
    
    -- Test non-existent asset
    local nonExistentSuccess, nonExistentErr = manager:transferAsset("non_existent", "0x123", "0x456", 1000)
    assert(not nonExistentSuccess, "Transfer should fail")
    assert(nonExistentErr == "Asset not found", "Should return not found error")
    
    print("Asset transfer tests passed!")
end

-- Test balance retrieval
local function testBalanceRetrieval()
    print("Testing balance retrieval...")
    
    local manager = AssetManager.new()
    manager:initialize()
    
    -- Create test asset
    local metadata = createTestMetadata()
    local assetId = manager:createAsset(metadata)
    
    -- Test existing balance
    local balance = manager:getBalance("0x123", assetId)
    assert(balance == metadata.totalSupply, "Balance should match total supply")
    
    -- Test non-existent balance
    local nonExistentBalance = manager:getBalance("0x456", assetId)
    assert(nonExistentBalance == 0, "Non-existent balance should return 0")
    
    print("Balance retrieval tests passed!")
end

-- Test transaction history
local function testTransactionHistory()
    print("Testing transaction history...")
    
    local manager = AssetManager.new()
    manager:initialize()
    
    -- Create test asset
    local metadata = createTestMetadata()
    local assetId = manager:createAsset(metadata)
    
    -- Perform transfers
    manager:transferAsset(assetId, "0x123", "0x456", 1000)
    manager:transferAsset(assetId, "0x456", "0x789", 500)
    
    -- Test history retrieval
    local history = manager:getTransactionHistory(assetId)
    assert(#history == 2, "Should have 2 transactions")
    assert(history[1].from == "0x123", "First transaction from should match")
    assert(history[1].to == "0x456", "First transaction to should match")
    assert(history[1].amount == 1000, "First transaction amount should match")
    
    print("Transaction history tests passed!")
end

-- Run all tests
print("Running Asset Manager tests...")
testInitialization()
testAssetCreation()
testAssetRetrieval()
testAssetTransfer()
testBalanceRetrieval()
testTransactionHistory()
print("All Asset Manager tests passed!") 