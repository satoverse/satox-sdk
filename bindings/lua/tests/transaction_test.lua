local luaunit = require('luaunit')
local TransactionManager = require('../transaction')
local BlockchainManager = require('../blockchain')
local QuantumManager = require('../quantum')

TestTransactionManager = {}

function TestTransactionManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    
    self.quantumManager = QuantumManager.new()
    self.quantumManager:initialize()
    
    self.transactionManager = TransactionManager.new()
    self.transactionManager:initialize(self.blockchainManager, self.quantumManager)
end

function TestTransactionManager:testInitialization()
    local manager = TransactionManager.new()
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
    end, "TransactionManager already initialized")
end

function TestTransactionManager:testCreateTransaction()
    -- Create valid transaction
    local inputs = {
        {address = "addr1", amount = 100}
    }
    local outputs = {
        {address = "addr2", amount = 90}
    }
    
    local transaction = self.transactionManager:createTransaction(inputs, outputs)
    luaunit.assertNotNil(transaction)
    luaunit.assertEquals(transaction.fee, 0.0001)  -- Default fee
    luaunit.assertEquals(#transaction.inputs, 1)
    luaunit.assertEquals(#transaction.outputs, 1)
    
    -- Test with custom fee
    local transaction2 = self.transactionManager:createTransaction(inputs, outputs, 0.001)
    luaunit.assertEquals(transaction2.fee, 0.001)
    
    -- Test with too many inputs
    local manyInputs = {}
    for i = 1, 101 do
        table.insert(manyInputs, {address = "addr" .. i, amount = 1})
    end
    
    luaunit.assertError(function()
        self.transactionManager:createTransaction(manyInputs, outputs)
    end, "Too many inputs")
    
    -- Test with too many outputs
    local manyOutputs = {}
    for i = 1, 101 do
        table.insert(manyOutputs, {address = "addr" .. i, amount = 1})
    end
    
    luaunit.assertError(function()
        self.transactionManager:createTransaction(inputs, manyOutputs)
    end, "Too many outputs")
    
    -- Test with insufficient funds
    local insufficientOutputs = {
        {address = "addr2", amount = 200}  -- More than input
    }
    
    luaunit.assertError(function()
        self.transactionManager:createTransaction(inputs, insufficientOutputs)
    end, "Insufficient funds")
end

function TestTransactionManager:testSignTransaction()
    -- Create transaction
    local inputs = {
        {address = "addr1", amount = 100}
    }
    local outputs = {
        {address = "addr2", amount = 90}
    }
    
    local transaction = self.transactionManager:createTransaction(inputs, outputs)
    
    -- Generate key pair
    local publicKey, privateKey = self.quantumManager:generateKeyPair("NTRU")
    
    -- Sign transaction
    local signedTransaction = self.transactionManager:signTransaction(transaction, privateKey)
    luaunit.assertNotNil(signedTransaction.signature)
    
    -- Test signing without private key
    luaunit.assertError(function()
        self.transactionManager:signTransaction(transaction, nil)
    end, "Private key is required")
end

function TestTransactionManager:testVerifyTransaction()
    -- Create and sign transaction
    local inputs = {
        {address = "addr1", amount = 100}
    }
    local outputs = {
        {address = "addr2", amount = 90}
    }
    
    local transaction = self.transactionManager:createTransaction(inputs, outputs)
    local publicKey, privateKey = self.quantumManager:generateKeyPair("NTRU")
    local signedTransaction = self.transactionManager:signTransaction(transaction, privateKey)
    
    -- Verify valid transaction
    luaunit.assertTrue(self.transactionManager:verifyTransaction(signedTransaction))
    
    -- Test verification of unsigned transaction
    luaunit.assertFalse(self.transactionManager:verifyTransaction(transaction))
    
    -- Test verification of invalid transaction
    local invalidTransaction = {
        hash = "invalid",
        inputs = {{address = "addr1", amount = 50}},
        outputs = {{address = "addr2", amount = 100}},  -- Output exceeds input
        fee = 0.0001,
        timestamp = os.time(),
        version = 1,
        signature = "dummy"
    }
    
    luaunit.assertFalse(self.transactionManager:verifyTransaction(invalidTransaction))
end

function TestTransactionManager:testSubmitTransaction()
    -- Create and sign transaction
    local inputs = {
        {address = "addr1", amount = 100}
    }
    local outputs = {
        {address = "addr2", amount = 90}
    }
    
    local transaction = self.transactionManager:createTransaction(inputs, outputs)
    local publicKey, privateKey = self.quantumManager:generateKeyPair("NTRU")
    local signedTransaction = self.transactionManager:signTransaction(transaction, privateKey)
    
    -- Submit valid transaction
    local success = self.transactionManager:submitTransaction(signedTransaction)
    luaunit.assertTrue(success)
    
    -- Test submitting unsigned transaction
    luaunit.assertError(function()
        self.transactionManager:submitTransaction(transaction)
    end, "Invalid transaction")
end

function TestTransactionManager:testGetTransactionStatus()
    -- Create and submit transaction
    local inputs = {
        {address = "addr1", amount = 100}
    }
    local outputs = {
        {address = "addr2", amount = 90}
    }
    
    local transaction = self.transactionManager:createTransaction(inputs, outputs)
    local publicKey, privateKey = self.quantumManager:generateKeyPair("NTRU")
    local signedTransaction = self.transactionManager:signTransaction(transaction, privateKey)
    self.transactionManager:submitTransaction(signedTransaction)
    
    -- Check status
    local status = self.transactionManager:getTransactionStatus(signedTransaction.hash)
    luaunit.assertEquals(status, "pending")
    
    -- Test with non-existent transaction
    local nonExistentStatus = self.transactionManager:getTransactionStatus("nonexistent")
    luaunit.assertEquals(nonExistentStatus, "not_found")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 