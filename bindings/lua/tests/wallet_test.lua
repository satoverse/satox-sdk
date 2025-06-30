local luaunit = require('luaunit')
local WalletManager = require('../wallet')
local BlockchainManager = require('../blockchain')
local QuantumManager = require('../quantum')

TestWalletManager = {}

function TestWalletManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    
    self.quantumManager = QuantumManager.new()
    self.quantumManager:initialize()
    
    self.walletManager = WalletManager.new()
    self.walletManager:initialize(self.blockchainManager, self.quantumManager)
end

function TestWalletManager:testInitialization()
    local manager = WalletManager.new()
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
    end, "WalletManager already initialized")
end

function TestWalletManager:testCreateWallet()
    -- Create valid wallet
    local wallet = self.walletManager:createWallet(
        "Test Wallet",
        "Test Description",
        "NTRU"
    )
    
    luaunit.assertNotNil(wallet)
    luaunit.assertEquals(wallet.name, "Test Wallet")
    luaunit.assertEquals(wallet.description, "Test Description")
    luaunit.assertEquals(wallet.key_type, "NTRU")
    luaunit.assertNotNil(wallet.address)
    luaunit.assertNotNil(wallet.public_key)
    luaunit.assertNotNil(wallet.private_key)
    luaunit.assertEquals(wallet.version, 1)
    
    -- Test with invalid name
    luaunit.assertError(function()
        self.walletManager:createWallet(
            string.rep("a", 65),  -- Too long
            "Test Description",
            "NTRU"
        )
    end, "Invalid wallet name")
    
    -- Test with invalid description
    luaunit.assertError(function()
        self.walletManager:createWallet(
            "Test Wallet",
            string.rep("a", 257),  -- Too long
            "NTRU"
        )
    end, "Description too long")
    
    -- Test with invalid key type
    luaunit.assertError(function()
        self.walletManager:createWallet(
            "Test Wallet",
            "Test Description",
            "INVALID_TYPE"
        )
    end, "Invalid key type")
end

function TestWalletManager:testGetWallet()
    -- Create wallet
    local wallet = self.walletManager:createWallet(
        "Test Wallet",
        "Test Description",
        "NTRU"
    )
    
    -- Get wallet
    local retrievedWallet = self.walletManager:getWallet(wallet.id)
    luaunit.assertEquals(retrievedWallet.id, wallet.id)
    luaunit.assertEquals(retrievedWallet.name, wallet.name)
    
    -- Test with non-existent wallet
    luaunit.assertError(function()
        self.walletManager:getWallet("non_existent")
    end, "Wallet not found")
end

function TestWalletManager:testGetWalletByAddress()
    -- Create wallet
    local wallet = self.walletManager:createWallet(
        "Test Wallet",
        "Test Description",
        "NTRU"
    )
    
    -- Get wallet by address
    local retrievedWallet = self.walletManager:getWalletByAddress(wallet.address)
    luaunit.assertEquals(retrievedWallet.id, wallet.id)
    luaunit.assertEquals(retrievedWallet.address, wallet.address)
    
    -- Test with non-existent address
    local nonExistentWallet = self.walletManager:getWalletByAddress("non_existent")
    luaunit.assertNil(nonExistentWallet)
end

function TestWalletManager:testUpdateWallet()
    -- Create wallet
    local wallet = self.walletManager:createWallet(
        "Test Wallet",
        "Test Description",
        "NTRU"
    )
    
    -- Update wallet
    local updatedWallet = self.walletManager:updateWallet(wallet.id, {
        name = "Updated Wallet",
        description = "Updated Description"
    })
    
    luaunit.assertEquals(updatedWallet.name, "Updated Wallet")
    luaunit.assertEquals(updatedWallet.description, "Updated Description")
    luaunit.assertEquals(updatedWallet.version, 2)
    
    -- Test updating non-existent wallet
    luaunit.assertError(function()
        self.walletManager:updateWallet("non_existent", {name = "New Name"})
    end, "Wallet not found")
    
    -- Test with invalid updates
    luaunit.assertError(function()
        self.walletManager:updateWallet(wallet.id, {
            name = string.rep("a", 65)  -- Too long
        })
    end, "Invalid wallet name")
    
    -- Test updating protected fields
    local protectedWallet = self.walletManager:updateWallet(wallet.id, {
        address = "new_address",
        public_key = "new_public_key",
        private_key = "new_private_key",
        key_type = "BIKE",
        created_at = os.time(),
        version = 999
    })
    
    luaunit.assertEquals(protectedWallet.address, wallet.address)
    luaunit.assertEquals(protectedWallet.public_key, wallet.public_key)
    luaunit.assertEquals(protectedWallet.private_key, wallet.private_key)
    luaunit.assertEquals(protectedWallet.key_type, wallet.key_type)
    luaunit.assertEquals(protectedWallet.created_at, wallet.created_at)
    luaunit.assertEquals(protectedWallet.version, 3)  -- Only incremented by 1
end

function TestWalletManager:testSignMessage()
    -- Create wallet
    local wallet = self.walletManager:createWallet(
        "Test Wallet",
        "Test Description",
        "NTRU"
    )
    
    -- Sign message
    local message = "Test message"
    local signature = self.walletManager:signMessage(wallet.id, message)
    luaunit.assertNotNil(signature)
    
    -- Test signing with non-existent wallet
    luaunit.assertError(function()
        self.walletManager:signMessage("non_existent", message)
    end, "Wallet not found")
    
    -- Test signing without message
    luaunit.assertError(function()
        self.walletManager:signMessage(wallet.id, nil)
    end, "Message is required")
end

function TestWalletManager:testVerifySignature()
    -- Create wallet
    local wallet = self.walletManager:createWallet(
        "Test Wallet",
        "Test Description",
        "NTRU"
    )
    
    -- Sign and verify message
    local message = "Test message"
    local signature = self.walletManager:signMessage(wallet.id, message)
    local isValid = self.walletManager:verifySignature(message, signature, wallet.public_key)
    luaunit.assertTrue(isValid)
    
    -- Test with invalid message
    local isInvalid = self.walletManager:verifySignature("Invalid message", signature, wallet.public_key)
    luaunit.assertFalse(isInvalid)
    
    -- Test with missing parameters
    luaunit.assertError(function()
        self.walletManager:verifySignature(nil, signature, wallet.public_key)
    end, "Message, signature, and public key are required")
end

function TestWalletManager:testGetWalletBalance()
    -- Create wallet
    local wallet = self.walletManager:createWallet(
        "Test Wallet",
        "Test Description",
        "NTRU"
    )
    
    -- Get balance
    local balance = self.walletManager:getWalletBalance(wallet.id)
    luaunit.assertNotNil(balance)
    
    -- Test with non-existent wallet
    luaunit.assertError(function()
        self.walletManager:getWalletBalance("non_existent")
    end, "Wallet not found")
end

function TestWalletManager:testGetWalletTransactions()
    -- Create wallet
    local wallet = self.walletManager:createWallet(
        "Test Wallet",
        "Test Description",
        "NTRU"
    )
    
    -- Get transactions
    local transactions = self.walletManager:getWalletTransactions(wallet.id)
    luaunit.assertNotNil(transactions)
    
    -- Test with non-existent wallet
    luaunit.assertError(function()
        self.walletManager:getWalletTransactions("non_existent")
    end, "Wallet not found")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 