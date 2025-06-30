local SecurityManager = {}
SecurityManager.__index = SecurityManager

-- Constants
local MAX_PASSWORD_LENGTH = 128
local MIN_PASSWORD_LENGTH = 12
local MAX_API_KEY_LENGTH = 64
local MIN_API_KEY_LENGTH = 32
local SUPPORTED_ENCRYPTION_ALGORITHMS = {
    "aes-256-gcm",
    "chacha20-poly1305",
    "xchacha20-poly1305"
}
local SUPPORTED_HASH_ALGORITHMS = {
    "sha2-256",
    "sha2-512",
    "blake2b-256",
    "blake2b-512"
}
local DEFAULT_KEY_ROTATION_PERIOD = 30 * 24 * 60 * 60  -- 30 days in seconds

function SecurityManager.new()
    local self = setmetatable({}, SecurityManager)
    self.initialized = false
    self.blockchainManager = nil
    self.quantumManager = nil
    self.apiKeys = {}
    self.encryptionKeys = {}
    return self
end

function SecurityManager:initialize(blockchainManager, quantumManager, options)
    if self.initialized then
        return nil, "SecurityManager already initialized"
    end

    if not blockchainManager then
        return nil, "BlockchainManager is required"
    end

    if not quantumManager then
        return nil, "QuantumManager is required"
    end

    options = options or {}
    self.blockchainManager = blockchainManager
    self.quantumManager = quantumManager
    self.keyRotationPeriod = options.keyRotationPeriod or DEFAULT_KEY_ROTATION_PERIOD

    self.initialized = true
    return true
end

function SecurityManager:generateAPIKey(name, permissions)
    if not self.initialized then
        return nil, "SecurityManager not initialized"
    end
    if not name then
        return nil, "API key name is required"
    end
    if not permissions then
        return nil, "API key permissions are required"
    end

    -- Generate quantum-resistant key pair
    local keyPair = self.quantumManager:generateKeyPair()
    if not keyPair then
        return nil, "Failed to generate key pair"
    end

    -- Create API key
    local apiKey = {
        id = self:_generateKeyId(),
        name = name,
        permissions = permissions,
        publicKey = keyPair.publicKey,
        privateKey = keyPair.privateKey,
        created_at = os.time(),
        expires_at = os.time() + self.keyRotationPeriod
    }

    -- Store API key
    self.apiKeys[apiKey.id] = apiKey

    return apiKey
end

function SecurityManager:validateAPIKey(apiKeyId, signature, message)
    if not self.initialized then
        return nil, "SecurityManager not initialized"
    end
    if not apiKeyId then
        return nil, "API key ID is required"
    end
    if not signature then
        return nil, "Signature is required"
    end
    if not message then
        return nil, "Message is required"
    end

    -- Get API key
    local apiKey = self.apiKeys[apiKeyId]
    if not apiKey then
        return nil, "API key not found"
    end

    -- Check expiration
    if os.time() > apiKey.expires_at then
        return nil, "API key expired"
    end

    -- Verify signature
    return self.quantumManager:verifySignature(apiKey.publicKey, signature, message)
end

function SecurityManager:rotateAPIKey(apiKeyId)
    if not self.initialized then
        return nil, "SecurityManager not initialized"
    end
    if not apiKeyId then
        return nil, "API key ID is required"
    end

    -- Get API key
    local apiKey = self.apiKeys[apiKeyId]
    if not apiKey then
        return nil, "API key not found"
    end

    -- Generate new key pair
    local keyPair = self.quantumManager:generateKeyPair()
    if not keyPair then
        return nil, "Failed to generate key pair"
    end

    -- Update API key
    apiKey.publicKey = keyPair.publicKey
    apiKey.privateKey = keyPair.privateKey
    apiKey.created_at = os.time()
    apiKey.expires_at = os.time() + self.keyRotationPeriod

    return apiKey
end

function SecurityManager:encryptData(data, algorithm)
    if not self.initialized then
        return nil, "SecurityManager not initialized"
    end
    if not data then
        return nil, "Data is required"
    end

    algorithm = algorithm or "aes-256-gcm"
    if not self:_isValidEncryptionAlgorithm(algorithm) then
        return nil, "Unsupported encryption algorithm"
    end

    -- Generate encryption key
    local key = self:_generateEncryptionKey(algorithm)
    if not key then
        return nil, "Failed to generate encryption key"
    end

    -- Encrypt data
    local encrypted = self:_encrypt(data, key, algorithm)
    if not encrypted then
        return nil, "Failed to encrypt data"
    end

    return {
        data = encrypted,
        key = key,
        algorithm = algorithm
    }
end

function SecurityManager:decryptData(encryptedData, key, algorithm)
    if not self.initialized then
        return nil, "SecurityManager not initialized"
    end
    if not encryptedData then
        return nil, "Encrypted data is required"
    end
    if not key then
        return nil, "Encryption key is required"
    end

    algorithm = algorithm or "aes-256-gcm"
    if not self:_isValidEncryptionAlgorithm(algorithm) then
        return nil, "Unsupported encryption algorithm"
    end

    -- Decrypt data
    local decrypted = self:_decrypt(encryptedData, key, algorithm)
    if not decrypted then
        return nil, "Failed to decrypt data"
    end

    return decrypted
end

function SecurityManager:hashData(data, algorithm)
    if not self.initialized then
        return nil, "SecurityManager not initialized"
    end
    if not data then
        return nil, "Data is required"
    end

    algorithm = algorithm or "sha2-256"
    if not self:_isValidHashAlgorithm(algorithm) then
        return nil, "Unsupported hash algorithm"
    end

    -- Hash data
    local hash = self:_hash(data, algorithm)
    if not hash then
        return nil, "Failed to hash data"
    end

    return {
        hash = hash,
        algorithm = algorithm
    }
end

function SecurityManager:validatePassword(password)
    if not password then
        return nil, "Password is required"
    end

    if #password < MIN_PASSWORD_LENGTH then
        return nil, "Password too short"
    end

    if #password > MAX_PASSWORD_LENGTH then
        return nil, "Password too long"
    end

    -- Check for common patterns
    if self:_isCommonPassword(password) then
        return nil, "Password is too common"
    end

    -- Check for required character types
    if not self:_hasRequiredCharacters(password) then
        return nil, "Password must contain uppercase, lowercase, numbers, and special characters"
    end

    return true
end

-- Private methods
function SecurityManager:_generateKeyId()
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would generate a unique key ID
    return "key_" .. os.time()
end

function SecurityManager:_generateEncryptionKey(algorithm)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would generate an encryption key
    return "key_" .. algorithm .. "_" .. os.time()
end

function SecurityManager:_isValidEncryptionAlgorithm(algorithm)
    for _, validAlgorithm in ipairs(SUPPORTED_ENCRYPTION_ALGORITHMS) do
        if algorithm == validAlgorithm then
            return true
        end
    end
    return false
end

function SecurityManager:_isValidHashAlgorithm(algorithm)
    for _, validAlgorithm in ipairs(SUPPORTED_HASH_ALGORITHMS) do
        if algorithm == validAlgorithm then
            return true
        end
    end
    return false
end

function SecurityManager:_encrypt(data, key, algorithm)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would encrypt data using the specified algorithm
    return "encrypted_" .. data
end

function SecurityManager:_decrypt(encryptedData, key, algorithm)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would decrypt data using the specified algorithm
    return encryptedData:gsub("encrypted_", "")
end

function SecurityManager:_hash(data, algorithm)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would hash data using the specified algorithm
    return "hash_" .. data
end

function SecurityManager:_isCommonPassword(password)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would check against a list of common passwords
    local commonPasswords = {
        "password123",
        "12345678",
        "qwerty123"
    }
    for _, common in ipairs(commonPasswords) do
        if password == common then
            return true
        end
    end
    return false
end

function SecurityManager:_hasRequiredCharacters(password)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would check for required character types
    return password:match("%u") and  -- uppercase
           password:match("%l") and  -- lowercase
           password:match("%d") and  -- digit
           password:match("%p")      -- punctuation
end

return SecurityManager 