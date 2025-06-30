--[[
Satox SDK Wallet Manager Binding
]]

local wallet_manager = {}

-- Private variables
local initialized = false
local callbacks = {}

-- Initialize the wallet manager
function wallet_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "WalletManager already initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize WalletManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Create a new wallet
function wallet_manager.create_wallet()
    if not initialized then
        return nil, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_create()
    end)
    
    if not success then
        return nil, "Failed to create wallet: " .. tostring(result)
    end
    
    return result
end

-- Import wallet from private key
function wallet_manager.import_wallet(private_key)
    if not initialized then
        return nil, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_import(private_key)
    end)
    
    if not success then
        return nil, "Failed to import wallet: " .. tostring(result)
    end
    
    return result
end

-- Export wallet private key
function wallet_manager.export_private_key(wallet_address)
    if not initialized then
        return nil, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_export_private_key(wallet_address)
    end)
    
    if not success then
        return nil, "Failed to export private key: " .. tostring(result)
    end
    
    return result
end

-- Get wallet balance
function wallet_manager.get_balance(wallet_address)
    if not initialized then
        return nil, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_get_balance(wallet_address)
    end)
    
    if not success then
        return nil, "Failed to get wallet balance: " .. tostring(result)
    end
    
    return result
end

-- Send transaction
function wallet_manager.send_transaction(from_address, to_address, amount, private_key)
    if not initialized then
        return nil, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_send_transaction(from_address, to_address, amount, private_key)
    end)
    
    if not success then
        return nil, "Failed to send transaction: " .. tostring(result)
    end
    
    return result
end

-- Get transaction history
function wallet_manager.get_transaction_history(wallet_address)
    if not initialized then
        return nil, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_get_transaction_history(wallet_address)
    end)
    
    if not success then
        return nil, "Failed to get transaction history: " .. tostring(result)
    end
    
    return result
end

-- Get wallet info
function wallet_manager.get_wallet_info(wallet_address)
    if not initialized then
        return nil, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_get_info(wallet_address)
    end)
    
    if not success then
        return nil, "Failed to get wallet info: " .. tostring(result)
    end
    
    return result
end

-- Backup wallet
function wallet_manager.backup_wallet(wallet_address, backup_path)
    if not initialized then
        return false, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_backup(wallet_address, backup_path)
    end)
    
    if not success then
        return false, "Failed to backup wallet: " .. tostring(result)
    end
    
    return result
end

-- Restore wallet
function wallet_manager.restore_wallet(backup_path)
    if not initialized then
        return nil, "WalletManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.wallet_restore(backup_path)
    end)
    
    if not success then
        return nil, "Failed to restore wallet: " .. tostring(result)
    end
    
    return result
end

-- Register a callback for wallet events
function wallet_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function wallet_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function wallet_manager._trigger_callbacks(event, data)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, data)
    end
end

-- Check if initialized
function wallet_manager.is_initialized()
    return initialized
end

return wallet_manager 