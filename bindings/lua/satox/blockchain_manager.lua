--[[
Satox SDK Blockchain Manager Binding
]]

local blockchain_manager = {}

-- Private variables
local initialized = false
local callbacks = {}

-- Network types
blockchain_manager.NETWORK_TYPE = {
    MAINNET = "mainnet",
    TESTNET = "testnet",
    REGTEST = "regtest"
}

-- Initialize the blockchain manager
function blockchain_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "BlockchainManager already initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.blockchain_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize BlockchainManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Get blockchain info
function blockchain_manager.get_blockchain_info()
    if not initialized then
        return nil, "BlockchainManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.blockchain_get_info()
    end)
    
    if not success then
        return nil, "Failed to get blockchain info: " .. tostring(result)
    end
    
    return result
end

-- Get block height
function blockchain_manager.get_block_height()
    if not initialized then
        return nil, "BlockchainManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.blockchain_get_block_height()
    end)
    
    if not success then
        return nil, "Failed to get block height: " .. tostring(result)
    end
    
    return result
end

-- Get balance for address
function blockchain_manager.get_balance(address)
    if not initialized then
        return nil, "BlockchainManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.blockchain_get_balance(address)
    end)
    
    if not success then
        return nil, "Failed to get balance: " .. tostring(result)
    end
    
    return result
end

-- Send transaction
function blockchain_manager.send_transaction(tx_data)
    if not initialized then
        return nil, "BlockchainManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.blockchain_send_transaction(tx_data)
    end)
    
    if not success then
        return nil, "Failed to send transaction: " .. tostring(result)
    end
    
    return result
end

-- Get transaction info
function blockchain_manager.get_transaction(tx_id)
    if not initialized then
        return nil, "BlockchainManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.blockchain_get_transaction(tx_id)
    end)
    
    if not success then
        return nil, "Failed to get transaction: " .. tostring(result)
    end
    
    return result
end

-- Register a callback for blockchain events
function blockchain_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function blockchain_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function blockchain_manager._trigger_callbacks(event, data)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, data)
    end
end

-- Check if initialized
function blockchain_manager.is_initialized()
    return initialized
end

return blockchain_manager 