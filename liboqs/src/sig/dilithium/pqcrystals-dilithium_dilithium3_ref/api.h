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

#ifndef API_H
#define API_H

#include <stddef.h>
#include <stdint.h>

#define pqcrystals_dilithium2_PUBLICKEYBYTES 1312
#define pqcrystals_dilithium2_SECRETKEYBYTES 2528
#define pqcrystals_dilithium2_BYTES 2420

#define pqcrystals_dilithium2_ref_PUBLICKEYBYTES pqcrystals_dilithium2_PUBLICKEYBYTES
#define pqcrystals_dilithium2_ref_SECRETKEYBYTES pqcrystals_dilithium2_SECRETKEYBYTES
#define pqcrystals_dilithium2_ref_BYTES pqcrystals_dilithium2_BYTES

int pqcrystals_dilithium2_ref_keypair(uint8_t *pk, uint8_t *sk);

int pqcrystals_dilithium2_ref_signature(uint8_t *sig, size_t *siglen,
                                        const uint8_t *m, size_t mlen,
                                        const uint8_t *sk);

int pqcrystals_dilithium2_ref(uint8_t *sm, size_t *smlen,
                              const uint8_t *m, size_t mlen,
                              const uint8_t *sk);

int pqcrystals_dilithium2_ref_verify(const uint8_t *sig, size_t siglen,
                                     const uint8_t *m, size_t mlen,
                                     const uint8_t *pk);

int pqcrystals_dilithium2_ref_open(uint8_t *m, size_t *mlen,
                                   const uint8_t *sm, size_t smlen,
                                   const uint8_t *pk);

#define pqcrystals_dilithium2aes_ref_PUBLICKEYBYTES pqcrystals_dilithium2_ref_PUBLICKEYBYTES
#define pqcrystals_dilithium2aes_ref_SECRETKEYBYTES pqcrystals_dilithium2_ref_SECRETKEYBYTES
#define pqcrystals_dilithium2aes_ref_BYTES pqcrystals_dilithium2_ref_BYTES

int pqcrystals_dilithium2aes_ref_keypair(uint8_t *pk, uint8_t *sk);

int pqcrystals_dilithium2aes_ref_signature(uint8_t *sig, size_t *siglen,
                                           const uint8_t *m, size_t mlen,
                                           const uint8_t *sk);

int pqcrystals_dilithium2aes_ref(uint8_t *sm, size_t *smlen,
                                 const uint8_t *m, size_t mlen,
                                 const uint8_t *sk);

int pqcrystals_dilithium2aes_ref_verify(const uint8_t *sig, size_t siglen,
                                        const uint8_t *m, size_t mlen,
                                        const uint8_t *pk);

int pqcrystals_dilithium2aes_ref_open(uint8_t *m, size_t *mlen,
                                      const uint8_t *sm, size_t smlen,
                                      const uint8_t *pk);

#define pqcrystals_dilithium3_PUBLICKEYBYTES 1952
#define pqcrystals_dilithium3_SECRETKEYBYTES 4000
#define pqcrystals_dilithium3_BYTES 3293

#define pqcrystals_dilithium3_ref_PUBLICKEYBYTES pqcrystals_dilithium3_PUBLICKEYBYTES
#define pqcrystals_dilithium3_ref_SECRETKEYBYTES pqcrystals_dilithium3_SECRETKEYBYTES
#define pqcrystals_dilithium3_ref_BYTES pqcrystals_dilithium3_BYTES

int pqcrystals_dilithium3_ref_keypair(uint8_t *pk, uint8_t *sk);

int pqcrystals_dilithium3_ref_signature(uint8_t *sig, size_t *siglen,
                                        const uint8_t *m, size_t mlen,
                                        const uint8_t *sk);

int pqcrystals_dilithium3_ref(uint8_t *sm, size_t *smlen,
                              const uint8_t *m, size_t mlen,
                              const uint8_t *sk);

int pqcrystals_dilithium3_ref_verify(const uint8_t *sig, size_t siglen,
                                     const uint8_t *m, size_t mlen,
                                     const uint8_t *pk);

int pqcrystals_dilithium3_ref_open(uint8_t *m, size_t *mlen,
                                   const uint8_t *sm, size_t smlen,
                                   const uint8_t *pk);

#define pqcrystals_dilithium3aes_ref_PUBLICKEYBYTES pqcrystals_dilithium3_ref_PUBLICKEYBYTES
#define pqcrystals_dilithium3aes_ref_SECRETKEYBYTES pqcrystals_dilithium3_ref_SECRETKEYBYTES
#define pqcrystals_dilithium3aes_ref_BYTES pqcrystals_dilithium3_ref_BYTES

int pqcrystals_dilithium3aes_ref_keypair(uint8_t *pk, uint8_t *sk);

int pqcrystals_dilithium3aes_ref_signature(uint8_t *sig, size_t *siglen,
                                           const uint8_t *m, size_t mlen,
                                           const uint8_t *sk);

int pqcrystals_dilithium3aes_ref(uint8_t *sm, size_t *smlen,
                                 const uint8_t *m, size_t mlen,
                                 const uint8_t *sk);

int pqcrystals_dilithium3aes_ref_verify(const uint8_t *sig, size_t siglen,
                                        const uint8_t *m, size_t mlen,
                                        const uint8_t *pk);

int pqcrystals_dilithium3aes_ref_open(uint8_t *m, size_t *mlen,
                                      const uint8_t *sm, size_t smlen,
                                      const uint8_t *pk);

#define pqcrystals_dilithium5_PUBLICKEYBYTES 2592
#define pqcrystals_dilithium5_SECRETKEYBYTES 4864
#define pqcrystals_dilithium5_BYTES 4595

#define pqcrystals_dilithium5_ref_PUBLICKEYBYTES pqcrystals_dilithium5_PUBLICKEYBYTES
#define pqcrystals_dilithium5_ref_SECRETKEYBYTES pqcrystals_dilithium5_SECRETKEYBYTES
#define pqcrystals_dilithium5_ref_BYTES pqcrystals_dilithium5_BYTES

int pqcrystals_dilithium5_ref_keypair(uint8_t *pk, uint8_t *sk);

int pqcrystals_dilithium5_ref_signature(uint8_t *sig, size_t *siglen,
                                        const uint8_t *m, size_t mlen,
                                        const uint8_t *sk);

int pqcrystals_dilithium5_ref(uint8_t *sm, size_t *smlen,
                              const uint8_t *m, size_t mlen,
                              const uint8_t *sk);

int pqcrystals_dilithium5_ref_verify(const uint8_t *sig, size_t siglen,
                                     const uint8_t *m, size_t mlen,
                                     const uint8_t *pk);

int pqcrystals_dilithium5_ref_open(uint8_t *m, size_t *mlen,
                                   const uint8_t *sm, size_t smlen,
                                   const uint8_t *pk);

#define pqcrystals_dilithium5aes_ref_PUBLICKEYBYTES pqcrystals_dilithium5_ref_PUBLICKEYBYTES
#define pqcrystals_dilithium5aes_ref_SECRETKEYBYTES pqcrystals_dilithium5_ref_SECRETKEYBYTES
#define pqcrystals_dilithium5aes_ref_BYTES pqcrystals_dilithium5_ref_BYTES

int pqcrystals_dilithium5aes_ref_keypair(uint8_t *pk, uint8_t *sk);

int pqcrystals_dilithium5aes_ref_signature(uint8_t *sig, size_t *siglen,
                                           const uint8_t *m, size_t mlen,
                                           const uint8_t *sk);

int pqcrystals_dilithium5aes_ref(uint8_t *sm, size_t *smlen,
                                 const uint8_t *m, size_t mlen,
                                 const uint8_t *sk);

int pqcrystals_dilithium5aes_ref_verify(const uint8_t *sig, size_t siglen,
                                        const uint8_t *m, size_t mlen,
                                        const uint8_t *pk);

int pqcrystals_dilithium5aes_ref_open(uint8_t *m, size_t *mlen,
                                      const uint8_t *sm, size_t smlen,
                                      const uint8_t *pk);


#endif
