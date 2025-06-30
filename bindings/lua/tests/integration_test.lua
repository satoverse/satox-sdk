local satox = require("satox")
local blockchain = require("satox.blockchain")
local transactions = require("satox.transactions")
local assets = require("satox.assets")
local wallet = require("satox.wallet")
local quantum = require("satox.quantum")
local luaunit = require("luaunit")

TestIntegration = {}

function TestIntegration:setUp()
    self.manager = satox.SatoxManager.new()
    self.blockchainManager = blockchain.BlockchainManager.new()
    self.transactionManager = transactions.TransactionManager.new()
    self.assetManager = assets.AssetManager.new()
    self.walletManager = wallet.WalletManager.new()
    self.quantumManager = quantum.QuantumManager.new()
end

function TestIntegration:testFullTransactionFlow()
    self.manager:initialize()
    self.blockchainManager:initialize()
    self.transactionManager:initialize()
    self.assetManager:initialize()
    self.walletManager:initialize()
    self.quantumManager:initialize()

    -- Create wallet
    local wallet = self.walletManager:createWallet("test-wallet")
    luaunit.assertNotNil(wallet)

    -- Create asset
    local asset = self.assetManager:createAsset("test-asset", 1000)
    luaunit.assertNotNil(asset)

    -- Create transaction
    local transaction = self.transactionManager:createTransaction(
        wallet:getAddress(),
        "recipient-address",
        asset:getId(),
        100
    )
    luaunit.assertNotNil(transaction)

    -- Sign transaction
    local signedTransaction = self.walletManager:signTransaction(transaction, wallet)
    luaunit.assertNotNil(signedTransaction)

    -- Broadcast transaction
    local result = self.blockchainManager:broadcastTransaction(signedTransaction)
    luaunit.assertTrue(result:isSuccess())
end

function TestIntegration:testAssetManagementFlow()
    self.manager:initialize()
    self.assetManager:initialize()
    self.walletManager:initialize()

    -- Create wallet
    local wallet = self.walletManager:createWallet("test-wallet")
    luaunit.assertNotNil(wallet)

    -- Create asset
    local asset = self.assetManager:createAsset("test-asset", 1000)
    luaunit.assertNotNil(asset)

    -- Transfer asset
    local transferResult = self.assetManager:transferAsset(
        asset:getId(),
        wallet:getAddress(),
        "recipient-address",
        100
    )
    luaunit.assertTrue(transferResult:isSuccess())

    -- Verify balance
    local balance = self.assetManager:getBalance(asset:getId(), wallet:getAddress())
    luaunit.assertEquals(900, balance)
end

function TestIntegration:testQuantumSecurityFlow()
    self.manager:initialize()
    self.quantumManager:initialize()
    self.walletManager:initialize()

    -- Generate quantum-resistant keys
    local publicKey, privateKey = self.quantumManager:generateHybridKeyPair()
    luaunit.assertNotNil(publicKey)
    luaunit.assertNotNil(privateKey)

    -- Store keys
    self.quantumManager:storeKeyPair("test-keys", publicKey, privateKey)

    -- Encrypt sensitive data
    local sensitiveData = "sensitive-information"
    local encrypted = self.quantumManager:encryptHybrid(sensitiveData, publicKey)
    luaunit.assertNotNil(encrypted)

    -- Decrypt data
    local decrypted = self.quantumManager:decryptHybrid(encrypted, privateKey)
    luaunit.assertEquals(sensitiveData, decrypted)
end

os.exit(luaunit.LuaUnit.run()) 