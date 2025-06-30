#!/usr/bin/env lua

package.path = package.path .. ";../src/?.lua"
local luaunit = require('luaunit')
local satox = require('satox')

-- Test suite for Satox SDK Lua bindings
TestSatox = {}

function TestSatox:setUp()
    self.manager = satox
end

function TestSatox:tearDown()
    self.manager = nil
end

function TestSatox:testInitialize()
    local result = self.manager.initialize({})
    luaunit.assertTrue(result)
end

function TestSatox:testShutdown()
    local result = self.manager.shutdown()
    luaunit.assertTrue(result)
end

function TestSatox:testRegisterComponent()
    local component = {
        getType = function() return "TestComponent" end
    }
    local result = self.manager.registerComponent("test", component)
    luaunit.assertTrue(result)
end

function TestSatox:testGetComponent()
    local component = {
        getType = function() return "TestComponent" end
    }
    self.manager.registerComponent("test", component)
    local result = self.manager.getComponent("test")
    luaunit.assertNotNil(result)
end

function TestSatox:testSetConfig()
    local result = self.manager.setConfig("test", "value")
    luaunit.assertTrue(result)
end

function TestSatox:testGetConfig()
    self.manager.setConfig("test", "value")
    local result = self.manager.getConfig("test")
    luaunit.assertEquals(result, "value")
end

function TestSatox:testCreateTransaction()
    local result = self.manager.createTransaction({
        from = "test",
        to = "test2",
        value = 1.0
    })
    luaunit.assertNotNil(result)
    luaunit.assertTrue(result.success)
    luaunit.assertNotNil(result.txid)
end

function TestSatox:testGenerateQuantumKey()
    local result = self.manager.generateQuantumKey()
    luaunit.assertNotNil(result)
    luaunit.assertNotNil(result.publicKey)
    luaunit.assertNotNil(result.privateKey)
end

-- Run the tests
os.exit(luaunit.LuaUnit.run()) 