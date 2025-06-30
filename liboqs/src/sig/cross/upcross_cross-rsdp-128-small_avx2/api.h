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

/**
 *
 * Reference ISO-C11 Implementation of CROSS.
 *
 * @version 2.0 (February 2025)
 *
 * Authors listed in alphabetical order:
 *
 * @author: Alessandro Barenghi <alessandro.barenghi@polimi.it>
 * @author: Marco Gianvecchio <marco.gianvecchio@mail.polimi.it>
 * @author: Patrick Karl <patrick.karl@tum.de>
 * @author: Gerardo Pelosi <gerardo.pelosi@polimi.it>
 * @author: Jonas Schupp <jonas.schupp@tum.de>
 *
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **/

#ifndef PQCLEAN_CROSSRSDP128SMALL_AVX2_API_H
#define PQCLEAN_CROSSRSDP128SMALL_AVX2_API_H

#pragma once

#include <stddef.h>
#include <stdint.h>

#define PQCLEAN_CROSSRSDP128SMALL_AVX2_CRYPTO_ALGNAME "cross-rsdp-128-small"

/*  no. of bytes of the secret key */
#define PQCLEAN_CROSSRSDP128SMALL_AVX2_CRYPTO_SECRETKEYBYTES 32

/*  no. of bytes of the public key */
#define PQCLEAN_CROSSRSDP128SMALL_AVX2_CRYPTO_PUBLICKEYBYTES 77

/* no. of bytes of overhead in a signed message */
#define PQCLEAN_CROSSRSDP128SMALL_AVX2_CRYPTO_BYTES 12432

/* required bytes of input randomness */
#define PQCLEAN_CROSSRSDP128SMALL_AVX2_CRYPTO_RANDOMBYTES 16

int PQCLEAN_CROSSRSDP128SMALL_AVX2_crypto_sign_keypair(unsigned char *pk,
        unsigned char *sk
                                                      );

int PQCLEAN_CROSSRSDP128SMALL_AVX2_crypto_sign(unsigned char *sm,
        size_t *smlen,
        const unsigned char *m,
        size_t mlen,
        const unsigned char *sk
                                              );

int PQCLEAN_CROSSRSDP128SMALL_AVX2_crypto_sign_open(unsigned char *m,
        size_t *mlen,
        const unsigned char *sm,
        size_t smlen,
        const unsigned char *pk
                                                   );

int PQCLEAN_CROSSRSDP128SMALL_AVX2_crypto_sign_signature(unsigned char *sig,
        size_t *siglen,
        const unsigned char *m,
        size_t mlen,
        const unsigned char *sk
                                                        );

int PQCLEAN_CROSSRSDP128SMALL_AVX2_crypto_sign_verify(const unsigned char *sig,
        size_t siglen,
        const unsigned char *m,
        size_t mlen,
        const unsigned char *pk
                                                     );

#endif
