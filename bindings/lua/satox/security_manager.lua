--[[
Satox SDK Security Manager Binding
]]

local security_manager = {}

-- Private variables
local initialized = false
local callbacks = {}

-- Initialize the security manager
function security_manager.initialize(config)
    config = config or {}
    
    if initialized then
        return false, "SecurityManager already initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_initialize(config)
    end)
    
    if not success then
        return false, "Failed to initialize SecurityManager: " .. tostring(result)
    end
    
    initialized = result
    return result
end

-- Generate key pair
function security_manager.generate_key_pair()
    if not initialized then
        return nil, "SecurityManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_generate_key_pair()
    end)
    
    if not success then
        return nil, "Failed to generate key pair: " .. tostring(result)
    end
    
    return result
end

-- Sign data
function security_manager.sign_data(private_key, data)
    if not initialized then
        return nil, "SecurityManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_sign(private_key, data)
    end)
    
    if not success then
        return nil, "Failed to sign data: " .. tostring(result)
    end
    
    return result
end

-- Verify signature
function security_manager.verify_signature(public_key, data, signature)
    if not initialized then
        return nil, "SecurityManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_verify(public_key, data, signature)
    end)
    
    if not success then
        return nil, "Failed to verify signature: " .. tostring(result)
    end
    
    return result
end

-- Encrypt data
function security_manager.encrypt_data(public_key, data)
    if not initialized then
        return nil, "SecurityManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_encrypt(public_key, data)
    end)
    
    if not success then
        return nil, "Failed to encrypt data: " .. tostring(result)
    end
    
    return result
end

-- Decrypt data
function security_manager.decrypt_data(private_key, encrypted_data)
    if not initialized then
        return nil, "SecurityManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_decrypt(private_key, encrypted_data)
    end)
    
    if not success then
        return nil, "Failed to decrypt data: " .. tostring(result)
    end
    
    return result
end

-- Hash data
function security_manager.hash_data(data, algorithm)
    algorithm = algorithm or "sha256"
    
    if not initialized then
        return nil, "SecurityManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_hash(data, algorithm)
    end)
    
    if not success then
        return nil, "Failed to hash data: " .. tostring(result)
    end
    
    return result
end

-- Generate random bytes
function security_manager.generate_random_bytes(length)
    length = length or 32
    
    if not initialized then
        return nil, "SecurityManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_random_bytes(length)
    end)
    
    if not success then
        return nil, "Failed to generate random bytes: " .. tostring(result)
    end
    
    return result
end

-- Validate address
function security_manager.validate_address(address)
    if not initialized then
        return nil, "SecurityManager not initialized"
    end
    
    local success, result = pcall(function()
        return _G.satox_native.security_validate_address(address)
    end)
    
    if not success then
        return nil, "Failed to validate address: " .. tostring(result)
    end
    
    return result
end

-- Register a callback for security events
function security_manager.register_callback(callback)
    if type(callback) == "function" then
        table.insert(callbacks, callback)
    end
end

-- Unregister a callback
function security_manager.unregister_callback(callback)
    for i, cb in ipairs(callbacks) do
        if cb == callback then
            table.remove(callbacks, i)
            break
        end
    end
end

-- Trigger callbacks
function security_manager._trigger_callbacks(event, data)
    for _, callback in ipairs(callbacks) do
        pcall(callback, event, data)
    end
end

-- Check if initialized
function security_manager.is_initialized()
    return initialized
end

return security_manager 