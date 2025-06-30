--[[
 * @file satox_sdk.lua
 * @brief Main Lua binding for the Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
]]

local ffi = require("ffi")
local json = require("cjson")

-- Load the native library
local satox_lib = ffi.load("satox-sdk")

-- FFI declarations
ffi.cdef[[
    // Core SDK functions
    int satox_sdk_initialize();
    void satox_sdk_shutdown();
    const char* satox_sdk_get_version();
    int satox_sdk_connect();
    int satox_sdk_disconnect();
    
    // Core manager functions
    void* satox_core_manager_create();
    void satox_core_manager_destroy(void* handle);
    int satox_core_manager_initialize(void* handle, const char* config_json);
    int satox_core_manager_start(void* handle);
    int satox_core_manager_stop(void* handle);
    
    // Wallet manager functions
    void* satox_wallet_manager_create();
    void satox_wallet_manager_destroy(void* handle);
    int satox_wallet_manager_initialize(void* handle, const char* config_json);
    int satox_wallet_manager_start(void* handle);
    int satox_wallet_manager_stop(void* handle);
    char* satox_wallet_manager_create_wallet(void* handle, const char* name);
    char* satox_wallet_manager_get_wallet_info(void* handle, const char* address);
    
    // Security manager functions
    void* satox_security_manager_create();
    void satox_security_manager_destroy(void* handle);
    int satox_security_manager_initialize(void* handle, const char* config_json);
    int satox_security_manager_start(void* handle);
    int satox_security_manager_stop(void* handle);
    char* satox_security_manager_generate_keypair(void* handle);
    char* satox_security_manager_sign_data(void* handle, const char* data, const char* private_key);
    int satox_security_manager_verify_signature(void* handle, const char* data, const char* signature, const char* public_key);
    
    // Asset manager functions
    void* satox_asset_manager_create();
    void satox_asset_manager_destroy(void* handle);
    int satox_asset_manager_initialize(void* handle, const char* config_json);
    int satox_asset_manager_start(void* handle);
    int satox_asset_manager_stop(void* handle);
    char* satox_asset_manager_create_asset(void* handle, const char* config_json, const char* owner_address);
    char* satox_asset_manager_get_asset_info(void* handle, const char* asset_id);
    
    // NFT manager functions
    void* satox_nft_manager_create();
    void satox_nft_manager_destroy(void* handle);
    int satox_nft_manager_initialize(void* handle, const char* config_json);
    int satox_nft_manager_start(void* handle);
    int satox_nft_manager_stop(void* handle);
    char* satox_nft_manager_create_nft(void* handle, const char* asset_id, const char* metadata_json, const char* owner_address);
    char* satox_nft_manager_get_nft_info(void* handle, const char* nft_id);
    
    // Blockchain manager functions
    void* satox_blockchain_manager_create();
    void satox_blockchain_manager_destroy(void* handle);
    int satox_blockchain_manager_initialize(void* handle, const char* config_json);
    int satox_blockchain_manager_start(void* handle);
    int satox_blockchain_manager_stop(void* handle);
    char* satox_blockchain_manager_get_block_info(void* handle, const char* hash);
    char* satox_blockchain_manager_get_transaction_info(void* handle, const char* txid);
    
    // IPFS manager functions
    void* satox_ipfs_manager_create();
    void satox_ipfs_manager_destroy(void* handle);
    int satox_ipfs_manager_initialize(void* handle, const char* config_json);
    int satox_ipfs_manager_start(void* handle);
    int satox_ipfs_manager_stop(void* handle);
    char* satox_ipfs_manager_upload_file(void* handle, const char* file_path);
    char* satox_ipfs_manager_get_file_info(void* handle, const char* hash);
    
    // Network manager functions
    void* satox_network_manager_create();
    void satox_network_manager_destroy(void* handle);
    int satox_network_manager_initialize(void* handle, const char* config_json);
    int satox_network_manager_start(void* handle);
    int satox_network_manager_stop(void* handle);
    int satox_network_manager_connect(void* handle);
    int satox_network_manager_disconnect(void* handle);
    
    // Database manager functions
    void* satox_database_manager_create();
    void satox_database_manager_destroy(void* handle);
    int satox_database_manager_initialize(void* handle, const char* config_json);
    int satox_database_manager_start(void* handle);
    int satox_database_manager_stop(void* handle);
    char* satox_database_manager_create_database(void* handle, const char* name);
    char* satox_database_manager_list_databases(void* handle);
]]

-- Utility function to convert C string to Lua string
local function cstring_to_lua(cstr)
    if cstr == nil then
        return nil
    end
    return ffi.string(cstr)
end

-- Utility function to convert Lua string to C string
local function lua_to_cstring(str)
    if str == nil then
        return nil
    end
    return ffi.new("char[?]", #str + 1, str)
end

-- Default configuration
local DEFAULT_CONFIG = {
    data_dir = "./data",
    network = "mainnet",
    enable_mining = false,
    enable_sync = true,
    sync_interval_ms = 1000,
    mining_threads = 1,
    max_connections = 10,
    rpc_port = 7777,
    p2p_port = 67777,
    enable_rpc = true,
    enable_p2p = true,
    log_level = "info",
    log_file = "./logs/satox-sdk.log"
}

-- SatoxSDK class
local SatoxSDK = {}
SatoxSDK.__index = SatoxSDK

-- Constructor
function SatoxSDK.new(config)
    local self = setmetatable({}, SatoxSDK)
    self.config = config or DEFAULT_CONFIG
    self.is_running = false
    self.managers = {}
    return self
end

-- Initialize the SDK
function SatoxSDK:initialize()
    if self.is_running then
        error("SDK is already running")
    end
    
    -- Initialize core SDK
    local result = satox_lib.satox_sdk_initialize()
    if result ~= 0 then
        error("Failed to initialize SDK")
    end
    
    -- Create and initialize managers
    self.managers.core = satox_lib.satox_core_manager_create()
    self.managers.wallet = satox_lib.satox_wallet_manager_create()
    self.managers.security = satox_lib.satox_security_manager_create()
    self.managers.asset = satox_lib.satox_asset_manager_create()
    self.managers.nft = satox_lib.satox_nft_manager_create()
    self.managers.blockchain = satox_lib.satox_blockchain_manager_create()
    self.managers.ipfs = satox_lib.satox_ipfs_manager_create()
    self.managers.network = satox_lib.satox_network_manager_create()
    self.managers.database = satox_lib.satox_database_manager_create()
    
    -- Initialize all managers
    local config_json = json.encode(self.config)
    local config_cstr = lua_to_cstring(config_json)
    
    for name, manager in pairs(self.managers) do
        local init_func = satox_lib["satox_" .. name .. "_manager_initialize"]
        local result = init_func(manager, config_cstr)
        if result ~= 0 then
            error("Failed to initialize " .. name .. " manager")
        end
    end
    
    self.is_running = true
end

-- Start all services
function SatoxSDK:start()
    if not self.is_running then
        error("SDK is not initialized")
    end
    
    for name, manager in pairs(self.managers) do
        local start_func = satox_lib["satox_" .. name .. "_manager_start"]
        local result = start_func(manager)
        if result ~= 0 then
            error("Failed to start " .. name .. " manager")
        end
    end
end

-- Stop all services
function SatoxSDK:stop()
    if not self.is_running then
        return
    end
    
    for name, manager in pairs(self.managers) do
        local stop_func = satox_lib["satox_" .. name .. "_manager_stop"]
        stop_func(manager)
    end
    
    self.is_running = false
end

-- Connect to network
function SatoxSDK:connect()
    if not self.is_running then
        error("SDK is not running")
    end
    local result = satox_lib.satox_network_manager_connect(self.managers.network)
    if result ~= 0 then
        error("Failed to connect to network")
    end
end

-- Disconnect from network
function SatoxSDK:disconnect()
    if not self.is_running then
        return
    end
    satox_lib.satox_network_manager_disconnect(self.managers.network)
end

-- Shutdown the SDK
function SatoxSDK:shutdown()
    self:stop()
    satox_lib.satox_sdk_shutdown()
end

-- Get manager by name
function SatoxSDK:get_manager(name)
    return self.managers[name]
end

-- Get version
function SatoxSDK:get_version()
    local version_ptr = satox_lib.satox_sdk_get_version()
    return cstring_to_lua(version_ptr)
end

-- Get configuration
function SatoxSDK:get_config()
    return self.config
end

-- Check if running
function SatoxSDK:is_running()
    return self.is_running
end

-- Module exports
local M = {}

-- Create new SDK instance
function M.new(config)
    return SatoxSDK.new(config)
end

-- Get SDK version
function M.get_version()
    local version_ptr = satox_lib.satox_sdk_get_version()
    return cstring_to_lua(version_ptr)
end

-- Initialize SDK (global)
function M.initialize()
    local result = satox_lib.satox_sdk_initialize()
    if result ~= 0 then
        error("Failed to initialize SDK")
    end
end

-- Shutdown SDK (global)
function M.shutdown()
    satox_lib.satox_sdk_shutdown()
end

-- Default configuration
M.DEFAULT_CONFIG = DEFAULT_CONFIG

-- Version information
M.VERSION = "1.0.0"
M.BUILD_DATE = "2025-06-30"

return M 