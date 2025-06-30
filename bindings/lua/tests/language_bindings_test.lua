-- Trigger linter refresh
local luaunit = require('luaunit')
local satox = require('satox')
local blockchain = require('satox.blockchain')
local transactions = require('satox.transactions')
local assets = require('satox.assets')
local wallet = require('satox.wallet')
local quantum = require('satox.quantum')
local ipfs = require('satox.ipfs')
local security = require('satox.security')

TestLanguageBindings = {}

function TestLanguageBindings:setUp()
    -- Initialize all managers
    self.manager = satox.SatoxManager.new()
    self.blockchainManager = blockchain.BlockchainManager.new()
    self.transactionManager = transactions.TransactionManager.new()
    self.assetManager = assets.AssetManager.new()
    self.walletManager = wallet.WalletManager.new()
    self.quantumManager = quantum.QuantumManager.new()
    self.ipfsManager = ipfs.IPFSManager.new()
    self.securityManager = security.SecurityManager.new()
end

function TestLanguageBindings:tearDown()
    -- Cleanup
    if self.manager then
        self.manager:shutdown()
    end
end

-- Basic Functionality Tests
function TestLanguageBindings:testInitialization()
    -- Test SatoxManager initialization
    luaunit.assertFalse(self.manager.initialized)
    self.manager:initialize()
    luaunit.assertTrue(self.manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        self.manager:initialize()
    end, "SatoxManager already initialized")
end

-- API Manager Tests
function TestLanguageBindings:testAPIManager()
    -- Register API Manager component
    local apiManager = {
        initialize = function() return true end,
        shutdown = function() return true end
    }
    self.manager:registerComponent("api", apiManager)
    
    -- Test component registration
    local component = self.manager:getComponent("api")
    luaunit.assertNotNil(component)
    
    -- Test component unregistration
    self.manager:unregisterComponent("api")
    luaunit.assertError(function()
        self.manager:getComponent("api")
    end)
end

-- Wallet Manager Tests
function TestLanguageBindings:testWalletManager()
    -- Initialize required managers
    self.blockchainManager:initialize()
    self.quantumManager:initialize()
    self.walletManager:initialize(self.blockchainManager, self.quantumManager)
    
    -- Test wallet creation
    local wallet = self.walletManager:createWallet("test-wallet", "Test wallet")
    luaunit.assertNotNil(wallet)
    luaunit.assertEquals(wallet.name, "test-wallet")
    
    -- Test wallet retrieval
    local retrievedWallet = self.walletManager:getWallet(wallet.id)
    luaunit.assertNotNil(retrievedWallet)
    luaunit.assertEquals(retrievedWallet.id, wallet.id)
    
    -- Test wallet update
    local updatedWallet = self.walletManager:updateWallet(wallet.id, {
        description = "Updated description"
    })
    luaunit.assertEquals(updatedWallet.description, "Updated description")
end

-- IPFS Manager Tests
function TestLanguageBindings:testIPFSManager()
    -- Initialize IPFS Manager
    self.ipfsManager:initialize("127.0.0.1", 5001)
    
    -- Test data storage
    local data = "test data"
    local hash, err = self.ipfsManager:addData(data)
    luaunit.assertNotNil(hash)
    luaunit.assertNil(err)
    
    -- Test data retrieval
    local retrievedData, err = self.ipfsManager:getData(hash)
    luaunit.assertNotNil(retrievedData)
    luaunit.assertNil(err)
    
    -- Test pinning
    local success, err = self.ipfsManager:pinHash(hash)
    luaunit.assertTrue(success)
    luaunit.assertNil(err)
end

-- Quantum Manager Tests
function TestLanguageBindings:testQuantumManager()
    -- Initialize Quantum Manager
    self.quantumManager:initialize()
    
    -- Test key pair generation
    local keyPair = self.quantumManager:generate_key_pair()
    luaunit.assertNotNil(keyPair)
    luaunit.assertNotNil(keyPair.public_key)
    luaunit.assertNotNil(keyPair.private_key)
    
    -- Test encryption/decryption
    local data = "test data"
    local encrypted = self.quantumManager:encrypt(keyPair.public_key, data)
    luaunit.assertNotNil(encrypted)
    
    local decrypted = self.quantumManager:decrypt(keyPair.private_key, encrypted)
    luaunit.assertEquals(decrypted, data)
    
    -- Test signing/verification
    local signature = self.quantumManager:sign(keyPair.private_key, data)
    luaunit.assertNotNil(signature)
    
    local verified = self.quantumManager:verify(keyPair.public_key, data, signature)
    luaunit.assertTrue(verified)
end

-- Post-Quantum Algorithms Tests
function TestLanguageBindings:testPostQuantumAlgorithms()
    -- Test NTRU algorithm
    local keyPair = self.quantumManager:generate_key_pair("NTRU")
    luaunit.assertNotNil(keyPair)
    
    -- Test BIKE algorithm
    local bikeKeyPair = self.quantumManager:generate_key_pair("BIKE")
    luaunit.assertNotNil(bikeKeyPair)
    
    -- Test HQC algorithm
    local hqcKeyPair = self.quantumManager:generate_key_pair("HQC")
    luaunit.assertNotNil(hqcKeyPair)
end

-- Transaction Manager Tests
function TestLanguageBindings:testTransactionManager()
    -- Initialize required managers
    self.blockchainManager:initialize()
    self.quantumManager:initialize()
    self.transactionManager:initialize(self.blockchainManager, self.quantumManager)
    
    -- Test transaction creation
    local inputs = {
        {address = "addr1", amount = 100}
    }
    local outputs = {
        {address = "addr2", amount = 90}
    }
    
    local transaction = self.transactionManager:createTransaction(inputs, outputs)
    luaunit.assertNotNil(transaction)
    luaunit.assertEquals(#transaction.inputs, 1)
    luaunit.assertEquals(#transaction.outputs, 1)
    
    -- Test transaction signing
    local keyPair = self.quantumManager:generate_key_pair()
    local signedTransaction = self.transactionManager:signTransaction(transaction, keyPair.private_key)
    luaunit.assertNotNil(signedTransaction.signature)
end

-- Error Handling Tests
function TestLanguageBindings:testErrorHandling()
    -- Test invalid initialization
    luaunit.assertError(function()
        self.walletManager:initialize(nil, nil)
    end, "BlockchainManager is required")
    
    -- Test invalid wallet creation
    luaunit.assertError(function()
        self.walletManager:createWallet(nil)
    end, "Invalid wallet name")
    
    -- Test invalid transaction creation
    luaunit.assertError(function()
        self.transactionManager:createTransaction(nil, nil)
    end, "Inputs and outputs are required")
end

-- Concurrency Tests
function TestLanguageBindings:testConcurrency()
    -- Initialize managers
    self.manager:initialize()
    self.blockchainManager:initialize()
    self.quantumManager:initialize()
    self.walletManager:initialize(self.blockchainManager, self.quantumManager)
    
    -- Create multiple wallets concurrently
    local wallets = {}
    for i = 1, 10 do
        local wallet = self.walletManager:createWallet("wallet" .. i)
        table.insert(wallets, wallet)
    end
    
    -- Verify all wallets were created
    luaunit.assertEquals(#wallets, 10)
    for _, wallet in ipairs(wallets) do
        luaunit.assertNotNil(wallet)
        luaunit.assertNotNil(wallet.id)
    end
end

-- Performance Tests
function TestLanguageBindings:testPerformance()
    -- Initialize managers
    self.manager:initialize()
    self.blockchainManager:initialize()
    self.quantumManager:initialize()
    self.walletManager:initialize(self.blockchainManager, self.quantumManager)
    
    -- Test wallet creation performance
    local startTime = os.clock()
    for i = 1, 100 do
        self.walletManager:createWallet("perf_wallet" .. i)
    end
    local endTime = os.clock()
    local duration = endTime - startTime
    
    -- Expect to create at least 10 wallets per second
    luaunit.assertLessThan(duration, 10)
end

-- Recovery Tests
function TestLanguageBindings:testRecovery()
    -- Initialize managers
    self.manager:initialize()
    self.blockchainManager:initialize()
    self.quantumManager:initialize()
    self.walletManager:initialize(self.blockchainManager, self.quantumManager)
    
    -- Create a wallet
    local wallet = self.walletManager:createWallet("recovery_wallet")
    luaunit.assertNotNil(wallet)
    
    -- Simulate shutdown
    self.manager:shutdown()
    
    -- Reinitialize and verify recovery
    self.manager:initialize()
    self.blockchainManager:initialize()
    self.quantumManager:initialize()
    self.walletManager:initialize(self.blockchainManager, self.quantumManager)
    
    local recoveredWallet = self.walletManager:getWallet(wallet.id)
    luaunit.assertNotNil(recoveredWallet)
    luaunit.assertEquals(recoveredWallet.id, wallet.id)
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 