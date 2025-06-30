--[[
Test file for Wallet Manager
]]

local luaunit = require('luaunit')
local WalletManager = require('../satox/wallet/wallet_manager')

TestWalletManager = {}

function TestWalletManager:setUp()
    self.manager = WalletManager.new()
    self.manager:initialize()
end

function TestWalletManager:testInitialization()
    local manager = WalletManager.new()
    luaunit.assertFalse(manager.initialized)
    
    manager:initialize()
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize()
    end, "WalletManager already initialized")
end

function TestWalletManager:testCreateWallet()
    -- Test successful wallet creation
    local wallet = self.manager:createWallet("test_wallet", "password123")
    luaunit.assertNotNil(wallet)
    luaunit.assertEquals(wallet.name, "test_wallet")
    luaunit.assertNotNil(wallet.address)
    luaunit.assertNotNil(wallet.publicKey)
    luaunit.assertNotNil(wallet.privateKey)
    luaunit.assertNotNil(wallet.created)
    luaunit.assertNotNil(wallet.lastUsed)
    luaunit.assertEquals(wallet.version, "1.0.0")
    
    -- Test duplicate wallet name
    luaunit.assertError(function()
        self.manager:createWallet("test_wallet", "password123")
    end, "Wallet already exists: test_wallet")
    
    -- Test invalid wallet name
    luaunit.assertError(function()
        self.manager:createWallet(nil, "password123")
    end, "Invalid wallet name")
    
    -- Test invalid password
    luaunit.assertError(function()
        self.manager:createWallet("test_wallet2", nil)
    end, "Invalid password")
end

function TestWalletManager:testImportWallet()
    -- Test successful wallet import
    local privateKey = "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"
    local wallet = self.manager:importWallet("imported_wallet", privateKey, "password123")
    luaunit.assertNotNil(wallet)
    luaunit.assertEquals(wallet.name, "imported_wallet")
    luaunit.assertNotNil(wallet.address)
    luaunit.assertNotNil(wallet.publicKey)
    luaunit.assertNotNil(wallet.privateKey)
    
    -- Test invalid private key
    luaunit.assertError(function()
        self.manager:importWallet("test_wallet", "invalid_key", "password123")
    end, "Invalid private key format")
    
    -- Test duplicate wallet name
    luaunit.assertError(function()
        self.manager:importWallet("imported_wallet", privateKey, "password123")
    end, "Wallet already exists: imported_wallet")
end

function TestWalletManager:testGetWallet()
    -- Create a test wallet
    self.manager:createWallet("test_wallet", "password123")
    
    -- Test successful wallet retrieval
    local wallet = self.manager:getWallet("test_wallet")
    luaunit.assertNotNil(wallet)
    luaunit.assertEquals(wallet.name, "test_wallet")
    
    -- Test non-existent wallet
    luaunit.assertError(function()
        self.manager:getWallet("non_existent")
    end, "Wallet not found: non_existent")
end

function TestWalletManager:testListWallets()
    -- Create test wallets
    self.manager:createWallet("wallet1", "password123")
    self.manager:createWallet("wallet2", "password123")
    
    -- Test wallet listing
    local wallets = self.manager:listWallets()
    luaunit.assertNotNil(wallets)
    luaunit.assertEquals(#wallets, 2)
    luaunit.assertNotNil(wallets["wallet1"])
    luaunit.assertNotNil(wallets["wallet2"])
end

function TestWalletManager:testDeleteWallet()
    -- Create a test wallet
    self.manager:createWallet("test_wallet", "password123")
    
    -- Test successful wallet deletion
    self.manager:deleteWallet("test_wallet", "password123")
    luaunit.assertError(function()
        self.manager:getWallet("test_wallet")
    end, "Wallet not found: test_wallet")
    
    -- Test deletion with wrong password
    self.manager:createWallet("test_wallet2", "password123")
    luaunit.assertError(function()
        self.manager:deleteWallet("test_wallet2", "wrong_password")
    end, "Invalid password")
end

function TestWalletManager:testSignTransaction()
    -- Create a test wallet
    self.manager:createWallet("test_wallet", "password123")
    
    -- Test transaction signing
    local transaction = { to = "0x123", value = 100 }
    local signature = self.manager:signTransaction("test_wallet", transaction, "password123")
    luaunit.assertNotNil(signature)
    
    -- Test signing with wrong password
    luaunit.assertError(function()
        self.manager:signTransaction("test_wallet", transaction, "wrong_password")
    end, "Invalid password")
end

function TestWalletManager:testVerifySignature()
    -- Create a test wallet
    self.manager:createWallet("test_wallet", "password123")
    
    -- Test signature verification
    local transaction = { to = "0x123", value = 100 }
    local signature = self.manager:signTransaction("test_wallet", transaction, "password123")
    local wallet = self.manager:getWallet("test_wallet")
    
    local isValid = self.manager:verifySignature(transaction, signature, wallet.publicKey)
    luaunit.assertTrue(isValid)
end

function TestWalletManager:testChangePassword()
    -- Create a test wallet
    self.manager:createWallet("test_wallet", "old_password")
    
    -- Test password change
    self.manager:changePassword("test_wallet", "old_password", "new_password")
    
    -- Verify old password no longer works
    luaunit.assertError(function()
        self.manager:signTransaction("test_wallet", {}, "old_password")
    end, "Invalid password")
    
    -- Verify new password works
    local signature = self.manager:signTransaction("test_wallet", {}, "new_password")
    luaunit.assertNotNil(signature)
end

function TestWalletManager:testBackupAndRestore()
    -- Create a test wallet
    self.manager:createWallet("test_wallet", "password123")
    
    -- Test wallet backup
    local backup = self.manager:backupWallet("test_wallet", "password123")
    luaunit.assertNotNil(backup)
    luaunit.assertEquals(backup.name, "test_wallet")
    luaunit.assertNotNil(backup.address)
    luaunit.assertNotNil(backup.publicKey)
    luaunit.assertNotNil(backup.privateKey)
    
    -- Delete original wallet
    self.manager:deleteWallet("test_wallet", "password123")
    
    -- Test wallet restore
    local restored = self.manager:restoreWallet(backup, "password123")
    luaunit.assertNotNil(restored)
    luaunit.assertEquals(restored.name, "test_wallet")
    luaunit.assertEquals(restored.address, backup.address)
    luaunit.assertEquals(restored.publicKey, backup.publicKey)
end

function TestWalletManager:testExportWallet()
    -- Create a test wallet
    self.manager:createWallet("test_wallet", "password123")
    
    -- Test wallet export
    local exported = self.manager:exportWallet("test_wallet", "password123")
    luaunit.assertNotNil(exported)
    luaunit.assertEquals(exported.name, "test_wallet")
    luaunit.assertNotNil(exported.address)
    luaunit.assertNotNil(exported.publicKey)
    luaunit.assertNotNil(exported.privateKey)
    
    -- Test export with wrong password
    luaunit.assertError(function()
        self.manager:exportWallet("test_wallet", "wrong_password")
    end, "Invalid password")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 