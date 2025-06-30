--[[
Test file for Security Manager
]]

local luaunit = require('luaunit')
local SecurityManager = require('../satox/security/security_manager')

TestSecurityManager = {}

function TestSecurityManager:setUp()
    self.manager = SecurityManager.new()
    self.manager:initialize()
end

function TestSecurityManager:testInitialization()
    local manager = SecurityManager.new()
    luaunit.assertFalse(manager.initialized)
    
    manager:initialize()
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize()
    end, "SecurityManager already initialized")
end

function TestSecurityManager:testKeyPairGeneration()
    -- Test quantum-resistant key pair generation
    local privateKey, publicKey = self.manager:generateKeyPair()
    luaunit.assertNotNil(privateKey)
    luaunit.assertNotNil(publicKey)
    
    -- Test RSA key pair generation
    local rsaPrivateKey, rsaPublicKey = self.manager:generateRSAKeyPair()
    luaunit.assertNotNil(rsaPrivateKey)
    luaunit.assertNotNil(rsaPublicKey)
end

function TestSecurityManager:testEncryption()
    -- Generate key pair
    local privateKey, publicKey = self.manager:generateKeyPair()
    
    -- Test encryption
    local data = "test data"
    local encrypted = self.manager:encrypt(publicKey, data)
    luaunit.assertNotNil(encrypted)
    luaunit.assertNotEquals(encrypted, data)
    
    -- Test decryption
    local decrypted = self.manager:decrypt(privateKey, encrypted)
    luaunit.assertEquals(decrypted, data)
    
    -- Test invalid public key
    luaunit.assertError(function()
        self.manager:encrypt(nil, data)
    end, "Invalid public key")
    
    -- Test invalid data
    luaunit.assertError(function()
        self.manager:encrypt(publicKey, nil)
    end, "Invalid data")
end

function TestSecurityManager:testPasswordHashing()
    -- Test password hashing
    local password = "Test123!@#"
    local hash = self.manager:hashPassword(password)
    luaunit.assertNotNil(hash)
    luaunit.assertNotEquals(hash, password)
    
    -- Test password verification
    local isValid = self.manager:verifyPassword(password, hash)
    luaunit.assertTrue(isValid)
    
    -- Test invalid password
    luaunit.assertError(function()
        self.manager:hashPassword("weak")
    end, "Invalid password")
    
    -- Test invalid hash
    luaunit.assertError(function()
        self.manager:verifyPassword(password, nil)
    end, "Invalid hash")
end

function TestSecurityManager:testSessionTokens()
    -- Test token generation
    local userId = "user123"
    local metadata = { role = "admin" }
    local token = self.manager:generateSessionToken(userId, metadata)
    luaunit.assertNotNil(token)
    
    -- Test token verification
    local isValid = self.manager:verifySessionToken(token)
    luaunit.assertTrue(isValid)
    
    -- Test invalid user ID
    luaunit.assertError(function()
        self.manager:generateSessionToken(nil, metadata)
    end, "Invalid user ID")
    
    -- Test invalid token
    luaunit.assertError(function()
        self.manager:verifySessionToken(nil)
    end, "Invalid token")
end

function TestSecurityManager:testRSASigning()
    -- Generate RSA key pair
    local privateKey, publicKey = self.manager:generateRSAKeyPair()
    
    -- Test signing
    local data = "test data"
    local signature = self.manager:signRSA(privateKey, data)
    luaunit.assertNotNil(signature)
    
    -- Test signature verification
    local isValid = self.manager:verifyRSASignature(publicKey, data, signature)
    luaunit.assertTrue(isValid)
    
    -- Test invalid private key
    luaunit.assertError(function()
        self.manager:signRSA(nil, data)
    end, "Invalid private key")
    
    -- Test invalid data
    luaunit.assertError(function()
        self.manager:signRSA(privateKey, nil)
    end, "Invalid data")
    
    -- Test invalid signature
    luaunit.assertError(function()
        self.manager:verifyRSASignature(publicKey, data, nil)
    end, "Invalid signature")
end

function TestSecurityManager:testPolicyValidation()
    -- Test valid policy
    local policy = {
        minPasswordLength = 8,
        requireSpecialChars = true,
        requireNumbers = true,
        requireUppercase = true,
        requireLowercase = true,
        maxLoginAttempts = 3,
        lockoutDuration = 300
    }
    
    local isValid = self.manager:validatePolicy(policy)
    luaunit.assertTrue(isValid)
    
    -- Test invalid policy
    luaunit.assertError(function()
        self.manager:validatePolicy(nil)
    end, "Invalid policy")
    
    -- Test incomplete policy
    local incompletePolicy = {
        minPasswordLength = 8
    }
    
    luaunit.assertError(function()
        self.manager:validatePolicy(incompletePolicy)
    end, "Invalid policy")
end

function TestSecurityManager:testAuditEvents()
    -- Test audit event
    local event = {
        type = "login",
        userId = "user123",
        timestamp = os.time(),
        ip = "127.0.0.1",
        success = true
    }
    
    local success = self.manager:auditEvent(event)
    luaunit.assertTrue(success)
    
    -- Test invalid event
    luaunit.assertError(function()
        self.manager:auditEvent(nil)
    end, "Invalid event")
    
    -- Test incomplete event
    local incompleteEvent = {
        type = "login"
    }
    
    luaunit.assertError(function()
        self.manager:auditEvent(incompleteEvent)
    end, "Invalid event")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 