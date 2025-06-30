/**
 * @file satox_sdk.c
 * @brief C interface for Satox SDK Lua binding
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for SDK functions
extern int satox_sdk_initialize(void);
extern void satox_sdk_shutdown(void);
extern const char* satox_sdk_get_version(void);
extern int satox_sdk_connect(void);
extern int satox_sdk_disconnect(void);

// Core manager functions
extern void* satox_core_manager_create(void);
extern void satox_core_manager_destroy(void* handle);
extern int satox_core_manager_initialize(void* handle, const char* config_json);
extern int satox_core_manager_start(void* handle);
extern int satox_core_manager_stop(void* handle);

// Wallet manager functions
extern void* satox_wallet_manager_create(void);
extern void satox_wallet_manager_destroy(void* handle);
extern int satox_wallet_manager_initialize(void* handle, const char* config_json);
extern int satox_wallet_manager_start(void* handle);
extern int satox_wallet_manager_stop(void* handle);
extern char* satox_wallet_manager_create_wallet(void* handle, const char* name);
extern char* satox_wallet_manager_get_wallet_info(void* handle, const char* address);

// Security manager functions
extern void* satox_security_manager_create(void);
extern void satox_security_manager_destroy(void* handle);
extern int satox_security_manager_initialize(void* handle, const char* config_json);
extern int satox_security_manager_start(void* handle);
extern int satox_security_manager_stop(void* handle);
extern char* satox_security_manager_generate_keypair(void* handle);
extern char* satox_security_manager_sign_data(void* handle, const char* data, const char* private_key);
extern int satox_security_manager_verify_signature(void* handle, const char* data, const char* signature, const char* public_key);

// Asset manager functions
extern void* satox_asset_manager_create(void);
extern void satox_asset_manager_destroy(void* handle);
extern int satox_asset_manager_initialize(void* handle, const char* config_json);
extern int satox_asset_manager_start(void* handle);
extern int satox_asset_manager_stop(void* handle);
extern char* satox_asset_manager_create_asset(void* handle, const char* config_json, const char* owner_address);
extern char* satox_asset_manager_get_asset_info(void* handle, const char* asset_id);

// NFT manager functions
extern void* satox_nft_manager_create(void);
extern void satox_nft_manager_destroy(void* handle);
extern int satox_nft_manager_initialize(void* handle, const char* config_json);
extern int satox_nft_manager_start(void* handle);
extern int satox_nft_manager_stop(void* handle);
extern char* satox_nft_manager_create_nft(void* handle, const char* asset_id, const char* metadata_json, const char* owner_address);
extern char* satox_nft_manager_get_nft_info(void* handle, const char* nft_id);

// Blockchain manager functions
extern void* satox_blockchain_manager_create(void);
extern void satox_blockchain_manager_destroy(void* handle);
extern int satox_blockchain_manager_initialize(void* handle, const char* config_json);
extern int satox_blockchain_manager_start(void* handle);
extern int satox_blockchain_manager_stop(void* handle);
extern char* satox_blockchain_manager_get_block_info(void* handle, const char* hash);
extern char* satox_blockchain_manager_get_transaction_info(void* handle, const char* txid);

// IPFS manager functions
extern void* satox_ipfs_manager_create(void);
extern void satox_ipfs_manager_destroy(void* handle);
extern int satox_ipfs_manager_initialize(void* handle, const char* config_json);
extern int satox_ipfs_manager_start(void* handle);
extern int satox_ipfs_manager_stop(void* handle);
extern char* satox_ipfs_manager_upload_file(void* handle, const char* file_path);
extern char* satox_ipfs_manager_get_file_info(void* handle, const char* hash);

// Network manager functions
extern void* satox_network_manager_create(void);
extern void satox_network_manager_destroy(void* handle);
extern int satox_network_manager_initialize(void* handle, const char* config_json);
extern int satox_network_manager_start(void* handle);
extern int satox_network_manager_stop(void* handle);
extern int satox_network_manager_connect(void* handle);
extern int satox_network_manager_disconnect(void* handle);

// Database manager functions
extern void* satox_database_manager_create(void);
extern void satox_database_manager_destroy(void* handle);
extern int satox_database_manager_initialize(void* handle, const char* config_json);
extern int satox_database_manager_start(void* handle);
extern int satox_database_manager_stop(void* handle);
extern char* satox_database_manager_create_database(void* handle, const char* name);
extern char* satox_database_manager_list_databases(void* handle);

// Lua function: satox.initialize()
static int l_satox_initialize(lua_State* L) {
    int result = satox_sdk_initialize();
    lua_pushboolean(L, result == 0);
    return 1;
}

// Lua function: satox.shutdown()
static int l_satox_shutdown(lua_State* L) {
    satox_sdk_shutdown();
    return 0;
}

// Lua function: satox.get_version()
static int l_satox_get_version(lua_State* L) {
    const char* version = satox_sdk_get_version();
    lua_pushstring(L, version ? version : "Unknown");
    return 1;
}

// Lua function: satox.connect()
static int l_satox_connect(lua_State* L) {
    int result = satox_sdk_connect();
    lua_pushboolean(L, result == 0);
    return 1;
}

// Lua function: satox.disconnect()
static int l_satox_disconnect(lua_State* L) {
    int result = satox_sdk_disconnect();
    lua_pushboolean(L, result == 0);
    return 1;
}

// Lua function: satox.new_sdk(config)
static int l_satox_new_sdk(lua_State* L) {
    // This would create a new SDK instance
    // For now, just return a table with the SDK interface
    lua_newtable(L);
    
    // Add methods to the table
    lua_pushcfunction(L, l_satox_initialize);
    lua_setfield(L, -2, "initialize");
    
    lua_pushcfunction(L, l_satox_shutdown);
    lua_setfield(L, -2, "shutdown");
    
    lua_pushcfunction(L, l_satox_get_version);
    lua_setfield(L, -2, "get_version");
    
    lua_pushcfunction(L, l_satox_connect);
    lua_setfield(L, -2, "connect");
    
    lua_pushcfunction(L, l_satox_disconnect);
    lua_setfield(L, -2, "disconnect");
    
    return 1;
}

// Module registration
static const luaL_Reg satox_functions[] = {
    {"initialize", l_satox_initialize},
    {"shutdown", l_satox_shutdown},
    {"get_version", l_satox_get_version},
    {"connect", l_satox_connect},
    {"disconnect", l_satox_disconnect},
    {"new_sdk", l_satox_new_sdk},
    {NULL, NULL}
};

// Module loader
int luaopen_satox_sdk(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, satox_functions, 0);
    
    // Add version information
    lua_pushstring(L, "1.0.0");
    lua_setfield(L, -2, "VERSION");
    
    lua_pushstring(L, "2025-06-30");
    lua_setfield(L, -2, "BUILD_DATE");
    
    return 1;
} 