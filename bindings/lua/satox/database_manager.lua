--[[
Satox SDK Database Manager Binding
]]

local database_manager = {}

-- Private variables
local initialized = false
local connected = false
local callbacks = {}

-- Database types
database_manager.DATABASE_TYPE = {
    SQLITE = "sqlite",
    POSTGRESQL = "postgresql",
    MYSQL = "mysql",
    REDIS = "redis",
    MONGODB = "mongodb"
}

-- Initialize the database manager
function database_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "DatabaseManager already initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.database_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize DatabaseManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Connect to database
function database_manager.connect(db_type, connection_string)
    if not initialized then
        return false, "DatabaseManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.database_connect(db_type, connection_string)
    end)
    
    if not success then
        return false, "Failed to connect to database: " .. tostring(result)
    end
    
    connected = result
    return result
end

-- Execute a database query
function database_manager.execute_query(query, params)
    params = params or {}
    
    if not connected then
        return {
            success = false,
            rows = {},
            error = "Not connected to database",
            affected_rows = 0,
            last_insert_id = nil
        }
    end
    
    local success, result = pcall(function()
        return _G.satox_native.database_execute_query(query, params)
    end)
    
    if not success then
        return {
            success = false,
            rows = {},
            error = tostring(result),
            affected_rows = 0,
            last_insert_id = nil
        }
    end
    
    return result
end

-- Execute a transaction
function database_manager.execute_transaction(queries)
    if not connected then
        return false, "Not connected to database"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.database_execute_transaction(queries)
    end)
    
    if not success then
        return false, "Failed to execute transaction: " .. tostring(result)
    end
    
    return result
end

-- Disconnect from database
function database_manager.disconnect()
    if not connected then
        return true
    end
    
    local success, result = pcall(function()
        return _G.satox_native.database_disconnect()
    end)
    
    if not success then
        return false, "Failed to disconnect: " .. tostring(result)
    end
    
    connected = false
    return result
end

-- Register a callback for database events
function database_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function database_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function database_manager._trigger_callbacks(event, data)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, data)
    end
end

-- Check if initialized
function database_manager.is_initialized()
    return initialized
end

-- Check if connected
function database_manager.is_connected()
    return connected
end

return database_manager 