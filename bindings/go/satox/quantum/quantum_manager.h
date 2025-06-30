#ifndef SATOX_QUANTUM_MANAGER_H
#define SATOX_QUANTUM_MANAGER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the quantum manager
int satox_quantum_manager_initialize();

// Shutdown the quantum manager
int satox_quantum_manager_shutdown();

// Generate a new key pair
int satox_quantum_manager_generate_key_pair(
    uint8_t** public_key,
    size_t* public_key_length,
    uint8_t** private_key,
    size_t* private_key_length
);

// Encrypt data using a public key
int satox_quantum_manager_encrypt(
    const uint8_t* public_key,
    size_t public_key_length,
    const uint8_t* data,
    size_t data_length,
    uint8_t** encrypted_data,
    size_t* encrypted_data_length
);

// Decrypt data using a private key
int satox_quantum_manager_decrypt(
    const uint8_t* private_key,
    size_t private_key_length,
    const uint8_t* encrypted_data,
    size_t encrypted_data_length,
    uint8_t** decrypted_data,
    size_t* decrypted_data_length
);

// Sign data using a private key
int satox_quantum_manager_sign(
    const uint8_t* private_key,
    size_t private_key_length,
    const uint8_t* data,
    size_t data_length,
    uint8_t** signature,
    size_t* signature_length
);

// Verify a signature using a public key
int satox_quantum_manager_verify(
    const uint8_t* public_key,
    size_t public_key_length,
    const uint8_t* data,
    size_t data_length,
    const uint8_t* signature,
    size_t signature_length,
    int* is_valid
);

// Generate a random number within a range
int satox_quantum_manager_generate_random_number(
    int min,
    int max,
    int* random_number
);

// Generate random bytes
int satox_quantum_manager_generate_random_bytes(
    size_t length,
    uint8_t** random_bytes,
    size_t* random_bytes_length
);

// Get the version of the quantum manager
int satox_quantum_manager_get_version(char** version);

// Free a buffer allocated by the quantum manager
void satox_quantum_manager_free_buffer(void* buffer);

#ifdef __cplusplus
}
#endif

#endif // SATOX_QUANTUM_MANAGER_H 