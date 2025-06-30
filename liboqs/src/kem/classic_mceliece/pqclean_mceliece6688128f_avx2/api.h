/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PQCLEAN_MCELIECE6688128F_AVX2_API_H
#define PQCLEAN_MCELIECE6688128F_AVX2_API_H

#include <stdint.h>

#define PQCLEAN_MCELIECE6688128F_AVX2_CRYPTO_ALGNAME "Classic McEliece 6688128"
#define PQCLEAN_MCELIECE6688128F_AVX2_CRYPTO_PUBLICKEYBYTES 1044992
#define PQCLEAN_MCELIECE6688128F_AVX2_CRYPTO_SECRETKEYBYTES 13932
#define PQCLEAN_MCELIECE6688128F_AVX2_CRYPTO_CIPHERTEXTBYTES 208
#define PQCLEAN_MCELIECE6688128F_AVX2_CRYPTO_BYTES 32

int PQCLEAN_MCELIECE6688128F_AVX2_crypto_kem_enc(
    uint8_t *c,
    uint8_t *key,
    const uint8_t *pk
);

int PQCLEAN_MCELIECE6688128F_AVX2_crypto_kem_dec(
    uint8_t *key,
    const uint8_t *c,
    const uint8_t *sk
);

int PQCLEAN_MCELIECE6688128F_AVX2_crypto_kem_keypair
(
    uint8_t *pk,
    uint8_t *sk
);

#endif
