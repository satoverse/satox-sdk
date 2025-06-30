local luaunit = require('luaunit')
local PluginManager = require('../plugin')
local BlockchainManager = require('../blockchain')

TestPluginManager = {}

function TestPluginManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    self.pluginManager = PluginManager.new()
    self.pluginManager:initialize(self.blockchainManager)
end

function TestPluginManager:testInitialization()
    local manager = PluginManager.new()
    luaunit.assertFalse(manager.initialized)
    luaunit.assertError(function()
        manager:initialize(nil)
    end, "BlockchainManager is required")
    manager:initialize(self.blockchainManager)
    luaunit.assertTrue(manager.initialized)
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager)
    end, "PluginManager already initialized")
end

function TestPluginManager:testRegisterAndList()
    local plugin = {}
    local success = self.pluginManager:register("test_plugin", plugin)
    luaunit.assertTrue(success)
    local plugins = self.pluginManager:list()
    luaunit.assertEquals(#plugins, 1)
    luaunit.assertEquals(plugins[1], "test_plugin")
    luaunit.assertEquals(self.pluginManager:getState("test_plugin"), "registered")
    -- Registering again should error
    luaunit.assertError(function()
        self.pluginManager:register("test_plugin", plugin)
    end, "Plugin already registered")
end

function TestPluginManager:testLoadAndUnload()
    local loaded = false
    local unloaded = false
    local plugin = {
        onLoad = function() loaded = true end,
        onUnload = function() unloaded = true end
    }
    self.pluginManager:register("lifecycle_plugin", plugin)
    local success = self.pluginManager:load("lifecycle_plugin")
    luaunit.assertTrue(success)
    luaunit.assertTrue(loaded)
    luaunit.assertEquals(self.pluginManager:getState("lifecycle_plugin"), "loaded")
    success = self.pluginManager:unload("lifecycle_plugin")
    luaunit.assertTrue(success)
    luaunit.assertTrue(unloaded)
    luaunit.assertEquals(self.pluginManager:getState("lifecycle_plugin"), "registered")
end

function TestPluginManager:testRemove()
    local plugin = {}
    self.pluginManager:register("remove_plugin", plugin)
    local success = self.pluginManager:remove("remove_plugin")
    luaunit.assertTrue(success)
    luaunit.assertNil(self.pluginManager.plugins["remove_plugin"])
    luaunit.assertNil(self.pluginManager.pluginStates["remove_plugin"])
    -- Removing again should return false
    luaunit.assertFalse(self.pluginManager:remove("remove_plugin"))
end

function TestPluginManager:testErrorHandling()
    -- Register errors
    luaunit.assertError(function()
        self.pluginManager:register(nil, {})
    end, "Plugin name is required")
    luaunit.assertError(function()
        self.pluginManager:register("err_plugin", nil)
    end, "Plugin must be a table")
    -- Load errors
    luaunit.assertError(function()
        self.pluginManager:load(nil)
    end, "Plugin name is required")
    luaunit.assertError(function()
        self.pluginManager:load("not_registered")
    end, "Plugin not registered")
    -- Unload errors
    luaunit.assertError(function()
        self.pluginManager:unload(nil)
    end, "Plugin name is required")
    luaunit.assertError(function()
        self.pluginManager:unload("not_registered")
    end, "Plugin not registered")
    -- Get state errors
    luaunit.assertError(function()
        self.pluginManager:getState(nil)
    end, "Plugin name is required")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 