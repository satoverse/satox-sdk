local luaunit = require('luaunit')
local ConfigurationManager = require('../configuration')
local BlockchainManager = require('../blockchain')

TestConfigurationManager = {}

function TestConfigurationManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    
    self.configManager = ConfigurationManager.new()
    self.configManager:initialize(self.blockchainManager)
end

function TestConfigurationManager:tearDown()
    -- Clean up configuration file after each test
    os.remove("satox.config.json")
end

function TestConfigurationManager:testInitialization()
    local manager = ConfigurationManager.new()
    luaunit.assertFalse(manager.initialized)
    
    -- Test initialization without required manager
    luaunit.assertError(function()
        manager:initialize(nil)
    end, "BlockchainManager is required")
    
    -- Test proper initialization
    manager:initialize(self.blockchainManager)
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager)
    end, "ConfigurationManager already initialized")
end

function TestConfigurationManager:testSetAndGet()
    -- Test setting string value
    local success = self.configManager:set("test_string", "test_value", "string")
    luaunit.assertTrue(success)
    luaunit.assertEquals(self.configManager:get("test_string"), "test_value")
    
    -- Test setting number value
    success = self.configManager:set("test_number", 42, "number")
    luaunit.assertTrue(success)
    luaunit.assertEquals(self.configManager:get("test_number"), 42)
    
    -- Test setting boolean value
    success = self.configManager:set("test_boolean", true, "boolean")
    luaunit.assertTrue(success)
    luaunit.assertEquals(self.configManager:get("test_boolean"), true)
    
    -- Test setting table value
    local testTable = { key = "value" }
    success = self.configManager:set("test_table", testTable, "table")
    luaunit.assertTrue(success)
    luaunit.assertEquals(self.configManager:get("test_table").key, "value")
    
    -- Test setting array value
    local testArray = { 1, 2, 3 }
    success = self.configManager:set("test_array", testArray, "array")
    luaunit.assertTrue(success)
    luaunit.assertEquals(self.configManager:get("test_array")[1], 1)
    
    -- Test without key
    luaunit.assertError(function()
        self.configManager:set(nil, "value", "string")
    end, "Configuration key is required")
    
    -- Test without type
    luaunit.assertError(function()
        self.configManager:set("test", "value", nil)
    end, "Configuration type is required")
    
    -- Test invalid type
    luaunit.assertError(function()
        self.configManager:set("test", "value", "invalid")
    end, "Invalid configuration type")
    
    -- Test type mismatch
    luaunit.assertError(function()
        self.configManager:set("test", "value", "number")
    end, "Value type does not match specified type")
end

function TestConfigurationManager:testDefaults()
    -- Set default value
    local success = self.configManager:setDefault("test_default", "default_value", "string")
    luaunit.assertTrue(success)
    
    -- Get value (should return default)
    luaunit.assertEquals(self.configManager:get("test_default"), "default_value")
    
    -- Set new value
    success = self.configManager:set("test_default", "new_value", "string")
    luaunit.assertTrue(success)
    luaunit.assertEquals(self.configManager:get("test_default"), "new_value")
    
    -- Reset to default
    success = self.configManager:reset("test_default")
    luaunit.assertTrue(success)
    luaunit.assertEquals(self.configManager:get("test_default"), "default_value")
end

function TestConfigurationManager:testValidators()
    -- Set validator
    local success = self.configManager:setValidator("test_validator", function(value)
        return value > 0
    end)
    luaunit.assertTrue(success)
    
    -- Test valid value
    success = self.configManager:set("test_validator", 42, "number")
    luaunit.assertTrue(success)
    
    -- Test invalid value
    luaunit.assertError(function()
        self.configManager:set("test_validator", -1, "number")
    end, "Value validation failed")
    
    -- Test invalid validator
    luaunit.assertError(function()
        self.configManager:setValidator("test", "not_a_function")
    end, "Validator must be a function")
end

function TestConfigurationManager:testResetAll()
    -- Set some values
    self.configManager:set("test1", "value1", "string")
    self.configManager:set("test2", "value2", "string")
    
    -- Set some defaults
    self.configManager:setDefault("test1", "default1", "string")
    self.configManager:setDefault("test3", "default3", "string")
    
    -- Reset all
    local success = self.configManager:resetAll()
    luaunit.assertTrue(success)
    
    -- Verify values
    luaunit.assertEquals(self.configManager:get("test1"), "default1")
    luaunit.assertNil(self.configManager:get("test2"))
    luaunit.assertEquals(self.configManager:get("test3"), "default3")
end

function TestConfigurationManager:testSaveAndLoad()
    -- Set some values
    self.configManager:set("test1", "value1", "string")
    self.configManager:set("test2", 42, "number")
    
    -- Save configuration
    local success = self.configManager:save()
    luaunit.assertTrue(success)
    
    -- Create new manager and load configuration
    local newManager = ConfigurationManager.new()
    newManager:initialize(self.blockchainManager)
    
    -- Verify values
    luaunit.assertEquals(newManager:get("test1"), "value1")
    luaunit.assertEquals(newManager:get("test2"), 42)
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 