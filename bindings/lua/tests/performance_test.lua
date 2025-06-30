local luaunit = require('luaunit')
local PerformanceManager = require('../performance')
local BlockchainManager = require('../blockchain')

TestPerformanceManager = {}

function TestPerformanceManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    
    self.performanceManager = PerformanceManager.new()
    self.performanceManager:initialize(self.blockchainManager)
end

function TestPerformanceManager:testInitialization()
    local manager = PerformanceManager.new()
    luaunit.assertFalse(manager.initialized)
    
    -- Test initialization without required manager
    luaunit.assertError(function()
        manager:initialize(nil)
    end, "BlockchainManager is required")
    
    -- Test initialization with invalid cache size
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager, { cacheSize = 50 })
    end, "Invalid cache size")
    
    -- Test proper initialization
    manager:initialize(self.blockchainManager)
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager)
    end, "PerformanceManager already initialized")
end

function TestPerformanceManager:testCacheOperations()
    -- Set cache item
    local success = self.performanceManager:setCacheItem("test_key", "test_value")
    luaunit.assertTrue(success)
    
    -- Test without key
    luaunit.assertError(function()
        self.performanceManager:setCacheItem(nil, "test_value")
    end, "Cache key is required")
    
    -- Test without value
    luaunit.assertError(function()
        self.performanceManager:setCacheItem("test_key", nil)
    end, "Cache value is required")
    
    -- Get cache item
    local value = self.performanceManager:getCacheItem("test_key")
    luaunit.assertEquals(value, "test_value")
    
    -- Test without key
    luaunit.assertError(function()
        self.performanceManager:getCacheItem(nil)
    end, "Cache key is required")
    
    -- Test non-existent key
    local value = self.performanceManager:getCacheItem("non_existent")
    luaunit.assertNil(value)
    
    -- Remove cache item
    success = self.performanceManager:removeCacheItem("test_key")
    luaunit.assertTrue(success)
    
    -- Test without key
    luaunit.assertError(function()
        self.performanceManager:removeCacheItem(nil)
    end, "Cache key is required")
    
    -- Clear cache
    success = self.performanceManager:clearCache()
    luaunit.assertTrue(success)
end

function TestPerformanceManager:testMetrics()
    -- Get all metrics
    local metrics = self.performanceManager:getMetrics()
    luaunit.assertNotNil(metrics)
    luaunit.assertNotNil(metrics.memory)
    luaunit.assertNotNil(metrics.cpu)
    luaunit.assertNotNil(metrics.io)
    luaunit.assertNotNil(metrics.network)
    
    -- Get specific metrics
    local memoryMetrics = self.performanceManager:getMetrics("memory")
    luaunit.assertNotNil(memoryMetrics)
    luaunit.assertNotNil(memoryMetrics.allocated)
    luaunit.assertNotNil(memoryMetrics.peak)
    luaunit.assertNotNil(memoryMetrics.current)
    
    -- Test invalid strategy
    luaunit.assertError(function()
        self.performanceManager:getMetrics("invalid")
    end, "Unsupported optimization strategy")
end

function TestPerformanceManager:testOptimization()
    -- Test memory optimization
    local success = self.performanceManager:optimize("memory")
    luaunit.assertTrue(success)
    
    -- Test CPU optimization
    success = self.performanceManager:optimize("cpu")
    luaunit.assertTrue(success)
    
    -- Test IO optimization
    success = self.performanceManager:optimize("io")
    luaunit.assertTrue(success)
    
    -- Test network optimization
    success = self.performanceManager:optimize("network")
    luaunit.assertTrue(success)
    
    -- Test without strategy
    luaunit.assertError(function()
        self.performanceManager:optimize(nil)
    end, "Optimization strategy is required")
    
    -- Test invalid strategy
    luaunit.assertError(function()
        self.performanceManager:optimize("invalid")
    end, "Unsupported optimization strategy")
end

function TestPerformanceManager:testMemoryTracking()
    -- Track allocation
    local success = self.performanceManager:trackMemoryAllocation(1024)
    luaunit.assertTrue(success)
    
    -- Test without size
    luaunit.assertError(function()
        self.performanceManager:trackMemoryAllocation(nil)
    end, "Allocation size is required")
    
    -- Track deallocation
    success = self.performanceManager:trackMemoryDeallocation(512)
    luaunit.assertTrue(success)
    
    -- Test without size
    luaunit.assertError(function()
        self.performanceManager:trackMemoryDeallocation(nil)
    end, "Deallocation size is required")
    
    -- Verify metrics
    local metrics = self.performanceManager:getMetrics("memory")
    luaunit.assertEquals(metrics.allocated, 1024)
    luaunit.assertEquals(metrics.current, 512)
    luaunit.assertEquals(metrics.peak, 1024)
end

function TestPerformanceManager:testIOTracking()
    -- Track read operation
    local success = self.performanceManager:trackIOOperation("read", 1024)
    luaunit.assertTrue(success)
    
    -- Track write operation
    success = self.performanceManager:trackIOOperation("write", 2048)
    luaunit.assertTrue(success)
    
    -- Test without operation
    luaunit.assertError(function()
        self.performanceManager:trackIOOperation(nil, 1024)
    end, "IO operation is required")
    
    -- Test without bytes
    luaunit.assertError(function()
        self.performanceManager:trackIOOperation("read", nil)
    end, "Bytes count is required")
    
    -- Test invalid operation
    luaunit.assertError(function()
        self.performanceManager:trackIOOperation("invalid", 1024)
    end, "Invalid IO operation")
    
    -- Verify metrics
    local metrics = self.performanceManager:getMetrics("io")
    luaunit.assertEquals(metrics.reads, 1)
    luaunit.assertEquals(metrics.writes, 1)
    luaunit.assertEquals(metrics.bytes_read, 1024)
    luaunit.assertEquals(metrics.bytes_written, 2048)
end

function TestPerformanceManager:testNetworkTracking()
    -- Track send operation
    local success = self.performanceManager:trackNetworkOperation("send", 1024)
    luaunit.assertTrue(success)
    
    -- Track receive operation
    success = self.performanceManager:trackNetworkOperation("receive", 2048)
    luaunit.assertTrue(success)
    
    -- Test without operation
    luaunit.assertError(function()
        self.performanceManager:trackNetworkOperation(nil, 1024)
    end, "Network operation is required")
    
    -- Test without bytes
    luaunit.assertError(function()
        self.performanceManager:trackNetworkOperation("send", nil)
    end, "Bytes count is required")
    
    -- Test invalid operation
    luaunit.assertError(function()
        self.performanceManager:trackNetworkOperation("invalid", 1024)
    end, "Invalid network operation")
    
    -- Verify metrics
    local metrics = self.performanceManager:getMetrics("network")
    luaunit.assertEquals(metrics.requests, 2)
    luaunit.assertEquals(metrics.bytes_sent, 1024)
    luaunit.assertEquals(metrics.bytes_received, 2048)
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 