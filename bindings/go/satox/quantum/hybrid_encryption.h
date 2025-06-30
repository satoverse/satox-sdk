#ifndef SATOX_HYBRID_ENCRYPTION_H
#define SATOX_HYBRID_ENCRYPTION_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the hybrid encryption system
int satox_hybrid_encryption_initialize();

// Shutdown the hybrid encryption system
int satox_hybrid_encryption_shutdown();

// Generate a new key pair
int satox_hybrid_encryption_generate_key_pair(
    uint8_t** public_key,
    size_t* public_key_length,
    uint8_t** private_key,
    size_t* private_key_length
);

// Encrypt data using a public key
int satox_hybrid_encryption_encrypt(
    const uint8_t* public_key,
    size_t public_key_length,
    const uint8_t* data,
    size_t data_length,
    uint8_t** encrypted_data,
    size_t* encrypted_data_length
);

// Decrypt data using a private key
int satox_hybrid_encryption_decrypt(
    const uint8_t* private_key,
    size_t private_key_length,
    const uint8_t* encrypted_data,
    size_t encrypted_data_length,
    uint8_t** decrypted_data,
    size_t* decrypted_data_length
);

// Get the current session key
int satox_hybrid_encryption_get_session_key(
    uint8_t** session_key,
    size_t* session_key_length
);

// Rotate the session key
int satox_hybrid_encryption_rotate_session_key();

// Get the version of the hybrid encryption system
int satox_hybrid_encryption_get_version(char** version);

// Free a buffer allocated by the hybrid encryption system
void satox_hybrid_encryption_free_buffer(void* buffer);

#ifdef __cplusplus
}
#endif

#endif // SATOX_HYBRID_ENCRYPTION_H 