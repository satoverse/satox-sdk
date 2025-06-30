--[[
Satox SDK Asset Manager Binding
]]

local asset_manager = {}

-- Private variables
local initialized = false
local callbacks = {}

-- Asset types
asset_manager.ASSET_TYPE = {
    TOKEN = "token",
    NFT = "nft",
    CURRENCY = "currency"
}

-- Initialize the asset manager
function asset_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "AssetManager already initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.asset_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize AssetManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Create an asset
function asset_manager.create_asset(config)
    if not initialized then
        return nil, "AssetManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.asset_create(config)
    end)
    
    if not success then
        return nil, "Failed to create asset: " .. tostring(result)
    end
    
    return result
end

-- Get asset info
function asset_manager.get_asset(asset_id)
    if not initialized then
        return nil, "AssetManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.asset_get(asset_id)
    end)
    
    if not success then
        return nil, "Failed to get asset: " .. tostring(result)
    end
    
    return result
end

-- Transfer asset
function asset_manager.transfer_asset(asset_id, from_address, to_address, amount)
    if not initialized then
        return nil, "AssetManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.asset_transfer(asset_id, from_address, to_address, amount)
    end)
    
    if not success then
        return nil, "Failed to transfer asset: " .. tostring(result)
    end
    
    return result
end

-- Get asset balance
function asset_manager.get_asset_balance(asset_id, address)
    if not initialized then
        return nil, "AssetManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.asset_get_balance(asset_id, address)
    end)
    
    if not success then
        return nil, "Failed to get asset balance: " .. tostring(result)
    end
    
    return result
end

-- Register a callback for asset events
function asset_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function asset_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function asset_manager._trigger_callbacks(event, data)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, data)
    end
end

-- Check if initialized
function asset_manager.is_initialized()
    return initialized
end

return asset_manager 