local LoggingManager = {}
LoggingManager.__index = LoggingManager

-- Constants
local LOG_LEVELS = {
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    CRITICAL = 5
}

local DEFAULT_LOG_LEVEL = LOG_LEVELS.INFO
local MAX_LOG_SIZE = 10 * 1024 * 1024  -- 10MB
local MAX_LOG_FILES = 5
local LOG_FORMAT = "[%s] [%s] %s: %s\n"

function LoggingManager.new()
    local self = setmetatable({}, LoggingManager)
    self.initialized = false
    self.blockchainManager = nil
    self.logLevel = DEFAULT_LOG_LEVEL
    self.logFile = nil
    self.currentLogSize = 0
    self.logCount = 0
    return self
end

function LoggingManager:initialize(blockchainManager, options)
    if self.initialized then
        error("LoggingManager already initialized")
    end

    if not blockchainManager then
        error("BlockchainManager is required")
    end

    options = options or {}
    self.blockchainManager = blockchainManager
    self.logLevel = options.logLevel or DEFAULT_LOG_LEVEL
    self.logFile = options.logFile or "satox.log"

    -- Validate log level
    if not self:_isValidLogLevel(self.logLevel) then
        error("Invalid log level")
    end

    -- Initialize log file
    self:_initializeLogFile()

    self.initialized = true
end

function LoggingManager:debug(message, context)
    self:_log(LOG_LEVELS.DEBUG, message, context)
end

function LoggingManager:info(message, context)
    self:_log(LOG_LEVELS.INFO, message, context)
end

function LoggingManager:warning(message, context)
    self:_log(LOG_LEVELS.WARNING, message, context)
end

function LoggingManager:error(message, context)
    self:_log(LOG_LEVELS.ERROR, message, context)
end

function LoggingManager:critical(message, context)
    self:_log(LOG_LEVELS.CRITICAL, message, context)
end

function LoggingManager:setLogLevel(level)
    if not self.initialized then
        error("LoggingManager not initialized")
    end

    if not self:_isValidLogLevel(level) then
        error("Invalid log level")
    end

    self.logLevel = level
    return true
end

function LoggingManager:getLogLevel()
    if not self.initialized then
        error("LoggingManager not initialized")
    end

    return self.logLevel
end

function LoggingManager:rotateLog()
    if not self.initialized then
        error("LoggingManager not initialized")
    end

    return self:_rotateLogFile()
end

function LoggingManager:clearLogs()
    if not self.initialized then
        error("LoggingManager not initialized")
    end

    -- Close current log file
    if self.logFile then
        self.logFile:close()
    end

    -- Remove all log files
    for i = 1, MAX_LOG_FILES do
        local filename = i == 1 and "satox.log" or string.format("satox.%d.log", i)
        os.remove(filename)
    end

    -- Reinitialize log file
    self:_initializeLogFile()
    self.currentLogSize = 0
    self.logCount = 0

    return true
end

-- Private methods
function LoggingManager:_isValidLogLevel(level)
    for _, validLevel in pairs(LOG_LEVELS) do
        if level == validLevel then
            return true
        end
    end
    return false
end

function LoggingManager:_initializeLogFile()
    self.logFile = io.open(self.logFile, "a")
    if not self.logFile then
        error("Failed to open log file")
    end

    -- Get current file size
    self.logFile:seek("end")
    self.currentLogSize = self.logFile:seek("cur")
    self.logFile:seek("set")
end

function LoggingManager:_rotateLogFile()
    -- Close current log file
    self.logFile:close()

    -- Rotate existing log files
    for i = MAX_LOG_FILES - 1, 1, -1 do
        local oldName = i == 1 and "satox.log" or string.format("satox.%d.log", i)
        local newName = string.format("satox.%d.log", i + 1)
        os.rename(oldName, newName)
    end

    -- Create new log file
    self:_initializeLogFile()
    self.currentLogSize = 0
    self.logCount = 0

    return true
end

function LoggingManager:_log(level, message, context)
    if not self.initialized then
        error("LoggingManager not initialized")
    end

    if not message then
        error("Log message is required")
    end

    if level < self.logLevel then
        return
    end

    -- Format log message
    local timestamp = os.date("%Y-%m-%d %H:%M:%S")
    local levelName = self:_getLevelName(level)
    local contextStr = context and string.format(" [%s]", context) or ""
    local logMessage = string.format(LOG_FORMAT, timestamp, levelName, contextStr, message)

    -- Check if we need to rotate the log file
    if self.currentLogSize + #logMessage > MAX_LOG_SIZE then
        self:_rotateLogFile()
    end

    -- Write to log file
    self.logFile:write(logMessage)
    self.logFile:flush()
    self.currentLogSize = self.currentLogSize + #logMessage
    self.logCount = self.logCount + 1
end

function LoggingManager:_getLevelName(level)
    for name, value in pairs(LOG_LEVELS) do
        if value == level then
            return name
        end
    end
    return "UNKNOWN"
end

return LoggingManager 