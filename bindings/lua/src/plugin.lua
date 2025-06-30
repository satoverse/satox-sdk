local PluginManager = {}
PluginManager.__index = PluginManager

function PluginManager.new()
    local self = setmetatable({}, PluginManager)
    self.initialized = false
    self.blockchainManager = nil
    self.plugins = {}
    self.pluginStates = {}
    return self
end

function PluginManager:initialize(blockchainManager)
    if self.initialized then
        error("PluginManager already initialized")
    end
    if not blockchainManager then
        error("BlockchainManager is required")
    end
    self.blockchainManager = blockchainManager
    self.initialized = true
end

function PluginManager:register(name, plugin)
    if not self.initialized then
        error("PluginManager not initialized")
    end
    if not name or type(name) ~= "string" then
        error("Plugin name is required")
    end
    if not plugin or type(plugin) ~= "table" then
        error("Plugin must be a table")
    end
    if self.plugins[name] then
        error("Plugin already registered")
    end
    self.plugins[name] = plugin
    self.pluginStates[name] = "registered"
    return true
end

function PluginManager:load(name)
    if not self.initialized then
        error("PluginManager not initialized")
    end
    if not name or type(name) ~= "string" then
        error("Plugin name is required")
    end
    local plugin = self.plugins[name]
    if not plugin then
        error("Plugin not registered")
    end
    if self.pluginStates[name] == "loaded" then
        return true
    end
    if plugin.onLoad and type(plugin.onLoad) == "function" then
        local ok, err = pcall(plugin.onLoad, self.blockchainManager)
        if not ok then
            error("Plugin onLoad failed: " .. tostring(err))
        end
    end
    self.pluginStates[name] = "loaded"
    return true
end

function PluginManager:unload(name)
    if not self.initialized then
        error("PluginManager not initialized")
    end
    if not name or type(name) ~= "string" then
        error("Plugin name is required")
    end
    local plugin = self.plugins[name]
    if not plugin then
        error("Plugin not registered")
    end
    if self.pluginStates[name] ~= "loaded" then
        return true
    end
    if plugin.onUnload and type(plugin.onUnload) == "function" then
        local ok, err = pcall(plugin.onUnload, self.blockchainManager)
        if not ok then
            error("Plugin onUnload failed: " .. tostring(err))
        end
    end
    self.pluginStates[name] = "registered"
    return true
end

function PluginManager:remove(name)
    if not self.initialized then
        error("PluginManager not initialized")
    end
    if not name or type(name) ~= "string" then
        error("Plugin name is required")
    end
    if not self.plugins[name] then
        return false
    end
    if self.pluginStates[name] == "loaded" then
        self:unload(name)
    end
    self.plugins[name] = nil
    self.pluginStates[name] = nil
    return true
end

function PluginManager:list()
    if not self.initialized then
        error("PluginManager not initialized")
    end
    local result = {}
    for name, _ in pairs(self.plugins) do
        table.insert(result, name)
    end
    return result
end

function PluginManager:getState(name)
    if not self.initialized then
        error("PluginManager not initialized")
    end
    if not name or type(name) ~= "string" then
        error("Plugin name is required")
    end
    return self.pluginStates[name]
end

return PluginManager 