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

// SPDX-License-Identifier: Apache-2.0

#include <api.h>
#include <mayo.h>

#ifdef ENABLE_PARAMS_DYNAMIC
#define MAYO_PARAMS &MAYO_5
#else
#define MAYO_PARAMS 0
#endif

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk) {
    return mayo_keypair(MAYO_PARAMS, pk, sk);
}

int
crypto_sign(unsigned char *sm, size_t *smlen,
            const unsigned char *m, size_t mlen,
            const unsigned char *sk) {
    return mayo_sign(MAYO_PARAMS, sm, smlen, m, mlen, sk);
}

int
crypto_sign_signature(unsigned char *sig,
              size_t *siglen, const unsigned char *m,
              size_t mlen, const unsigned char *sk) {
    return mayo_sign_signature(MAYO_PARAMS, sig, siglen, m, mlen, sk);
}

int
crypto_sign_open(unsigned char *m, size_t *mlen,
                 const unsigned char *sm, size_t smlen,
                 const unsigned char *pk) {
    return mayo_open(MAYO_PARAMS, m, mlen, sm, smlen, pk);
}

int
crypto_sign_verify(const unsigned char *sig, size_t siglen,
                   const unsigned char *m, size_t mlen,
                   const unsigned char *pk) {
    if (siglen != CRYPTO_BYTES)
        return -1;
    return mayo_verify(MAYO_PARAMS, m, mlen, sig, pk);
}

