local SecurityManager = {}
SecurityManager.__index = SecurityManager

-- Load required libraries
local crypto = require("crypto")

function SecurityManager.new()
    local self = setmetatable({}, SecurityManager)
    self.initialized = false
    self.encryptionEnabled = false
    self.keys = {}
    return self
end

function SecurityManager:initialize()
    if self.initialized then
        error("SecurityManager already initialized")
    end
    self.initialized = true
end

function SecurityManager:isInitialized()
    return self.initialized
end

function SecurityManager:enableEncryption()
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    self.encryptionEnabled = true
end

function SecurityManager:disableEncryption()
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    self.encryptionEnabled = false
end

function SecurityManager:isEncryptionEnabled()
    return self.encryptionEnabled
end

function SecurityManager:encrypt(data)
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    if not self.encryptionEnabled then
        error("Encryption not enabled")
    end
    if #self.keys == 0 then
        error("No encryption keys available")
    end
    
    -- Use the first available key for encryption
    local key = self.keys[1]
    local iv = crypto.random(16) -- Generate random IV
    local encrypted = crypto.encrypt("aes-256-gcm", data, key, iv)
    return encrypted, iv
end

function SecurityManager:decrypt(encrypted, iv)
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    if not self.encryptionEnabled then
        error("Encryption not enabled")
    end
    if #self.keys == 0 then
        error("No encryption keys available")
    end
    
    -- Try each key until decryption succeeds
    for _, key in ipairs(self.keys) do
        local success, decrypted = pcall(function()
            return crypto.decrypt("aes-256-gcm", encrypted, key, iv)
        end)
        if success then
            return decrypted
        end
    end
    error("Decryption failed with all available keys")
end

function SecurityManager:generateKey()
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    local key = crypto.random(32) -- Generate 256-bit key
    table.insert(self.keys, key)
    return key
end

function SecurityManager:addKey(key)
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    if type(key) ~= "string" or #key ~= 32 then
        error("Invalid key format")
    end
    table.insert(self.keys, key)
end

function SecurityManager:removeKey(key)
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    for i, k in ipairs(self.keys) do
        if k == key then
            table.remove(self.keys, i)
            return
        end
    end
    error("Key not found")
end

function SecurityManager:getKeys()
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    return self.keys
end

function SecurityManager:hash(data)
    if not self.initialized then
        error("SecurityManager not initialized")
    end
    return crypto.digest("sha256", data)
end

function SecurityManager:finalize()
    -- Implementation
end

function SecurityManager:someStub()
    -- Implementation
end

return SecurityManager 