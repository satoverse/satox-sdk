--[[
Satox SDK Core Manager Binding
]]

local core_manager = {}

-- Private variables
local initialized = false
local callbacks = {}

-- Initialize the core manager
function core_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "CoreManager already initialized"
    end
    
    -- Call native C++ implementation
    local success, result = pcall(function()
        -- This will be implemented by the native binding
        return _G.satox_native.core_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize CoreManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Shutdown the core manager
function core_manager.shutdown()
    if not initialized then
        return true
    end
    
    local success, result = pcall(function()
        return _G.satox_native.core_shutdown()
    end)
    
    if not success then
        return false, "Failed to shutdown CoreManager: " .. tostring(result)
    end
    
    initialized = false
    return result
end

-- Get core manager status
function core_manager.get_status()
    if not initialized then
        return nil, "CoreManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.core_get_status()
    end)
    
    if not success then
        return nil, "Failed to get status: " .. tostring(result)
    end
    
    return result
end

-- Get SDK version
function core_manager.get_version()
    local success, result = pcall(function()
        return _G.satox_native.core_get_version()
    end)
    
    if not success then
        return "unknown"
    end
    
    return result
end

-- Register a callback for core events
function core_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function core_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function core_manager._trigger_callbacks(event, success)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, success)
    end
end

-- Check if initialized
function core_manager.is_initialized()
    return initialized
end

return core_manager 