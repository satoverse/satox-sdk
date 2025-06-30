#ifndef SATOX_POST_QUANTUM_ALGORITHMS_H
#define SATOX_POST_QUANTUM_ALGORITHMS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct satox_algorithm_info {
    const char* name;
    int security_level;
    int key_size;
    int signature_size;
    int is_recommended;
    const char* description;
};

int satox_post_quantum_algorithms_initialize(void);
int satox_post_quantum_algorithms_shutdown(void);

int satox_post_quantum_algorithms_generate_key_pair(
    const char* algorithm_name,
    uint8_t** public_key,
    size_t* public_key_len,
    uint8_t** private_key,
    size_t* private_key_len
);

int satox_post_quantum_algorithms_sign(
    const uint8_t* private_key,
    size_t private_key_len,
    const uint8_t* data,
    size_t data_len,
    uint8_t** signature,
    size_t* signature_len
);

int satox_post_quantum_algorithms_verify(
    const uint8_t* public_key,
    size_t public_key_len,
    const uint8_t* data,
    size_t data_len,
    const uint8_t* signature,
    size_t signature_len,
    int* is_valid
);

int satox_post_quantum_algorithms_encrypt(
    const uint8_t* public_key,
    size_t public_key_len,
    const uint8_t* data,
    size_t data_len,
    uint8_t** encrypted_data,
    size_t* encrypted_data_len
);

int satox_post_quantum_algorithms_decrypt(
    const uint8_t* private_key,
    size_t private_key_len,
    const uint8_t* encrypted_data,
    size_t encrypted_data_len,
    uint8_t** decrypted_data,
    size_t* decrypted_data_len
);

int satox_post_quantum_algorithms_get_algorithm_info(
    const char* algorithm_name,
    struct satox_algorithm_info* info
);

int satox_post_quantum_algorithms_get_available_algorithms(
    char*** algorithms,
    size_t* algorithms_len
);

int satox_post_quantum_algorithms_get_recommended_algorithms(
    char*** algorithms,
    size_t* algorithms_len
);

int satox_post_quantum_algorithms_is_algorithm_available(
    const char* algorithm_name,
    int* is_available
);

int satox_post_quantum_algorithms_is_algorithm_recommended(
    const char* algorithm_name,
    int* is_recommended
);

int satox_post_quantum_algorithms_get_algorithm_security_level(
    const char* algorithm_name,
    int* security_level
);

int satox_post_quantum_algorithms_get_algorithm_key_size(
    const char* algorithm_name,
    int* key_size
);

int satox_post_quantum_algorithms_get_algorithm_signature_size(
    const char* algorithm_name,
    int* signature_size
);

int satox_post_quantum_algorithms_get_default_algorithm(
    char** algorithm_name
);

int satox_post_quantum_algorithms_set_default_algorithm(
    const char* algorithm_name
);

void satox_post_quantum_algorithms_free_buffer(void* buffer);

#ifdef __cplusplus
}
#endif

#endif // SATOX_POST_QUANTUM_ALGORITHMS_H 