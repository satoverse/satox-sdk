local luaunit = require('luaunit')
local BlockchainManager = require('../blockchain')

TestBlockchainManager = {}

function TestBlockchainManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
end

function TestBlockchainManager:testInitialization()
    local manager = BlockchainManager.new()
    luaunit.assertFalse(manager.initialized)
    
    manager:initialize()
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize()
    end, "BlockchainManager already initialized")
end

function TestBlockchainManager:testBlockCreation()
    -- Create a block
    local transactions = {
        {
            hash = "tx1",
            inputs = {{address = "addr1", amount = 100}},
            outputs = {{address = "addr2", amount = 90}}
        }
    }
    
    local block = self.blockchainManager:createBlock(transactions, "miner1")
    luaunit.assertNotNil(block)
    luaunit.assertEquals(block.height, 0)
    luaunit.assertEquals(block.minerAddress, "miner1")
    luaunit.assertEquals(#block.transactions, 1)
    
    -- Get block by hash
    local retrievedBlock = self.blockchainManager:getBlock(block.hash)
    luaunit.assertEquals(retrievedBlock.hash, block.hash)
    
    -- Get latest block
    local latestBlock = self.blockchainManager:getLatestBlock()
    luaunit.assertEquals(latestBlock.hash, block.hash)
    
    -- Get block by height
    local blockByHeight = self.blockchainManager:getBlockByHeight(0)
    luaunit.assertEquals(blockByHeight.hash, block.hash)
end

function TestBlockchainManager:testTransactionManagement()
    -- Create a valid transaction
    local transaction = {
        hash = "tx1",
        inputs = {{address = "addr1", amount = 100}},
        outputs = {{address = "addr2", amount = 90}}
    }
    
    -- Add transaction
    local success = self.blockchainManager:addTransaction(transaction)
    luaunit.assertTrue(success)
    
    -- Get transaction
    local retrievedTx = self.blockchainManager:getTransaction("tx1")
    luaunit.assertEquals(retrievedTx.hash, "tx1")
    
    -- Test invalid transaction
    local invalidTx = {
        hash = "tx2",
        inputs = {{address = "addr1", amount = 50}},
        outputs = {{address = "addr2", amount = 100}}  -- Output exceeds input
    }
    
    luaunit.assertError(function()
        self.blockchainManager:addTransaction(invalidTx)
    end, "Invalid transaction")
end

function TestBlockchainManager:testBalanceTracking()
    -- Create and add transactions
    local tx1 = {
        hash = "tx1",
        inputs = {{address = "addr1", amount = 100}},
        outputs = {{address = "addr2", amount = 90}}
    }
    
    local tx2 = {
        hash = "tx2",
        inputs = {{address = "addr2", amount = 50}},
        outputs = {{address = "addr3", amount = 45}}
    }
    
    self.blockchainManager:addTransaction(tx1)
    self.blockchainManager:addTransaction(tx2)
    
    -- Check balances
    local balance1 = self.blockchainManager:getBalance("addr1")
    local balance2 = self.blockchainManager:getBalance("addr2")
    local balance3 = self.blockchainManager:getBalance("addr3")
    
    luaunit.assertEquals(balance1, -100)  -- Spent 100
    luaunit.assertEquals(balance2, 40)    -- Received 90, spent 50
    luaunit.assertEquals(balance3, 45)    -- Received 45
end

function TestBlockchainManager:testPendingTransactions()
    -- Add some pending transactions
    local tx1 = {
        hash = "tx1",
        inputs = {{address = "addr1", amount = 100}},
        outputs = {{address = "addr2", amount = 90}}
    }
    
    local tx2 = {
        hash = "tx2",
        inputs = {{address = "addr2", amount = 50}},
        outputs = {{address = "addr3", amount = 45}}
    }
    
    self.blockchainManager:addTransaction(tx1)
    self.blockchainManager:addTransaction(tx2)
    
    -- Get pending transactions
    local pending = self.blockchainManager:getPendingTransactions()
    luaunit.assertEquals(#pending, 2)
    
    -- Create a block with these transactions
    local block = self.blockchainManager:createBlock(pending, "miner1")
    
    -- Check that transactions are no longer pending
    pending = self.blockchainManager:getPendingTransactions()
    luaunit.assertEquals(#pending, 0)
end

function TestBlockchainManager:testDifficultyAdjustment()
    -- Create initial block
    local block1 = self.blockchainManager:createBlock({}, "miner1")
    luaunit.assertEquals(self.blockchainManager.difficulty, 1)
    
    -- Create blocks up to difficulty adjustment interval
    for i = 1, 2016 do
        self.blockchainManager:createBlock({}, "miner1")
    end
    
    -- Check if difficulty was adjusted
    luaunit.assertNotEquals(self.blockchainManager.difficulty, 1)
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 