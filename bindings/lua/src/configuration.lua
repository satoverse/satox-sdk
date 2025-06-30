local ConfigurationManager = {}
ConfigurationManager.__index = ConfigurationManager

-- Constants
local DEFAULT_CONFIG_FILE = "satox.config.json"
local SUPPORTED_CONFIG_TYPES = {
    "string",
    "number",
    "boolean",
    "table",
    "array"
}

function ConfigurationManager.new()
    local self = setmetatable({}, ConfigurationManager)
    self.initialized = false
    self.blockchainManager = nil
    self.configFile = nil
    self.config = {}
    self.defaults = {}
    self.validators = {}
    return self
end

function ConfigurationManager:initialize(blockchainManager, options)
    if self.initialized then
        return nil, "ConfigurationManager already initialized"
    end

    if not blockchainManager then
        return nil, "BlockchainManager is required"
    end

    options = options or {}
    self.blockchainManager = blockchainManager
    self.configFile = options.configFile or DEFAULT_CONFIG_FILE

    -- Load configuration
    self:_loadConfig()

    self.initialized = true
    return true
end

function ConfigurationManager:set(key, value, type)
    if not self.initialized then
        return nil, "ConfigurationManager not initialized"
    end

    if not key then
        return nil, "Configuration key is required"
    end

    if not type then
        return nil, "Configuration type is required"
    end

    if not self:_isValidType(type) then
        return nil, "Invalid configuration type"
    end

    -- Validate value type
    if not self:_validateType(value, type) then
        return nil, "Value type does not match specified type"
    end

    -- Run custom validator if exists
    if self.validators[key] and not self.validators[key](value) then
        return nil, "Value validation failed"
    end

    self.config[key] = value
    return true
end

function ConfigurationManager:get(key, defaultValue)
    if not self.initialized then
        return nil, "ConfigurationManager not initialized"
    end

    if not key then
        return nil, "Configuration key is required"
    end

    return self.config[key] or defaultValue
end

function ConfigurationManager:setDefault(key, value, type)
    if not self.initialized then
        return nil, "ConfigurationManager not initialized"
    end

    if not key then
        return nil, "Configuration key is required"
    end

    if not type then
        return nil, "Configuration type is required"
    end

    if not self:_isValidType(type) then
        return nil, "Invalid configuration type"
    end

    -- Validate value type
    if not self:_validateType(value, type) then
        return nil, "Value type does not match specified type"
    end

    self.defaults[key] = {
        value = value,
        type = type
    }
    return true
end

function ConfigurationManager:setValidator(key, validator)
    if not self.initialized then
        return nil, "ConfigurationManager not initialized"
    end

    if not key then
        return nil, "Configuration key is required"
    end

    if not validator or type(validator) ~= "function" then
        return nil, "Validator must be a function"
    end

    self.validators[key] = validator
    return true
end

function ConfigurationManager:reset(key)
    if not self.initialized then
        return nil, "ConfigurationManager not initialized"
    end

    if not key then
        return nil, "Configuration key is required"
    end

    if self.defaults[key] then
        self.config[key] = self.defaults[key].value
    else
        self.config[key] = nil
    end

    return true
end

function ConfigurationManager:resetAll()
    if not self.initialized then
        return nil, "ConfigurationManager not initialized"
    end

    self.config = {}
    for key, default in pairs(self.defaults) do
        self.config[key] = default.value
    end

    return true
end

function ConfigurationManager:save()
    if not self.initialized then
        return nil, "ConfigurationManager not initialized"
    end

    local file = io.open(self.configFile, "w")
    if not file then
        return nil, "Failed to open configuration file for writing"
    end

    local success, json = pcall(function()
        return require("json").encode(self.config)
    end)

    if not success then
        file:close()
        return nil, "Failed to encode configuration"
    end

    file:write(json)
    file:close()
    return true
end

-- Private methods
function ConfigurationManager:_isValidType(type)
    for _, validType in ipairs(SUPPORTED_CONFIG_TYPES) do
        if type == validType then
            return true
        end
    end
    return false
end

function ConfigurationManager:_validateType(value, type)
    if type == "string" then
        return type(value) == "string"
    elseif type == "number" then
        return type(value) == "number"
    elseif type == "boolean" then
        return type(value) == "boolean"
    elseif type == "table" then
        return type(value) == "table" and not self:_isArray(value)
    elseif type == "array" then
        return type(value) == "table" and self:_isArray(value)
    end
    return false
end

function ConfigurationManager:_isArray(t)
    if type(t) ~= "table" then
        return false
    end
    local i = 0
    for _ in pairs(t) do
        i = i + 1
        if t[i] == nil then
            return false
        end
    end
    return true
end

function ConfigurationManager:_loadConfig()
    local file = io.open(self.configFile, "r")
    if not file then
        -- Create default configuration file
        self:resetAll()
        self:save()
        return
    end

    local content = file:read("*all")
    file:close()

    if content == "" then
        self:resetAll()
        return
    end

    local success, config = pcall(function()
        return require("json").decode(content)
    end)

    if not success then
        error("Failed to decode configuration file")
    end

    self.config = config
end

return ConfigurationManager 