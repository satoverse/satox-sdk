--[[
Satox SDK NFT Manager Binding
]]

local nft_manager = {}

-- Private variables
local initialized = false
local callbacks = {}

-- Initialize the NFT manager
function nft_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "NFTManager already initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.nft_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize NFTManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Create an NFT
function nft_manager.create_nft(config)
    if not initialized then
        return nil, "NFTManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.nft_create(config)
    end)
    
    if not success then
        return nil, "Failed to create NFT: " .. tostring(result)
    end
    
    return result
end

-- Get NFT info
function nft_manager.get_nft(nft_id)
    if not initialized then
        return nil, "NFTManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.nft_get(nft_id)
    end)
    
    if not success then
        return nil, "Failed to get NFT: " .. tostring(result)
    end
    
    return result
end

-- Transfer NFT
function nft_manager.transfer_nft(nft_id, from_address, to_address)
    if not initialized then
        return nil, "NFTManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.nft_transfer(nft_id, from_address, to_address)
    end)
    
    if not success then
        return nil, "Failed to transfer NFT: " .. tostring(result)
    end
    
    return result
end

-- Get NFTs owned by address
function nft_manager.get_nfts_by_owner(address)
    if not initialized then
        return nil, "NFTManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.nft_get_by_owner(address)
    end)
    
    if not success then
        return nil, "Failed to get NFTs by owner: " .. tostring(result)
    end
    
    return result
end

-- Update NFT metadata
function nft_manager.update_nft_metadata(nft_id, metadata)
    if not initialized then
        return nil, "NFTManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.nft_update_metadata(nft_id, metadata)
    end)
    
    if not success then
        return nil, "Failed to update NFT metadata: " .. tostring(result)
    end
    
    return result
end

-- Register a callback for NFT events
function nft_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function nft_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function nft_manager._trigger_callbacks(event, data)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, data)
    end
end

-- Check if initialized
function nft_manager.is_initialized()
    return initialized
end

return nft_manager 