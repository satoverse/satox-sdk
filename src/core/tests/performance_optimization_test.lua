-- Test suite for Performance Optimization Module
local lu = require('luaunit')
local PerformanceOptimization = require('../src/performance_optimization')

TestPerformanceOptimization = {}

function TestPerformanceOptimization:setUp()
    -- Reset the module state before each test
    PerformanceOptimization.initialize()
end

function TestPerformanceOptimization:test_memory_management()
    -- Test memory allocation
    local size = 100
    local result = PerformanceOptimization.allocate_memory(size)
    lu.assertEquals(result, size)
    
    -- Test memory deallocation
    local dealloc_result = PerformanceOptimization.deallocate_memory(size)
    lu.assertTrue(dealloc_result)
    
    -- Test memory stats
    local stats = PerformanceOptimization.get_memory_stats()
    lu.assertEquals(stats.allocations, 1)
    lu.assertEquals(stats.deallocations, 1)
    lu.assertEquals(stats.peak_usage, size)
    lu.assertEquals(stats.current_usage, 0) -- allocations - deallocations
    
    -- Test invalid parameters
    local invalid_result, invalid_err = PerformanceOptimization.allocate_memory(-1)
    lu.assertNil(invalid_result)
    lu.assertEquals(invalid_err, "Invalid size parameter")
    
    local invalid_dealloc, invalid_dealloc_err = PerformanceOptimization.deallocate_memory("invalid")
    lu.assertFalse(invalid_dealloc)
    lu.assertEquals(invalid_dealloc_err, "Invalid size parameter")
    
    -- Test multiple allocations
    local sizes = {50, 100, 75, 200, 25}
    local total_allocated = 0
    for _, size in ipairs(sizes) do
        PerformanceOptimization.allocate_memory(size)
        total_allocated = total_allocated + size
    end
    
    stats = PerformanceOptimization.get_memory_stats()
    lu.assertEquals(stats.allocations, 6) -- 1 from previous test + 5 new
    lu.assertEquals(stats.peak_usage, 200) -- highest allocation
    lu.assertEquals(stats.current_usage, 5) -- 6 allocations - 1 deallocation
end

function TestPerformanceOptimization:test_cache_management()
    -- Test cache setting
    local key = "test_key"
    local value = "test_value"
    local size = 10
    
    local set_result = PerformanceOptimization.set_cache(key, value, size)
    lu.assertTrue(set_result)
    
    -- Test cache getting
    local cached_value = PerformanceOptimization.get_cache(key)
    lu.assertEquals(cached_value, value)
    
    -- Test cache stats
    local cache_stats = PerformanceOptimization.get_cache_stats()
    lu.assertEquals(cache_stats.current_size, size)
    lu.assertEquals(cache_stats.max_size, 1000)
    lu.assertEquals(cache_stats.item_count, 1)
    
    -- Test getting non-existent cache
    local non_existent = PerformanceOptimization.get_cache("non_existent")
    lu.assertNil(non_existent)
    
    -- Test invalid parameters
    local invalid_set, invalid_set_err = PerformanceOptimization.set_cache("", value, size)
    lu.assertFalse(invalid_set)
    lu.assertEquals(invalid_set_err, "Invalid key parameter")
    
    local invalid_size, invalid_size_err = PerformanceOptimization.set_cache(key, value, -1)
    lu.assertFalse(invalid_size)
    lu.assertEquals(invalid_size_err, "Invalid size parameter")
    
    local invalid_get, invalid_get_err = PerformanceOptimization.get_cache("")
    lu.assertNil(invalid_get)
    lu.assertEquals(invalid_get_err, "Invalid key parameter")
    
    -- Test cache update
    local new_value = "new_value"
    local update_result = PerformanceOptimization.set_cache(key, new_value, size)
    lu.assertTrue(update_result)
    local updated_value = PerformanceOptimization.get_cache(key)
    lu.assertEquals(updated_value, new_value)
    
    -- Test cache with different value types
    local test_cases = {
        {key = "number", value = 123, size = 8},
        {key = "boolean", value = true, size = 4},
        {key = "table", value = {a = 1, b = 2}, size = 16},
        {key = "function", value = function() return true end, size = 8}
    }
    
    for _, case in ipairs(test_cases) do
        local result = PerformanceOptimization.set_cache(case.key, case.value, case.size)
        lu.assertTrue(result)
        local retrieved = PerformanceOptimization.get_cache(case.key)
        lu.assertEquals(retrieved, case.value)
    end
end

function TestPerformanceOptimization:test_cache_limits()
    -- Test cache size limits
    local max_size = 1000
    local large_value = string.rep("x", max_size + 1)
    
    local set_result, set_err = PerformanceOptimization.set_cache("large_key", large_value, max_size + 1)
    lu.assertFalse(set_result)
    lu.assertEquals(set_err, "Cache size limit exceeded")
    
    -- Test multiple items within limit
    local success = true
    for i = 1, 5 do
        success = success and PerformanceOptimization.set_cache("key" .. i, "value" .. i, 100)
    end
    lu.assertTrue(success)
    
    local cache_stats = PerformanceOptimization.get_cache_stats()
    lu.assertEquals(cache_stats.item_count, 5)
    lu.assertEquals(cache_stats.current_size, 500)
    
    -- Test cache eviction (when adding would exceed limit)
    local eviction_result = PerformanceOptimization.set_cache("eviction_test", "value", 600)
    lu.assertFalse(eviction_result)
    
    -- Test exact size limit
    local exact_size_result = PerformanceOptimization.set_cache("exact_size", string.rep("x", 500), 500)
    lu.assertTrue(exact_size_result)
    
    -- Test cache with zero size
    local zero_size_result = PerformanceOptimization.set_cache("zero_size", "value", 0)
    lu.assertTrue(zero_size_result)
    local zero_size_stats = PerformanceOptimization.get_cache_stats()
    lu.assertEquals(zero_size_stats.current_size, 1000) -- previous items + exact size item
end

function TestPerformanceOptimization:test_optimization_utilities()
    -- Test table optimization
    local test_table = {1, 2, 3, 4, 5}
    local optimized_table = PerformanceOptimization.optimize_table(test_table)
    lu.assertEquals(optimized_table, test_table)
    
    -- Test string optimization
    local test_string = "test string"
    local optimized_string = PerformanceOptimization.optimize_string(test_string)
    lu.assertEquals(optimized_string, test_string)
    
    -- Test empty table
    local empty_table = {}
    local optimized_empty = PerformanceOptimization.optimize_table(empty_table)
    lu.assertEquals(optimized_empty, empty_table)
    
    -- Test empty string
    local empty_string = ""
    local optimized_empty_str = PerformanceOptimization.optimize_string(empty_string)
    lu.assertEquals(optimized_empty_str, empty_string)
    
    -- Test invalid parameters
    local invalid_table, invalid_table_err = PerformanceOptimization.optimize_table("not a table")
    lu.assertNil(invalid_table)
    lu.assertEquals(invalid_table_err, "Invalid table parameter")
    
    local invalid_string, invalid_string_err = PerformanceOptimization.optimize_string(123)
    lu.assertNil(invalid_string)
    lu.assertEquals(invalid_string_err, "Invalid string parameter")
    
    -- Test nested table
    local nested_table = {
        a = {1, 2, 3},
        b = {x = "test", y = true},
        c = {}
    }
    local optimized_nested = PerformanceOptimization.optimize_table(nested_table)
    lu.assertEquals(optimized_nested, nested_table)
    
    -- Test string with special characters
    local special_string = "test\nstring\twith\r\nspecial chars"
    local optimized_special = PerformanceOptimization.optimize_string(special_string)
    lu.assertEquals(optimized_special, special_string)
end

function TestPerformanceOptimization:test_initialization()
    -- Test initial state
    PerformanceOptimization.initialize()
    local memory_stats = PerformanceOptimization.get_memory_stats()
    local cache_stats = PerformanceOptimization.get_cache_stats()
    
    lu.assertEquals(memory_stats.allocations, 0)
    lu.assertEquals(memory_stats.deallocations, 0)
    lu.assertEquals(memory_stats.peak_usage, 0)
    lu.assertEquals(memory_stats.current_usage, 0)
    
    lu.assertEquals(cache_stats.current_size, 0)
    lu.assertEquals(cache_stats.item_count, 0)
    
    -- Test multiple initializations
    PerformanceOptimization.initialize()
    memory_stats = PerformanceOptimization.get_memory_stats()
    cache_stats = PerformanceOptimization.get_cache_stats()
    
    lu.assertEquals(memory_stats.allocations, 0)
    lu.assertEquals(memory_stats.deallocations, 0)
    lu.assertEquals(memory_stats.peak_usage, 0)
    lu.assertEquals(memory_stats.current_usage, 0)
    
    lu.assertEquals(cache_stats.current_size, 0)
    lu.assertEquals(cache_stats.item_count, 0)
end

function TestPerformanceOptimization:test_concurrent_operations()
    -- Test memory operations
    local function memory_operation()
        local size = math.random(1, 100)
        PerformanceOptimization.allocate_memory(size)
        PerformanceOptimization.deallocate_memory(size)
    end
    
    -- Test cache operations
    local function cache_operation()
        local key = "key_" .. math.random(1, 1000)
        local value = "value_" .. math.random(1, 1000)
        local size = math.random(1, 100)
        PerformanceOptimization.set_cache(key, value, size)
        PerformanceOptimization.get_cache(key)
    end
    
    -- Run multiple operations
    for i = 1, 10 do
        memory_operation()
        cache_operation()
    end
    
    -- Verify final state
    local memory_stats = PerformanceOptimization.get_memory_stats()
    local cache_stats = PerformanceOptimization.get_cache_stats()
    
    lu.assertEquals(memory_stats.allocations, memory_stats.deallocations)
    lu.assertEquals(memory_stats.current_usage, 0)
    lu.assertTrue(cache_stats.current_size <= cache_stats.max_size)
end

-- Run the tests
os.exit(lu.LuaUnit.run()) 