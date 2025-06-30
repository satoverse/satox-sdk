-- Core Manager Test for Satox SDK Lua Bindings

local luaunit = require('luaunit')
local satox = require('../satox')

TestCoreManager = {}

function TestCoreManager:setUp()
    -- Setup before each test
end

function TestCoreManager:tearDown()
    -- Cleanup after each test
    local success, err = satox.shutdown()
    if not success then
        print("Warning: Failed to shutdown SDK in tearDown: " .. tostring(err))
    end
end

function TestCoreManager:testInitialization()
    local config = {
        network = "mainnet",
        data_dir = "./test_data",
        enable_mining = false,
        enable_sync = false
    }
    
    local success, err = satox.initialize(config)
    luaunit.assertTrue(success, "SDK initialization should succeed")
    luaunit.assertNil(err, "Should not have error on successful initialization")
end

function TestCoreManager:testDoubleInitialization()
    local config = {
        network = "mainnet",
        data_dir = "./test_data",
        enable_mining = false,
        enable_sync = false
    }
    
    -- First initialization
    local success, err = satox.initialize(config)
    luaunit.assertTrue(success, "First SDK initialization should succeed")
    
    -- Second initialization should fail
    local success2, err2 = satox.initialize(config)
    luaunit.assertFalse(success2, "Second SDK initialization should fail")
    luaunit.assertNotNil(err2, "Should have error on double initialization")
end

function TestCoreManager:testShutdown()
    local config = {
        network = "mainnet",
        data_dir = "./test_data",
        enable_mining = false,
        enable_sync = false
    }
    
    -- Initialize first
    local success, err = satox.initialize(config)
    luaunit.assertTrue(success, "SDK initialization should succeed")
    
    -- Then shutdown
    local shutdown_success, shutdown_err = satox.shutdown()
    luaunit.assertTrue(shutdown_success, "SDK shutdown should succeed")
    luaunit.assertNil(shutdown_err, "Should not have error on successful shutdown")
end

function TestCoreManager:testGetVersion()
    local version = satox.get_version()
    luaunit.assertNotNil(version, "Version should not be nil")
    luaunit.assertIsString(version, "Version should be a string")
    luaunit.assertNotEquals(version, "", "Version should not be empty")
end

function TestCoreManager:testInvalidConfig()
    local invalid_config = "not a table"
    
    local success, err = satox.initialize(invalid_config)
    luaunit.assertFalse(success, "SDK initialization with invalid config should fail")
    luaunit.assertNotNil(err, "Should have error with invalid config")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 