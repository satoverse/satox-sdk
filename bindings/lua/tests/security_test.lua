local luaunit = require('luaunit')
local SecurityManager = require('../security')
local BlockchainManager = require('../blockchain')
local QuantumManager = require('../quantum')

TestSecurityManager = {}

function TestSecurityManager:setUp()
    self.blockchainManager = BlockchainManager.new()
    self.blockchainManager:initialize()
    
    self.quantumManager = QuantumManager.new()
    self.quantumManager:initialize()
    
    self.securityManager = SecurityManager.new()
    self.securityManager:initialize(self.blockchainManager, self.quantumManager)
end

function TestSecurityManager:testInitialization()
    local manager = SecurityManager.new()
    luaunit.assertFalse(manager.initialized)
    
    -- Test initialization without required managers
    luaunit.assertError(function()
        manager:initialize(nil, self.quantumManager)
    end, "BlockchainManager is required")
    
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager, nil)
    end, "QuantumManager is required")
    
    -- Test proper initialization
    manager:initialize(self.blockchainManager, self.quantumManager)
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize(self.blockchainManager, self.quantumManager)
    end, "SecurityManager already initialized")
end

function TestSecurityManager:testAPIKeyManagement()
    -- Generate API key
    local apiKey = self.securityManager:generateAPIKey("test_key", { "read", "write" })
    luaunit.assertNotNil(apiKey)
    luaunit.assertNotNil(apiKey.id)
    luaunit.assertNotNil(apiKey.name)
    luaunit.assertNotNil(apiKey.permissions)
    luaunit.assertNotNil(apiKey.publicKey)
    luaunit.assertNotNil(apiKey.privateKey)
    luaunit.assertNotNil(apiKey.created_at)
    luaunit.assertNotNil(apiKey.expires_at)
    
    -- Test without name
    luaunit.assertError(function()
        self.securityManager:generateAPIKey(nil, { "read" })
    end, "API key name is required")
    
    -- Test without permissions
    luaunit.assertError(function()
        self.securityManager:generateAPIKey("test_key", nil)
    end, "API key permissions are required")
    
    -- Validate API key
    local message = "test message"
    local signature = self.quantumManager:signMessage(apiKey.privateKey, message)
    local isValid = self.securityManager:validateAPIKey(apiKey.id, signature, message)
    luaunit.assertTrue(isValid)
    
    -- Test with invalid signature
    luaunit.assertFalse(self.securityManager:validateAPIKey(apiKey.id, "invalid", message))
    
    -- Test with non-existent API key
    luaunit.assertError(function()
        self.securityManager:validateAPIKey("non_existent", signature, message)
    end, "API key not found")
    
    -- Rotate API key
    local rotatedKey = self.securityManager:rotateAPIKey(apiKey.id)
    luaunit.assertNotNil(rotatedKey)
    luaunit.assertEquals(rotatedKey.id, apiKey.id)
    luaunit.assertNotEquals(rotatedKey.publicKey, apiKey.publicKey)
    luaunit.assertNotEquals(rotatedKey.privateKey, apiKey.privateKey)
    
    -- Test rotation of non-existent API key
    luaunit.assertError(function()
        self.securityManager:rotateAPIKey("non_existent")
    end, "API key not found")
end

function TestSecurityManager:testEncryption()
    -- Encrypt data
    local data = "test data"
    local result = self.securityManager:encryptData(data)
    luaunit.assertNotNil(result)
    luaunit.assertNotNil(result.data)
    luaunit.assertNotNil(result.key)
    luaunit.assertNotNil(result.algorithm)
    
    -- Test without data
    luaunit.assertError(function()
        self.securityManager:encryptData(nil)
    end, "Data is required")
    
    -- Test with invalid algorithm
    luaunit.assertError(function()
        self.securityManager:encryptData(data, "invalid")
    end, "Unsupported encryption algorithm")
    
    -- Decrypt data
    local decrypted = self.securityManager:decryptData(result.data, result.key, result.algorithm)
    luaunit.assertEquals(decrypted, data)
    
    -- Test without encrypted data
    luaunit.assertError(function()
        self.securityManager:decryptData(nil, result.key, result.algorithm)
    end, "Encrypted data is required")
    
    -- Test without key
    luaunit.assertError(function()
        self.securityManager:decryptData(result.data, nil, result.algorithm)
    end, "Encryption key is required")
end

function TestSecurityManager:testHashing()
    -- Hash data
    local data = "test data"
    local result = self.securityManager:hashData(data)
    luaunit.assertNotNil(result)
    luaunit.assertNotNil(result.hash)
    luaunit.assertNotNil(result.algorithm)
    
    -- Test without data
    luaunit.assertError(function()
        self.securityManager:hashData(nil)
    end, "Data is required")
    
    -- Test with invalid algorithm
    luaunit.assertError(function()
        self.securityManager:hashData(data, "invalid")
    end, "Unsupported hash algorithm")
end

function TestSecurityManager:testPasswordValidation()
    -- Test valid password
    local validPassword = "Test123!@#"
    luaunit.assertTrue(self.securityManager:validatePassword(validPassword))
    
    -- Test without password
    luaunit.assertError(function()
        self.securityManager:validatePassword(nil)
    end, "Password is required")
    
    -- Test too short password
    luaunit.assertError(function()
        self.securityManager:validatePassword("short")
    end, "Password too short")
    
    -- Test too long password
    local longPassword = string.rep("a", 129)
    luaunit.assertError(function()
        self.securityManager:validatePassword(longPassword)
    end, "Password too long")
    
    -- Test common password
    luaunit.assertError(function()
        self.securityManager:validatePassword("password123")
    end, "Password is too common")
    
    -- Test missing character types
    luaunit.assertError(function()
        self.securityManager:validatePassword("lowercase123")
    end, "Password must contain uppercase, lowercase, numbers, and special characters")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 