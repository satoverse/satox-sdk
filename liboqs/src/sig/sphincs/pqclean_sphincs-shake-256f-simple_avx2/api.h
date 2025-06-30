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

#ifndef PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_API_H
#define PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_API_H

#include <stddef.h>
#include <stdint.h>

#define PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_CRYPTO_ALGNAME "SPHINCS+-shake-256f-simple"

#define PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_CRYPTO_SECRETKEYBYTES 128
#define PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_CRYPTO_PUBLICKEYBYTES 64
#define PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_CRYPTO_BYTES          49856

#define PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_CRYPTO_SEEDBYTES      96

/*
 * Returns the length of a secret key, in bytes
 */
size_t PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_secretkeybytes(void);

/*
 * Returns the length of a public key, in bytes
 */
size_t PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_publickeybytes(void);

/*
 * Returns the length of a signature, in bytes
 */
size_t PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_bytes(void);

/*
 * Returns the length of the seed required to generate a key pair, in bytes
 */
size_t PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_seedbytes(void);

/*
 * Generates a SPHINCS+ key pair given a seed.
 * Format sk: [SK_SEED || SK_PRF || PUB_SEED || root]
 * Format pk: [root || PUB_SEED]
 */
int PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_seed_keypair(uint8_t *pk, uint8_t *sk,
        const uint8_t *seed);

/*
 * Generates a SPHINCS+ key pair.
 * Format sk: [SK_SEED || SK_PRF || PUB_SEED || root]
 * Format pk: [root || PUB_SEED]
 */
int PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_keypair(uint8_t *pk, uint8_t *sk);

/**
 * Returns an array containing a detached signature.
 */
int PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_signature(uint8_t *sig, size_t *siglen,
        const uint8_t *m, size_t mlen,
        const uint8_t *sk);

/**
 * Verifies a detached signature and message under a given public key.
 */
int PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_verify(const uint8_t *sig, size_t siglen,
        const uint8_t *m, size_t mlen,
        const uint8_t *pk);

/**
 * Returns an array containing the signature followed by the message.
 */
int PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign(uint8_t *sm, size_t *smlen,
        const uint8_t *m, size_t mlen,
        const uint8_t *sk);

/**
 * Verifies a given signature-message pair under a given public key.
 */
int PQCLEAN_SPHINCSSHAKE256FSIMPLE_AVX2_crypto_sign_open(uint8_t *m, size_t *mlen,
        const uint8_t *sm, size_t smlen,
        const uint8_t *pk);
#endif
