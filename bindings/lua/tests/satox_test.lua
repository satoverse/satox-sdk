local satox = require('satox')
local luaunit = require('luaunit')

TestSatoxSDK = {}

function TestSatoxSDK:setUp()
    -- Initialize test configurations
    self.blockchainConfig = {
        network = 'mainnet',
        rpcUrl = 'http://localhost:7777',
        p2pPort = 60777,
        enableMining = false
    }

    self.nftConfig = {
        enableMetadata = true,
        enableVersioning = true,
        maxMetadataSize = 1024 * 1024 -- 1MB
    }

    self.ipfsConfig = {
        apiHost = '127.0.0.1',
        apiPort = 5001,
        gatewayHost = '127.0.0.1',
        gatewayPort = 8080,
        enablePinning = true
    }

    self.assetConfig = {
        enableMetadata = true,
        enablePermissions = true,
        maxMetadataSize = 1024 * 1024 -- 1MB
    }

    self.transactionConfig = {
        enableFeeEstimation = true,
        enableUTXOManagement = true,
        enableMempool = true
    }

    -- Initialize SDK
    self.sdk = satox.SatoxSDK.new()
    self.sdk:initialize(
        self.blockchainConfig,
        self.nftConfig,
        self.ipfsConfig,
        self.assetConfig,
        self.transactionConfig
    )
end

function TestSatoxSDK:tearDown()
    -- Shutdown SDK
    self.sdk:shutdown()
end

-- Basic Functionality Tests
function TestSatoxSDK:testInitializationAndShutdown()
    local sdk = satox.SatoxSDK.new()
    local result = sdk:initialize(
        self.blockchainConfig,
        self.nftConfig,
        self.ipfsConfig,
        self.assetConfig,
        self.transactionConfig
    )
    luaunit.assertTrue(result)
    luaunit.assertTrue(sdk:isInitialized())

    sdk:shutdown()
    luaunit.assertFalse(sdk:isInitialized())
end

-- Blockchain Tests
function TestSatoxSDK:testBlockchainOperations()
    local block = self.sdk:getBlock('latest')
    luaunit.assertNotNil(block)

    local tx = self.sdk:getTransaction('test_tx_id')
    luaunit.assertNotNil(tx)
end

-- NFT Tests
function TestSatoxSDK:testNFTOperations()
    local metadata = {
        name = 'Test NFT',
        description = 'Test Description',
        properties = {
            type = 'test',
            rarity = 'common'
        }
    }

    local nftId = self.sdk:createNFT(metadata)
    luaunit.assertNotNil(nftId)

    local nft = self.sdk:getNFT(nftId)
    luaunit.assertNotNil(nft)
    luaunit.assertEquals(nft.metadata.name, 'Test NFT')
end

-- IPFS Tests
function TestSatoxSDK:testIPFSOperations()
    local testData = 'test data'
    local cid = self.sdk:addFile(testData)
    luaunit.assertNotNil(cid)

    local pinned = self.sdk:pinFile(cid)
    luaunit.assertTrue(pinned)

    local isPinned = self.sdk:isPinned(cid)
    luaunit.assertTrue(isPinned)
end

-- Asset Tests
function TestSatoxSDK:testAssetOperations()
    local metadata = {
        name = 'Test Asset',
        description = 'Test Description',
        properties = {
            type = 'test',
            supply = '1000'
        }
    }

    local assetId = self.sdk:createAsset(metadata)
    luaunit.assertNotNil(assetId)

    local asset = self.sdk:getAsset(assetId)
    luaunit.assertNotNil(asset)
    luaunit.assertEquals(asset.metadata.name, 'Test Asset')
end

-- Transaction Tests
function TestSatoxSDK:testTransactionOperations()
    local metadata = {
        type = 'transfer',
        description = 'Test Transfer'
    }

    local txId = self.sdk:createTransaction(
        'transfer',
        metadata,
        'sender_address',
        'recipient_address',
        100
    )
    luaunit.assertNotNil(txId)

    local tx = self.sdk:getTransaction(txId)
    luaunit.assertNotNil(tx)
    luaunit.assertEquals(tx.metadata.type, 'transfer')
end

-- Concurrency Tests
function TestSatoxSDK:testConcurrentOperations()
    local results = {}
    local threads = {}

    -- Create 10 threads for concurrent operations
    for i = 1, 10 do
        threads[i] = coroutine.create(function()
            local metadata = {
                name = 'Test NFT',
                description = 'Test Description'
            }
            local nftId = self.sdk:createNFT(metadata)
            table.insert(results, nftId)
        end)
    end

    -- Run all threads
    for i = 1, #threads do
        coroutine.resume(threads[i])
    end

    -- Wait for all threads to complete
    while #results < 10 do
        coroutine.yield()
    end

    luaunit.assertEquals(#results, 10)
    for _, result in ipairs(results) do
        luaunit.assertNotNil(result)
    end
end

-- Performance Tests
function TestSatoxSDK:testPerformance()
    local start = os.time()
    local results = {}

    -- Create 1000 NFTs
    for i = 1, 1000 do
        local metadata = {
            name = 'Test NFT',
            description = 'Test Description'
        }
        local nftId = self.sdk:createNFT(metadata)
        table.insert(results, nftId)
    end

    local duration = os.time() - start
    luaunit.assertEquals(#results, 1000)
    luaunit.assertLessThan(duration, 5) -- Should complete in less than 5 seconds
end

-- Error Handling Tests
function TestSatoxSDK:testErrorHandling()
    -- Test invalid NFT creation
    local invalidMetadata = {
        name = 'Test NFT',
        description = string.rep('A', 1024 * 1024 + 1) -- Exceeds max metadata size
    }

    local success, error = pcall(function()
        self.sdk:createNFT(invalidMetadata)
    end)
    luaunit.assertFalse(success)
    luaunit.assertNotNil(error)

    -- Test invalid transaction
    success, error = pcall(function()
        self.sdk:createTransaction(
            'invalid_type',
            {},
            'invalid_sender',
            'invalid_recipient',
            -1
        )
    end)
    luaunit.assertFalse(success)
    luaunit.assertNotNil(error)
end

-- Recovery Tests
function TestSatoxSDK:testRecovery()
    -- Create test data
    local metadata = {
        name = 'Test NFT',
        description = 'Test Description'
    }
    local nftId = self.sdk:createNFT(metadata)

    -- Simulate failure
    self.sdk:shutdown()

    -- Reinitialize
    self.sdk:initialize(
        self.blockchainConfig,
        self.nftConfig,
        self.ipfsConfig,
        self.assetConfig,
        self.transactionConfig
    )

    -- Verify data is still accessible
    local nft = self.sdk:getNFT(nftId)
    luaunit.assertNotNil(nft)
    luaunit.assertEquals(nft.metadata.name, 'Test NFT')
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 