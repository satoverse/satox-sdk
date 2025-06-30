local luaunit = require('luaunit')
local AssetManager = require('../asset')
local BlockchainManager = require('../blockchain')
local QuantumManager = require('../quantum')

TestAssetManager = {}

function TestAssetManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    
    self.quantumManager = QuantumManager.new()
    self.quantumManager:initialize()
    
    self.assetManager = AssetManager.new()
    self.assetManager:initialize(self.blockchainManager, self.quantumManager)
end

function TestAssetManager:testInitialization()
    local manager = AssetManager.new()
    luaunit.assertFalse(manager.initialized)
    
    -- Test initialization without required managers
    luaunit.assertError(function()
        manager:initialize(nil, nil)
    end, "BlockchainManager is required")
    
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager, nil)
    end, "QuantumManager is required")
    
    -- Test proper initialization
    manager:initialize(self.blockchainManager, self.quantumManager)
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager, self.quantumManager)
    end, "AssetManager already initialized")
end

function TestAssetManager:testCreateAsset()
    -- Create valid asset
    local asset = self.assetManager:createAsset(
        "Test Asset",
        "fungible",
        "Test Description",
        {key = "value"},
        "owner_address"
    )
    
    luaunit.assertNotNil(asset)
    luaunit.assertEquals(asset.name, "Test Asset")
    luaunit.assertEquals(asset.type, "fungible")
    luaunit.assertEquals(asset.description, "Test Description")
    luaunit.assertEquals(asset.metadata.key, "value")
    luaunit.assertEquals(asset.owner, "owner_address")
    luaunit.assertEquals(asset.supply, 0)
    luaunit.assertEquals(asset.decimals, 18)
    luaunit.assertEquals(asset.version, 1)
    
    -- Test with invalid name
    luaunit.assertError(function()
        self.assetManager:createAsset(
            string.rep("a", 65),  -- Too long
            "fungible",
            "Test Description",
            {},
            "owner_address"
        )
    end, "Invalid asset name")
    
    -- Test with invalid type
    luaunit.assertError(function()
        self.assetManager:createAsset(
            "Test Asset",
            "invalid_type",
            "Test Description",
            {},
            "owner_address"
        )
    end, "Invalid asset type")
    
    -- Test with invalid description
    luaunit.assertError(function()
        self.assetManager:createAsset(
            "Test Asset",
            "fungible",
            string.rep("a", 1025),  -- Too long
            {},
            "owner_address"
        )
    end, "Description too long")
    
    -- Test without owner
    luaunit.assertError(function()
        self.assetManager:createAsset(
            "Test Asset",
            "fungible",
            "Test Description",
            {},
            nil
        )
    end, "Owner address is required")
end

function TestAssetManager:testGetAsset()
    -- Create asset
    local asset = self.assetManager:createAsset(
        "Test Asset",
        "fungible",
        "Test Description",
        {},
        "owner_address"
    )
    
    -- Get asset
    local retrievedAsset = self.assetManager:getAsset(asset.id)
    luaunit.assertEquals(retrievedAsset.id, asset.id)
    luaunit.assertEquals(retrievedAsset.name, asset.name)
    
    -- Test with non-existent asset
    luaunit.assertError(function()
        self.assetManager:getAsset("non_existent")
    end, "Asset not found")
end

function TestAssetManager:testUpdateAsset()
    -- Create asset
    local asset = self.assetManager:createAsset(
        "Test Asset",
        "fungible",
        "Test Description",
        {},
        "owner_address"
    )
    
    -- Update asset
    local updatedAsset = self.assetManager:updateAsset(asset.id, {
        name = "Updated Asset",
        description = "Updated Description"
    })
    
    luaunit.assertEquals(updatedAsset.name, "Updated Asset")
    luaunit.assertEquals(updatedAsset.description, "Updated Description")
    luaunit.assertEquals(updatedAsset.version, 2)
    
    -- Test updating non-existent asset
    luaunit.assertError(function()
        self.assetManager:updateAsset("non_existent", {name = "New Name"})
    end, "Asset not found")
    
    -- Test with invalid updates
    luaunit.assertError(function()
        self.assetManager:updateAsset(asset.id, {
            name = string.rep("a", 65)  -- Too long
        })
    end, "Invalid asset name")
end

function TestAssetManager:testMintAsset()
    -- Create asset
    local asset = self.assetManager:createAsset(
        "Test Asset",
        "fungible",
        "Test Description",
        {},
        "owner_address"
    )
    
    -- Mint asset
    local success = self.assetManager:mintAsset(asset.id, 100, "recipient_address")
    luaunit.assertTrue(success)
    
    -- Verify supply update
    local updatedAsset = self.assetManager:getAsset(asset.id)
    luaunit.assertEquals(updatedAsset.supply, 100)
    
    -- Test minting non-existent asset
    luaunit.assertError(function()
        self.assetManager:mintAsset("non_existent", 100, "recipient_address")
    end, "Asset not found")
    
    -- Test with invalid amount
    luaunit.assertError(function()
        self.assetManager:mintAsset(asset.id, 0, "recipient_address")
    end, "Invalid amount")
end

function TestAssetManager:testBurnAsset()
    -- Create and mint asset
    local asset = self.assetManager:createAsset(
        "Test Asset",
        "fungible",
        "Test Description",
        {},
        "owner_address"
    )
    self.assetManager:mintAsset(asset.id, 100, "owner_address")
    
    -- Burn asset
    local success = self.assetManager:burnAsset(asset.id, 50, "owner_address")
    luaunit.assertTrue(success)
    
    -- Verify supply update
    local updatedAsset = self.assetManager:getAsset(asset.id)
    luaunit.assertEquals(updatedAsset.supply, 50)
    
    -- Test burning more than supply
    luaunit.assertError(function()
        self.assetManager:burnAsset(asset.id, 100, "owner_address")
    end, "Insufficient supply")
    
    -- Test burning without authorization
    luaunit.assertError(function()
        self.assetManager:burnAsset(asset.id, 10, "unauthorized_address")
    end, "Unauthorized")
end

function TestAssetManager:testTransferAsset()
    -- Create and mint asset
    local asset = self.assetManager:createAsset(
        "Test Asset",
        "fungible",
        "Test Description",
        {},
        "owner_address"
    )
    self.assetManager:mintAsset(asset.id, 100, "owner_address")
    
    -- Transfer asset
    local success = self.assetManager:transferAsset(
        asset.id,
        50,
        "owner_address",
        "recipient_address"
    )
    luaunit.assertTrue(success)
    
    -- Test transferring non-existent asset
    luaunit.assertError(function()
        self.assetManager:transferAsset(
            "non_existent",
            50,
            "owner_address",
            "recipient_address"
        )
    end, "Asset not found")
    
    -- Test with invalid amount
    luaunit.assertError(function()
        self.assetManager:transferAsset(
            asset.id,
            0,
            "owner_address",
            "recipient_address"
        )
    end, "Invalid amount")
end

function TestAssetManager:testGetAssetBalance()
    -- Create and mint asset
    local asset = self.assetManager:createAsset(
        "Test Asset",
        "fungible",
        "Test Description",
        {},
        "owner_address"
    )
    self.assetManager:mintAsset(asset.id, 100, "owner_address")
    
    -- Get balance
    local balance = self.assetManager:getAssetBalance(asset.id, "owner_address")
    luaunit.assertEquals(balance, 100)
    
    -- Test with non-existent asset
    luaunit.assertError(function()
        self.assetManager:getAssetBalance("non_existent", "owner_address")
    end, "Asset not found")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 