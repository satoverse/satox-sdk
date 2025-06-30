local satox = require("satox")
local luaunit = require("luaunit")

TestCore = {}

function TestCore:setUp()
    self.manager = satox.SatoxManager.new()
end

function TestCore:testSatoxManagerInitialization()
    luaunit.assertNotNil(self.manager)
    self.manager:initialize()
    luaunit.assertTrue(self.manager:isInitialized())
end

function TestCore:testComponentRegistration()
    self.manager:initialize()
    
    local component = TestComponent.new()
    self.manager:registerComponent(component)
    
    luaunit.assertTrue(self.manager:isComponentRegistered(component:getType()))
end

function TestCore:testErrorHandling()
    self.manager:initialize()
    
    luaunit.assertError(function()
        self.manager:registerComponent(nil)
    end, "SatoxException")
end

function TestCore:testThreadSafety()
    self.manager:initialize()
    
    local threads = {}
    for i = 1, 10 do
        threads[i] = coroutine.create(function()
            local component = TestComponent.new()
            self.manager:registerComponent(component)
        end)
    end
    
    for _, thread in ipairs(threads) do
        coroutine.resume(thread)
    end
    
    luaunit.assertTrue(self.manager:isInitialized())
end

TestComponent = {}
TestComponent.__index = TestComponent

function TestComponent.new()
    local self = setmetatable({}, TestComponent)
    self.initialized = false
    return self
end

function TestComponent:getType()
    return "TestComponent"
end

function TestComponent:initialize()
    self.initialized = true
end

function TestComponent:isInitialized()
    return self.initialized
end

os.exit(luaunit.LuaUnit.run()) 