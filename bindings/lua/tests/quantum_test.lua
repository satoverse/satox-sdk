local satox = require("satox")
local quantum = require("satox.quantum")
local luaunit = require("luaunit")

TestQuantum = {}

function TestQuantum:setUp()
    self.manager = quantum.QuantumManager.new()
end

function TestQuantum:testQuantumManagerInitialization()
    luaunit.assertNotNil(self.manager)
    self.manager:initialize()
    luaunit.assertTrue(self.manager:isInitialized())
end

function TestQuantum:testCRYSTALSKyberEncryption()
    self.manager:initialize()

    local message = "Test message"
    local publicKey, privateKey = self.manager:generateKeyPair()
    luaunit.assertNotNil(publicKey)
    luaunit.assertNotNil(privateKey)

    local encrypted = self.manager:encrypt(message, publicKey)
    luaunit.assertNotNil(encrypted)

    local decrypted = self.manager:decrypt(encrypted, privateKey)
    luaunit.assertEquals(message, decrypted)
end

function TestQuantum:testNTRUEncryption()
    self.manager:initialize()

    local message = "Test message"
    local publicKey, privateKey = self.manager:generateNTRUKeyPair()
    luaunit.assertNotNil(publicKey)
    luaunit.assertNotNil(privateKey)

    local encrypted = self.manager:encryptNTRU(message, publicKey)
    luaunit.assertNotNil(encrypted)

    local decrypted = self.manager:decryptNTRU(encrypted, privateKey)
    luaunit.assertEquals(message, decrypted)
end

function TestQuantum:testHybridEncryption()
    self.manager:initialize()

    local message = "Test message"
    local publicKey, privateKey = self.manager:generateHybridKeyPair()
    luaunit.assertNotNil(publicKey)
    luaunit.assertNotNil(privateKey)

    local encrypted = self.manager:encryptHybrid(message, publicKey)
    luaunit.assertNotNil(encrypted)

    local decrypted = self.manager:decryptHybrid(encrypted, privateKey)
    luaunit.assertEquals(message, decrypted)
end

function TestQuantum:testKeyStorage()
    self.manager:initialize()

    local publicKey, privateKey = self.manager:generateKeyPair()
    luaunit.assertNotNil(publicKey)
    luaunit.assertNotNil(privateKey)

    self.manager:storeKeyPair("test-key", publicKey, privateKey)
    local storedKeys = self.manager:retrieveKeyPair("test-key")
    
    luaunit.assertNotNil(storedKeys)
    luaunit.assertEquals(publicKey, storedKeys.publicKey)
    luaunit.assertEquals(privateKey, storedKeys.privateKey)
end

TestQuantumManager = {}

function TestQuantumManager:setUp()
    self.quantumManager = quantum.QuantumManager.new()
    self.quantumManager:initialize()
end

function TestQuantumManager:testInitialization()
    local manager = quantum.QuantumManager.new()
    luaunit.assertFalse(manager.initialized)
    
    manager:initialize()
    luaunit.assertTrue(manager.initialized)
    
    -- Test double initialization
    luaunit.assertError(function()
        manager:initialize()
    end, "QuantumManager already initialized")
end

function TestQuantumManager:testGenerateKeyPair()
    -- Test NTRU
    local publicKey, privateKey = self.quantumManager:generateKeyPair("NTRU")
    luaunit.assertEquals(publicKey.algorithm, "NTRU")
    luaunit.assertEquals(privateKey.algorithm, "NTRU")
    luaunit.assertEquals(publicKey.params.N, 509)
    luaunit.assertEquals(publicKey.params.p, 3)
    luaunit.assertEquals(publicKey.params.q, 2048)
    
    -- Test BIKE
    publicKey, privateKey = self.quantumManager:generateKeyPair("BIKE")
    luaunit.assertEquals(publicKey.algorithm, "BIKE")
    luaunit.assertEquals(privateKey.algorithm, "BIKE")
    luaunit.assertEquals(publicKey.params.R, 12323)
    luaunit.assertEquals(publicKey.params.W, 142)
    luaunit.assertEquals(publicKey.params.T, 134)
    
    -- Test unsupported algorithm
    luaunit.assertError(function()
        self.quantumManager:generateKeyPair("UNSUPPORTED")
    end, "Unsupported algorithm: UNSUPPORTED")
end

function TestQuantumManager:testEncryptDecrypt()
    -- Generate key pair
    local publicKey, privateKey = self.quantumManager:generateKeyPair("NTRU")
    
    -- Test data
    local testData = {message = "Hello, World!"}
    
    -- Encrypt data
    local encryptedData = self.quantumManager:encrypt(publicKey, testData)
    luaunit.assertEquals(encryptedData.algorithm, "NTRU")
    luaunit.assertNotNil(encryptedData.data)
    
    -- Decrypt data
    local decryptedData = self.quantumManager:decrypt(privateKey, encryptedData)
    luaunit.assertEquals(decryptedData.message, testData.message)
    
    -- Test invalid public key
    luaunit.assertError(function()
        self.quantumManager:encrypt(nil, testData)
    end, "Invalid public key")
    
    -- Test invalid private key
    luaunit.assertError(function()
        self.quantumManager:decrypt(nil, encryptedData)
    end, "Invalid private key")
    
    -- Test algorithm mismatch
    local wrongKey = {
        algorithm = "BIKE",
        params = {R = 12323, W = 142, T = 134},
        data = {}
    }
    luaunit.assertError(function()
        self.quantumManager:decrypt(wrongKey, encryptedData)
    end, "Algorithm mismatch between private key and encrypted data")
end

function TestQuantumManager:testSerializeDeserialize()
    -- Generate key pair
    local publicKey, privateKey = self.quantumManager:generateKeyPair("NTRU")
    
    -- Serialize keys
    local serializedPublic = self.quantumManager:serializeKey(publicKey)
    local serializedPrivate = self.quantumManager:serializeKey(privateKey)
    
    -- Verify serialized format
    luaunit.assertEquals(serializedPublic.algorithm, "NTRU")
    luaunit.assertEquals(serializedPublic.params.N, 509)
    luaunit.assertEquals(serializedPublic.params.p, 3)
    luaunit.assertEquals(serializedPublic.params.q, 2048)
    
    -- Deserialize keys
    local deserializedPublic = self.quantumManager:deserializeKey(serializedPublic)
    local deserializedPrivate = self.quantumManager:deserializeKey(serializedPrivate)
    
    -- Verify deserialized keys
    luaunit.assertEquals(deserializedPublic.algorithm, "NTRU")
    luaunit.assertEquals(deserializedPrivate.algorithm, "NTRU")
    
    -- Test invalid key
    luaunit.assertError(function()
        self.quantumManager:serializeKey(nil)
    end, "Invalid key")
    
    -- Test invalid serialized data
    luaunit.assertError(function()
        self.quantumManager:deserializeKey(nil)
    end, "Invalid serialized key")
end

os.exit(luaunit.LuaUnit.run()) 