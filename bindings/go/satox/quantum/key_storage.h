#ifndef SATOX_KEY_STORAGE_H
#define SATOX_KEY_STORAGE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Key metadata structure
struct satox_key_metadata {
    char* algorithm;
    int64_t creation_time;
    int64_t expiration;
};

// Initialize the key storage system
int satox_key_storage_initialize();

// Shutdown the key storage system
int satox_key_storage_shutdown();

// Store a key with metadata
int satox_key_storage_store_key(
    const char* key_id,
    const uint8_t* key,
    size_t key_length,
    const struct satox_key_metadata* metadata,
    const char** access_levels,
    size_t access_levels_length,
    const char** tags,
    size_t tags_length
);

// Retrieve a key by ID
int satox_key_storage_retrieve_key(
    const char* key_id,
    uint8_t** key,
    size_t* key_length
);

// Delete a key by ID
int satox_key_storage_delete_key(const char* key_id);

// Update a key's metadata
int satox_key_storage_update_key(
    const char* key_id,
    const struct satox_key_metadata* metadata,
    const char** access_levels,
    size_t access_levels_length,
    const char** tags,
    size_t tags_length
);

// Get a key's metadata
int satox_key_storage_get_key_metadata(
    const char* key_id,
    struct satox_key_metadata* metadata,
    char*** access_levels,
    size_t* access_levels_length,
    char*** tags,
    size_t* tags_length
);

// Rotate a key
int satox_key_storage_rotate_key(const char* key_id);

// Reencrypt a key
int satox_key_storage_reencrypt_key(const char* key_id);

// Validate a key
int satox_key_storage_validate_key(
    const char* key_id,
    int* is_valid
);

// Check if a key is expired
int satox_key_storage_is_key_expired(
    const char* key_id,
    int* is_expired
);

// Add an access level to a key
int satox_key_storage_add_key_access_level(
    const char* key_id,
    const char* access_level
);

// Remove an access level from a key
int satox_key_storage_remove_key_access_level(
    const char* key_id,
    const char* access_level
);

// Count the number of stored keys
int satox_key_storage_count_keys(size_t* count);

// List all key IDs
int satox_key_storage_list_key_ids(
    char*** key_ids,
    size_t* key_ids_length
);

// Free a buffer allocated by the key storage system
void satox_key_storage_free_buffer(void* buffer);

#ifdef __cplusplus
}
#endif

#endif // SATOX_KEY_STORAGE_H 