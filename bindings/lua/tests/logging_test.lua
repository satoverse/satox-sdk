local luaunit = require('luaunit')
local LoggingManager = require('../logging')
local BlockchainManager = require('../blockchain')

TestLoggingManager = {}

function TestLoggingManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    
    self.loggingManager = LoggingManager.new()
    self.loggingManager:initialize(self.blockchainManager)
end

function TestLoggingManager:tearDown()
    -- Clean up log files after each test
    self.loggingManager:clearLogs()
end

function TestLoggingManager:testInitialization()
    local manager = LoggingManager.new()
    luaunit.assertFalse(manager.initialized)
    
    -- Test initialization without required manager
    luaunit.assertError(function()
        manager:initialize(nil)
    end, "BlockchainManager is required")
    
    -- Test initialization with invalid log level
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager, { logLevel = 999 })
    end, "Invalid log level")
    
    -- Test proper initialization
    manager:initialize(self.blockchainManager)
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager)
    end, "LoggingManager already initialized")
end

function TestLoggingManager:testLogLevels()
    -- Test setting log level
    local success = self.loggingManager:setLogLevel(LoggingManager.LOG_LEVELS.DEBUG)
    luaunit.assertTrue(success)
    
    -- Test getting log level
    local level = self.loggingManager:getLogLevel()
    luaunit.assertEquals(level, LoggingManager.LOG_LEVELS.DEBUG)
    
    -- Test invalid log level
    luaunit.assertError(function()
        self.loggingManager:setLogLevel(999)
    end, "Invalid log level")
end

function TestLoggingManager:testLogging()
    -- Test debug logging
    self.loggingManager:debug("Debug message", "TestContext")
    
    -- Test info logging
    self.loggingManager:info("Info message", "TestContext")
    
    -- Test warning logging
    self.loggingManager:warning("Warning message", "TestContext")
    
    -- Test error logging
    self.loggingManager:error("Error message", "TestContext")
    
    -- Test critical logging
    self.loggingManager:critical("Critical message", "TestContext")
    
    -- Test without message
    luaunit.assertError(function()
        self.loggingManager:info(nil, "TestContext")
    end, "Log message is required")
    
    -- Test without context
    self.loggingManager:info("Message without context")
end

function TestLoggingManager:testLogRotation()
    -- Set a small max log size to force rotation
    self.loggingManager.MAX_LOG_SIZE = 100
    
    -- Write enough logs to trigger rotation
    for i = 1, 10 do
        self.loggingManager:info(string.rep("x", 50))
    end
    
    -- Verify that rotation occurred
    local file = io.open("satox.1.log", "r")
    luaunit.assertNotNil(file)
    if file then
        file:close()
    end
end

function TestLoggingManager:testClearLogs()
    -- Write some logs
    self.loggingManager:info("Test message 1")
    self.loggingManager:info("Test message 2")
    
    -- Clear logs
    local success = self.loggingManager:clearLogs()
    luaunit.assertTrue(success)
    
    -- Verify logs are cleared
    local file = io.open("satox.log", "r")
    luaunit.assertNotNil(file)
    if file then
        local content = file:read("*all")
        file:close()
        luaunit.assertEquals(content, "")
    end
end

function TestLoggingManager:testLogFormat()
    -- Write a test log
    self.loggingManager:info("Test message", "TestContext")
    
    -- Read the log file
    local file = io.open("satox.log", "r")
    luaunit.assertNotNil(file)
    if file then
        local content = file:read("*all")
        file:close()
        
        -- Verify log format
        luaunit.assertStrContains(content, "[INFO]")
        luaunit.assertStrContains(content, "[TestContext]")
        luaunit.assertStrContains(content, "Test message")
    end
end

function TestLoggingManager:testLogLevelFiltering()
    -- Set log level to WARNING
    self.loggingManager:setLogLevel(LoggingManager.LOG_LEVELS.WARNING)
    
    -- Write logs at different levels
    self.loggingManager:debug("Debug message")
    self.loggingManager:info("Info message")
    self.loggingManager:warning("Warning message")
    self.loggingManager:error("Error message")
    self.loggingManager:critical("Critical message")
    
    -- Read the log file
    local file = io.open("satox.log", "r")
    luaunit.assertNotNil(file)
    if file then
        local content = file:read("*all")
        file:close()
        
        -- Verify only WARNING and above are logged
        luaunit.assertStrNotContains(content, "Debug message")
        luaunit.assertStrNotContains(content, "Info message")
        luaunit.assertStrContains(content, "Warning message")
        luaunit.assertStrContains(content, "Error message")
        luaunit.assertStrContains(content, "Critical message")
    end
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 