--[[
Satox SDK IPFS Manager Binding
]]

local ipfs_manager = {}

-- Private variables
local initialized = false
local callbacks = {}

-- Initialize the IPFS manager
function ipfs_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "IPFSManager already initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.ipfs_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize IPFSManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Upload data to IPFS
function ipfs_manager.upload_data(data)
    if not initialized then
        return nil, "IPFSManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.ipfs_upload(data)
    end)
    
    if not success then
        return nil, "Failed to upload data to IPFS: " .. tostring(result)
    end
    
    return result
end

-- Download data from IPFS
function ipfs_manager.download_data(ipfs_hash)
    if not initialized then
        return nil, "IPFSManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.ipfs_download(ipfs_hash)
    end)
    
    if not success then
        return nil, "Failed to download data from IPFS: " .. tostring(result)
    end
    
    return result
end

-- Upload file to IPFS
function ipfs_manager.upload_file(file_path)
    if not initialized then
        return nil, "IPFSManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.ipfs_upload_file(file_path)
    end)
    
    if not success then
        return nil, "Failed to upload file to IPFS: " .. tostring(result)
    end
    
    return result
end

-- Download file from IPFS
function ipfs_manager.download_file(ipfs_hash, output_path)
    if not initialized then
        return nil, "IPFSManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.ipfs_download_file(ipfs_hash, output_path)
    end)
    
    if not success then
        return nil, "Failed to download file from IPFS: " .. tostring(result)
    end
    
    return result
end

-- Pin IPFS hash
function ipfs_manager.pin_hash(ipfs_hash)
    if not initialized then
        return nil, "IPFSManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.ipfs_pin(ipfs_hash)
    end)
    
    if not success then
        return nil, "Failed to pin IPFS hash: " .. tostring(result)
    end
    
    return result
end

-- Unpin IPFS hash
function ipfs_manager.unpin_hash(ipfs_hash)
    if not initialized then
        return nil, "IPFSManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.ipfs_unpin(ipfs_hash)
    end)
    
    if not success then
        return nil, "Failed to unpin IPFS hash: " .. tostring(result)
    end
    
    return result
end

-- Get IPFS node info
function ipfs_manager.get_node_info()
    if not initialized then
        return nil, "IPFSManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.ipfs_get_node_info()
    end)
    
    if not success then
        return nil, "Failed to get IPFS node info: " .. tostring(result)
    end
    
    return result
end

-- Register a callback for IPFS events
function ipfs_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function ipfs_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function ipfs_manager._trigger_callbacks(event, data)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, data)
    end
end

-- Check if initialized
function ipfs_manager.is_initialized()
    return initialized
end

return ipfs_manager 