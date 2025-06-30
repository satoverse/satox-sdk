local ffi = require("ffi")

-- Load the native library
local lib = ffi.load("satox_quantum")

-- Define C types
ffi.cdef[[
    typedef struct satox_quantum_manager satox_quantum_manager_t;
    typedef struct satox_key_pair {
        uint8_t* public_key;
        size_t public_key_len;
        uint8_t* private_key;
        size_t private_key_len;
    } satox_key_pair_t;

    // Function declarations
    satox_quantum_manager_t* satox_quantum_manager_create(void);
    void satox_quantum_manager_destroy(satox_quantum_manager_t* manager);
    int satox_quantum_manager_initialize(satox_quantum_manager_t* manager);
    int satox_quantum_manager_shutdown(satox_quantum_manager_t* manager);
    int satox_quantum_manager_generate_key_pair(satox_quantum_manager_t* manager, satox_key_pair_t* key_pair);
    int satox_quantum_manager_encrypt(satox_quantum_manager_t* manager, const uint8_t* public_key, size_t public_key_len,
                                    const uint8_t* data, size_t data_len, uint8_t** encrypted, size_t* encrypted_len);
    int satox_quantum_manager_decrypt(satox_quantum_manager_t* manager, const uint8_t* private_key, size_t private_key_len,
                                    const uint8_t* encrypted, size_t encrypted_len, uint8_t** decrypted, size_t* decrypted_len);
    int satox_quantum_manager_sign(satox_quantum_manager_t* manager, const uint8_t* private_key, size_t private_key_len,
                                 const uint8_t* data, size_t data_len, uint8_t** signature, size_t* signature_len);
    int satox_quantum_manager_verify(satox_quantum_manager_t* manager, const uint8_t* public_key, size_t public_key_len,
                                   const uint8_t* data, size_t data_len, const uint8_t* signature, size_t signature_len);
    int satox_quantum_manager_generate_random_bytes(satox_quantum_manager_t* manager, size_t length, uint8_t** random_bytes);
    const char* satox_quantum_manager_get_version(satox_quantum_manager_t* manager);
    void satox_quantum_manager_free_buffer(uint8_t* buffer);
]]

-- QuantumManager class for Satox SDK
local QuantumManager = {}
QuantumManager.__index = QuantumManager

function QuantumManager.new()
    local self = setmetatable({}, QuantumManager)
    self.initialized = false
    self.manager = lib.satox_quantum_manager_create()
    if self.manager == nil then
        error("Failed to create quantum manager")
    end
    return self
end

function QuantumManager:initialize()
    local result = lib.satox_quantum_manager_initialize(self.manager)
    if result ~= 0 then
        error("Failed to initialize quantum manager")
    end
end

function QuantumManager:shutdown()
    local result = lib.satox_quantum_manager_shutdown(self.manager)
    if result ~= 0 then
        error("Failed to shutdown quantum manager")
    end
end

function QuantumManager:generate_key_pair()
    local key_pair = ffi.new("satox_key_pair_t")
    local result = lib.satox_quantum_manager_generate_key_pair(self.manager, key_pair)
    if result ~= 0 then
        error("Failed to generate key pair")
    end

    local public_key = ffi.string(key_pair.public_key, key_pair.public_key_len)
    local private_key = ffi.string(key_pair.private_key, key_pair.private_key_len)

    lib.satox_quantum_manager_free_buffer(key_pair.public_key)
    lib.satox_quantum_manager_free_buffer(key_pair.private_key)

    return {
        public_key = public_key,
        private_key = private_key
    }
end

function QuantumManager:encrypt(public_key, data)
    local encrypted = ffi.new("uint8_t*[1]")
    local encrypted_len = ffi.new("size_t[1]")

    local result = lib.satox_quantum_manager_encrypt(
        self.manager,
        public_key,
        #public_key,
        data,
        #data,
        encrypted,
        encrypted_len
    )

    if result ~= 0 then
        error("Failed to encrypt data")
    end

    local encrypted_data = ffi.string(encrypted[0], encrypted_len[0])
    lib.satox_quantum_manager_free_buffer(encrypted[0])

    return encrypted_data
end

function QuantumManager:decrypt(private_key, encrypted)
    local decrypted = ffi.new("uint8_t*[1]")
    local decrypted_len = ffi.new("size_t[1]")

    local result = lib.satox_quantum_manager_decrypt(
        self.manager,
        private_key,
        #private_key,
        encrypted,
        #encrypted,
        decrypted,
        decrypted_len
    )

    if result ~= 0 then
        error("Failed to decrypt data")
    end

    local decrypted_data = ffi.string(decrypted[0], decrypted_len[0])
    lib.satox_quantum_manager_free_buffer(decrypted[0])

    return decrypted_data
end

function QuantumManager:sign(private_key, data)
    local signature = ffi.new("uint8_t*[1]")
    local signature_len = ffi.new("size_t[1]")

    local result = lib.satox_quantum_manager_sign(
        self.manager,
        private_key,
        #private_key,
        data,
        #data,
        signature,
        signature_len
    )

    if result ~= 0 then
        error("Failed to sign data")
    end

    local signature_data = ffi.string(signature[0], signature_len[0])
    lib.satox_quantum_manager_free_buffer(signature[0])

    return signature_data
end

function QuantumManager:verify(public_key, data, signature)
    local result = lib.satox_quantum_manager_verify(
        self.manager,
        public_key,
        #public_key,
        data,
        #data,
        signature,
        #signature
    )

    return result == 0
end

function QuantumManager:generate_random_bytes(length)
    local random_bytes = ffi.new("uint8_t*[1]")
    local result = lib.satox_quantum_manager_generate_random_bytes(self.manager, length, random_bytes)

    if result ~= 0 then
        error("Failed to generate random bytes")
    end

    local random_data = ffi.string(random_bytes[0], length)
    lib.satox_quantum_manager_free_buffer(random_bytes[0])

    return random_data
end

function QuantumManager:get_version()
    return ffi.string(lib.satox_quantum_manager_get_version(self.manager))
end

function QuantumManager:__gc()
    if self.manager then
        lib.satox_quantum_manager_destroy(self.manager)
        self.manager = nil
    end
end

return QuantumManager 