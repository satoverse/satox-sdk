local luaunit = require('luaunit')
local EventManager = require('../event')
local BlockchainManager = require('../blockchain')

TestEventManager = {}

function TestEventManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    self.eventManager = EventManager.new()
    self.eventManager:initialize(self.blockchainManager)
end

function TestEventManager:testInitialization()
    local manager = EventManager.new()
    luaunit.assertFalse(manager.initialized)
    luaunit.assertError(function()
        manager:initialize(nil)
    end, "BlockchainManager is required")
    manager:initialize(self.blockchainManager)
    luaunit.assertTrue(manager.initialized)
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager)
    end, "EventManager already initialized")
end

function TestEventManager:testOnAndEmit()
    local called = false
    local arg1, arg2
    local function listener(a, b)
        called = true
        arg1 = a
        arg2 = b
    end
    local success = self.eventManager:on("test_event", listener)
    luaunit.assertTrue(success)
    local count = self.eventManager:emit("test_event", 123, "abc")
    luaunit.assertTrue(called)
    luaunit.assertEquals(arg1, 123)
    luaunit.assertEquals(arg2, "abc")
    luaunit.assertEquals(count, 1)
end

function TestEventManager:testMultipleListeners()
    local callCount = 0
    local function listener1() callCount = callCount + 1 end
    local function listener2() callCount = callCount + 1 end
    self.eventManager:on("multi_event", listener1)
    self.eventManager:on("multi_event", listener2)
    local count = self.eventManager:emit("multi_event")
    luaunit.assertEquals(callCount, 2)
    luaunit.assertEquals(count, 2)
end

function TestEventManager:testOff()
    local called = false
    local function listener() called = true end
    self.eventManager:on("off_event", listener)
    local removed = self.eventManager:off("off_event", listener)
    luaunit.assertTrue(removed)
    self.eventManager:emit("off_event")
    luaunit.assertFalse(called)
    -- Removing again should return false
    luaunit.assertFalse(self.eventManager:off("off_event", listener))
end

function TestEventManager:testClear()
    local called = false
    local function listener() called = true end
    self.eventManager:on("clear_event", listener)
    self.eventManager:clear("clear_event")
    self.eventManager:emit("clear_event")
    luaunit.assertFalse(called)
    -- Add again and clear all
    self.eventManager:on("clear_event", listener)
    self.eventManager:clear()
    self.eventManager:emit("clear_event")
    luaunit.assertFalse(called)
end

function TestEventManager:testErrorHandling()
    -- Test missing event name
    luaunit.assertError(function()
        self.eventManager:on(nil, function() end)
    end, "Event name is required")
    luaunit.assertError(function()
        self.eventManager:on("err_event", nil)
    end, "Listener must be a function")
    luaunit.assertError(function()
        self.eventManager:off(nil, function() end)
    end, "Event name is required")
    luaunit.assertError(function()
        self.eventManager:off("err_event", nil)
    end, "Listener must be a function")
    luaunit.assertError(function()
        self.eventManager:emit(nil)
    end, "Event name is required")
    luaunit.assertError(function()
        self.eventManager:clear(nil)
    end)
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 