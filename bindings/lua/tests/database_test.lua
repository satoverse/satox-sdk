local luaunit = require('luaunit')
local DatabaseManager = require('../database')
local BlockchainManager = require('../blockchain')

TestDatabaseManager = {}

function TestDatabaseManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    
    self.databaseManager = DatabaseManager.new()
    self.databaseManager:initialize(self.blockchainManager)
end

function TestDatabaseManager:testInitialization()
    local manager = DatabaseManager.new()
    luaunit.assertFalse(manager.initialized)
    
    -- Test initialization without required manager
    luaunit.assertError(function()
        manager:initialize(nil)
    end, "BlockchainManager is required")
    
    -- Test initialization with invalid database type
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager, { dbType = "invalid" })
    end, "Unsupported database type")
    
    -- Test proper initialization
    manager:initialize(self.blockchainManager)
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager)
    end, "DatabaseManager already initialized")
end

function TestDatabaseManager:testTransactionManagement()
    -- Begin transaction
    local transaction = self.databaseManager:beginTransaction()
    luaunit.assertNotNil(transaction)
    luaunit.assertNotNil(transaction.id)
    luaunit.assertNotNil(transaction.started_at)
    
    -- Test double transaction
    luaunit.assertError(function()
        self.databaseManager:beginTransaction()
    end, "Transaction already in progress")
    
    -- Commit transaction
    local success = self.databaseManager:commitTransaction()
    luaunit.assertTrue(success)
    
    -- Test commit without transaction
    luaunit.assertError(function()
        self.databaseManager:commitTransaction()
    end, "No transaction in progress")
    
    -- Begin new transaction
    transaction = self.databaseManager:beginTransaction()
    
    -- Rollback transaction
    success = self.databaseManager:rollbackTransaction()
    luaunit.assertTrue(success)
    
    -- Test rollback without transaction
    luaunit.assertError(function()
        self.databaseManager:rollbackTransaction()
    end, "No transaction in progress")
end

function TestDatabaseManager:testQuery()
    -- Execute simple query
    local result = self.databaseManager:query("SELECT * FROM blocks")
    luaunit.assertNotNil(result)
    luaunit.assertNotNil(result.rows)
    luaunit.assertNotNil(result.rowCount)
    
    -- Test without SQL
    luaunit.assertError(function()
        self.databaseManager:query(nil)
    end, "SQL query is required")
    
    -- Test with parameters
    result = self.databaseManager:query("SELECT * FROM blocks WHERE id = ?", { "block_1" })
    luaunit.assertNotNil(result)
end

function TestDatabaseManager:testBatchQuery()
    -- Execute batch query
    local queries = {
        { sql = "SELECT * FROM blocks" },
        { sql = "SELECT * FROM transactions" }
    }
    local results = self.databaseManager:batchQuery(queries)
    luaunit.assertNotNil(results)
    luaunit.assertEquals(#results, 2)
    
    -- Test without queries
    luaunit.assertError(function()
        self.databaseManager:batchQuery(nil)
    end, "Queries are required")
    
    -- Test with empty queries
    luaunit.assertError(function()
        self.databaseManager:batchQuery({})
    end, "Queries are required")
    
    -- Test with too many queries
    local largeQueries = {}
    for i = 1, 1001 do
        largeQueries[i] = { sql = "SELECT * FROM blocks" }
    end
    luaunit.assertError(function()
        self.databaseManager:batchQuery(largeQueries)
    end, "Batch size exceeds maximum limit")
end

function TestDatabaseManager:testClose()
    -- Begin transaction
    self.databaseManager:beginTransaction()
    
    -- Close should rollback transaction
    local success = self.databaseManager:close()
    luaunit.assertTrue(success)
    luaunit.assertFalse(self.databaseManager.initialized)
    
    -- Test operations after close
    luaunit.assertError(function()
        self.databaseManager:query("SELECT * FROM blocks")
    end, "DatabaseManager not initialized")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 