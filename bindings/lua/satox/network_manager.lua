--[[
Satox SDK Network Manager Binding
]]

local network_manager = {}

-- Private variables
local initialized = false
local callbacks = {}

-- Mock native module for development/testing
local satox_native = _G.satox_native or {}

-- Mock implementations for development
if not _G.satox_native then
    satox_native.network_initialize = function(config)
        -- Mock implementation
        return true
    end
    
    satox_native.network_connect = function(network_type)
        -- Mock implementation
        return true
    end
    
    satox_native.network_disconnect = function()
        -- Mock implementation
        return true
    end
    
    satox_native.network_get_status = function()
        -- Mock implementation
        return {
            connected = true,
            network_type = "mainnet",
            peer_count = 5,
            uptime = 3600
        }
    end
    
    satox_native.network_get_peer_count = function()
        -- Mock implementation
        return 5
    end
    
    satox_native.network_get_info = function()
        -- Mock implementation
        return {
            network_type = "mainnet",
            port = 60777,
            version = "1.0.0",
            protocol_version = "1.0"
        }
    end
    
    satox_native.network_add_peer = function(peer_address)
        -- Mock implementation
        return true
    end
    
    satox_native.network_remove_peer = function(peer_address)
        -- Mock implementation
        return true
    end
    
    satox_native.network_get_peer_list = function()
        -- Mock implementation
        return {
            "192.168.1.100:60777",
            "192.168.1.101:60777",
            "192.168.1.102:60777"
        }
    end
end

-- Initialize the network manager
function network_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "NetworkManager already initialized"
    end
    
    local success, result = pcall(function()
        return satox_native.network_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize NetworkManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Connect to network
function network_manager.connect(network_type)
    if not initialized then
        return false, "NetworkManager not initialized"
    end
    
    local success, result = pcall(function()
        return satox_native.network_connect(network_type)
    end)
    
    if not success then
        return false, "Failed to connect to network: " .. tostring(result)
    end
    
    return result
end

-- Disconnect from network
function network_manager.disconnect()
    if not initialized then
        return true
    end
    
    local success, result = pcall(function()
        return satox_native.network_disconnect()
    end)
    
    if not success then
        return false, "Failed to disconnect from network: " .. tostring(result)
    end
    
    return result
end

-- Get network status
function network_manager.get_network_status()
    if not initialized then
        return nil, "NetworkManager not initialized"
    end
    
    local success, result = pcall(function()
        return satox_native.network_get_status()
    end)
    
    if not success then
        return nil, "Failed to get network status: " .. tostring(result)
    end
    
    return result
end

-- Get peer count
function network_manager.get_peer_count()
    if not initialized then
        return nil, "NetworkManager not initialized"
    end
    
    local success, result = pcall(function()
        return satox_native.network_get_peer_count()
    end)
    
    if not success then
        return nil, "Failed to get peer count: " .. tostring(result)
    end
    
    return result
end

-- Get network info
function network_manager.get_network_info()
    if not initialized then
        return nil, "NetworkManager not initialized"
    end
    
    local success, result = pcall(function()
        return satox_native.network_get_info()
    end)
    
    if not success then
        return nil, "Failed to get network info: " .. tostring(result)
    end
    
    return result
end

-- Add peer
function network_manager.add_peer(peer_address)
    if not initialized then
        return false, "NetworkManager not initialized"
    end
    
    local success, result = pcall(function()
        return satox_native.network_add_peer(peer_address)
    end)
    
    if not success then
        return false, "Failed to add peer: " .. tostring(result)
    end
    
    return result
end

-- Remove peer
function network_manager.remove_peer(peer_address)
    if not initialized then
        return false, "NetworkManager not initialized"
    end
    
    local success, result = pcall(function()
        return satox_native.network_remove_peer(peer_address)
    end)
    
    if not success then
        return false, "Failed to remove peer: " .. tostring(result)
    end
    
    return result
end

-- Get peer list
function network_manager.get_peer_list()
    if not initialized then
        return nil, "NetworkManager not initialized"
    end
    
    local success, result = pcall(function()
        return satox_native.network_get_peer_list()
    end)
    
    if not success then
        return nil, "Failed to get peer list: " .. tostring(result)
    end
    
    return result
end

-- Register a callback for network events
function network_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function network_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function network_manager._trigger_callbacks(event, data)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, data)
    end
end

-- Check if initialized
function network_manager.is_initialized()
    return initialized
end

return network_manager 