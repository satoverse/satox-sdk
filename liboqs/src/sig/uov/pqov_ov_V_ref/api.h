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

// SPDX-License-Identifier: CC0 OR Apache-2.0
#ifndef _API_H_
#define _API_H_


#include "params.h"

#define CRYPTO_SECRETKEYBYTES OV_SECRETKEYBYTES
#define CRYPTO_PUBLICKEYBYTES OV_PUBLICKEYBYTES
#define CRYPTO_BYTES          OV_SIGNATUREBYTES
#define CRYPTO_ALGNAME        OV_ALGNAME

//#define _SUPERCOP_

#if defined(PQM4) || defined(_UTILS_OQS_)
// for size_t
#include <stddef.h>

#ifdef  __cplusplus
extern  "C" {
#endif

#define crypto_sign_keypair PQOV_NAMESPACE(keypair)
int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

#define crypto_sign PQOV_NAMESPACE(sign)
int
crypto_sign(unsigned char *sm, size_t *smlen,
            const unsigned char *m, size_t mlen,
            const unsigned char *sk);

#define crypto_sign_signature PQOV_NAMESPACE(signature)
int
crypto_sign_signature(unsigned char  *sig, size_t *siglen,
                      const unsigned char  *m, size_t mlen,
                      const unsigned char  *sk);

#define crypto_sign_open PQOV_NAMESPACE(open)
int
crypto_sign_open(unsigned char *m, size_t *mlen,
                 const unsigned char *sm, size_t smlen,
                 const unsigned char *pk);

#define crypto_sign_verify PQOV_NAMESPACE(verify)
int
crypto_sign_verify(const unsigned char  *sig, size_t siglen,
                      const unsigned char  *m, size_t mlen,
                      const unsigned char  *pk);

#ifdef  __cplusplus
}
#endif


#elif defined(_SUPERCOP_)
#include "crypto_sign.h"

#else

#ifdef  __cplusplus
extern  "C" {
#endif

#define crypto_sign_keypair PQOV_NAMESPACE(keypair)
int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

#define crypto_sign PQOV_NAMESPACE(sign)
int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk);

#define crypto_sign_signature PQOV_NAMESPACE(signature)
int
crypto_sign_signature(unsigned char *sig, unsigned long long *siglen,
                      const unsigned char *m, unsigned long long mlen,
                      const unsigned char *sk);

#define crypto_sign_open PQOV_NAMESPACE(open)
int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk);

#define crypto_sign_verify PQOV_NAMESPACE(verify)
int
crypto_sign_verify(const unsigned char *sig, unsigned long long siglen,
                   const unsigned char *m, unsigned long long mlen,
                   const unsigned char *pk);

#ifdef  __cplusplus
}
#endif

#endif  // defined(PQM4)

#endif
