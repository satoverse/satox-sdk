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

/*
 * Wrapper for implementing the PQClean API.
 */

#include <stddef.h>
#include <string.h>

#include "api.h"
#include "inner.h"

#define NONCELEN   40

#include "randombytes.h"

/*
 * Encoding formats (nnnn = log of degree, 9 for Falcon-512, 10 for Falcon-1024)
 *
 *   private key:
 *      header byte: 0101nnnn
 *      private f  (6 or 5 bits by element, depending on degree)
 *      private g  (6 or 5 bits by element, depending on degree)
 *      private F  (8 bits by element)
 *
 *   public key:
 *      header byte: 0000nnnn
 *      public h   (14 bits by element)
 *
 *   signature:
 *      header byte: 0011nnnn
 *      nonce (r)  40 bytes
 *      value (s)  compressed format
 *
 *   message + signature:
 *      signature length   (2 bytes, big-endian)
 *      nonce              40 bytes
 *      message
 *      header byte:       0010nnnn
 *      value              compressed format
 *      (signature length is 1+len(value), not counting the nonce)
 */

/* see api.h */
int
PQCLEAN_FALCON1024_AARCH64_crypto_sign_keypair(
    uint8_t *pk, uint8_t *sk) {
    union {
        uint8_t b[28 * FALCON_N];
        uint64_t dummy_u64;
        fpr dummy_fpr;
    } tmp;
    int8_t f[FALCON_N], g[FALCON_N], F[FALCON_N];
    uint16_t h[FALCON_N];
    unsigned char seed[48];
    inner_shake256_context rng;
    size_t u, v;

    /*
     * Generate key pair.
     */
    randombytes(seed, sizeof seed);
    inner_shake256_init(&rng);
    inner_shake256_inject(&rng, seed, sizeof seed);
    inner_shake256_flip(&rng);
    PQCLEAN_FALCON1024_AARCH64_keygen(&rng, f, g, F, NULL, h, FALCON_LOGN, tmp.b);
    inner_shake256_ctx_release(&rng);

    /*
     * Encode private key.
     */
    sk[0] = 0x50 + FALCON_LOGN;
    u = 1;
    v = PQCLEAN_FALCON1024_AARCH64_trim_i8_encode(
            sk + u, PQCLEAN_FALCON1024_AARCH64_CRYPTO_SECRETKEYBYTES - u,
            f, PQCLEAN_FALCON1024_AARCH64_max_fg_bits[FALCON_LOGN]);
    if (v == 0) {
        return -1;
    }
    u += v;
    v = PQCLEAN_FALCON1024_AARCH64_trim_i8_encode(
            sk + u, PQCLEAN_FALCON1024_AARCH64_CRYPTO_SECRETKEYBYTES - u,
            g, PQCLEAN_FALCON1024_AARCH64_max_fg_bits[FALCON_LOGN]);
    if (v == 0) {
        return -1;
    }
    u += v;
    v = PQCLEAN_FALCON1024_AARCH64_trim_i8_encode(
            sk + u, PQCLEAN_FALCON1024_AARCH64_CRYPTO_SECRETKEYBYTES - u,
            F, PQCLEAN_FALCON1024_AARCH64_max_FG_bits[FALCON_LOGN]);
    if (v == 0) {
        return -1;
    }
    u += v;
    if (u != PQCLEAN_FALCON1024_AARCH64_CRYPTO_SECRETKEYBYTES) {
        return -1;
    }

    /*
     * Encode public key.
     */
    pk[0] = 0x00 + FALCON_LOGN;
    v = PQCLEAN_FALCON1024_AARCH64_modq_encode(
            pk + 1, PQCLEAN_FALCON1024_AARCH64_CRYPTO_PUBLICKEYBYTES - 1,
            h, FALCON_LOGN);
    if (v != PQCLEAN_FALCON1024_AARCH64_CRYPTO_PUBLICKEYBYTES - 1) {
        return -1;
    }

    return 0;
}

/*
 * Compute the signature. nonce[] receives the nonce and must have length
 * NONCELEN bytes. sigbuf[] receives the signature value (without nonce
 * or header byte), with *sigbuflen providing the maximum value length and
 * receiving the actual value length.
 *
 * If a signature could be computed but not encoded because it would
 * exceed the output buffer size, then an error is returned.
 *
 * Return value: 0 on success, -1 on error.
 */
static int
do_sign(uint8_t *nonce, uint8_t *sigbuf, size_t *sigbuflen,
        const uint8_t *m, size_t mlen, const uint8_t *sk) {
    union {
        uint8_t b[72 * FALCON_N];
        uint64_t dummy_u64;
        fpr dummy_fpr;
    } tmp;
    int8_t f[FALCON_N], g[FALCON_N], F[FALCON_N], G[FALCON_N];
    struct {
        int16_t sig[FALCON_N];
        uint16_t hm[FALCON_N];
    } r;
    unsigned char seed[48];
    inner_shake256_context sc;
    size_t u, v;

    /*
     * Decode the private key.
     */
    if (sk[0] != 0x50 + FALCON_LOGN) {
        return -1;
    }
    u = 1;
    v = PQCLEAN_FALCON1024_AARCH64_trim_i8_decode(
            f, PQCLEAN_FALCON1024_AARCH64_max_fg_bits[FALCON_LOGN],
            sk + u, PQCLEAN_FALCON1024_AARCH64_CRYPTO_SECRETKEYBYTES - u);
    if (v == 0) {
        return -1;
    }
    u += v;
    v = PQCLEAN_FALCON1024_AARCH64_trim_i8_decode(
            g, PQCLEAN_FALCON1024_AARCH64_max_fg_bits[FALCON_LOGN],
            sk + u, PQCLEAN_FALCON1024_AARCH64_CRYPTO_SECRETKEYBYTES - u);
    if (v == 0) {
        return -1;
    }
    u += v;
    v = PQCLEAN_FALCON1024_AARCH64_trim_i8_decode(
            F, PQCLEAN_FALCON1024_AARCH64_max_FG_bits[FALCON_LOGN],
            sk + u, PQCLEAN_FALCON1024_AARCH64_CRYPTO_SECRETKEYBYTES - u);
    if (v == 0) {
        return -1;
    }
    u += v;
    if (u != PQCLEAN_FALCON1024_AARCH64_CRYPTO_SECRETKEYBYTES) {
        return -1;
    }
    if (!PQCLEAN_FALCON1024_AARCH64_complete_private(G, f, g, F, tmp.b)) {
        return -1;
    }

    /*
     * Create a random nonce (40 bytes).
     */
    randombytes(nonce, NONCELEN);

    /*
     * Hash message nonce + message into a vector.
     */
    inner_shake256_init(&sc);
    inner_shake256_inject(&sc, nonce, NONCELEN);
    inner_shake256_inject(&sc, m, mlen);
    inner_shake256_flip(&sc);
    PQCLEAN_FALCON1024_AARCH64_hash_to_point_ct(&sc, r.hm, FALCON_LOGN, tmp.b);
    inner_shake256_ctx_release(&sc);

    /*
     * Initialize a RNG.
     */
    randombytes(seed, sizeof seed);
    inner_shake256_init(&sc);
    inner_shake256_inject(&sc, seed, sizeof seed);
    inner_shake256_flip(&sc);

    /*
     * Compute and return the signature.
     */
    PQCLEAN_FALCON1024_AARCH64_sign_dyn(r.sig, &sc, f, g, F, G, r.hm, tmp.b);
    v = PQCLEAN_FALCON1024_AARCH64_comp_encode(sigbuf, *sigbuflen, r.sig);
    if (v != 0) {
        inner_shake256_ctx_release(&sc);
        *sigbuflen = v;
        return 0;
    }
    return -1;
}

/*
 * Verify a sigature. The nonce has size NONCELEN bytes. sigbuf[]
 * (of size sigbuflen) contains the signature value, not including the
 * header byte or nonce. Return value is 0 on success, -1 on error.
 */
static int
do_verify(
    const uint8_t *nonce, const uint8_t *sigbuf, size_t sigbuflen,
    const uint8_t *m, size_t mlen, const uint8_t *pk) {
    union {
        uint8_t b[2 * FALCON_N];
        uint64_t dummy_u64;
        fpr dummy_fpr;
    } tmp;
    int16_t h[FALCON_N];
    int16_t hm[FALCON_N];
    int16_t sig[FALCON_N];
    inner_shake256_context sc;
    size_t v;

    /*
     * Decode public key.
     */
    if (pk[0] != 0x00 + FALCON_LOGN) {
        return -1;
    }
    if (PQCLEAN_FALCON1024_AARCH64_modq_decode( (uint16_t *) h,
            pk + 1, PQCLEAN_FALCON1024_AARCH64_CRYPTO_PUBLICKEYBYTES - 1, FALCON_LOGN)
            != PQCLEAN_FALCON1024_AARCH64_CRYPTO_PUBLICKEYBYTES - 1) {
        return -1;
    }
    // We move the conversion to NTT domain of `h` inside verify_raw()

    /*
     * Decode signature.
     */
    if (sigbuflen == 0) {
        return -1;
    }

    v = PQCLEAN_FALCON1024_AARCH64_comp_decode(sig, sigbuf, sigbuflen);
    if (v == 0) {
        return -1;
    }
    if (v != sigbuflen) {
        if (sigbuflen == PQCLEAN_FALCONPADDED1024_AARCH64_CRYPTO_BYTES - NONCELEN - 1) {
            while (v < sigbuflen) {
                if (sigbuf[v++] != 0) {
                    return -1;
                }
            }
        } else {
            return -1;
        }
    }

    /*
     * Hash nonce + message into a vector.
     */
    inner_shake256_init(&sc);
    inner_shake256_inject(&sc, nonce, NONCELEN);
    inner_shake256_inject(&sc, m, mlen);
    inner_shake256_flip(&sc);
    PQCLEAN_FALCON1024_AARCH64_hash_to_point_ct(&sc, (uint16_t *) hm, FALCON_LOGN, tmp.b);
    inner_shake256_ctx_release(&sc);

    /*
     * Verify signature.
     */
    if (!PQCLEAN_FALCON1024_AARCH64_verify_raw(hm, sig, h, (int16_t *) tmp.b)) {
        return -1;
    }
    return 0;
}

/* see api.h */
int
PQCLEAN_FALCON1024_AARCH64_crypto_sign_signature(
    uint8_t *sig, size_t *siglen,
    const uint8_t *m, size_t mlen, const uint8_t *sk) {
    size_t vlen;

    vlen = PQCLEAN_FALCON1024_AARCH64_CRYPTO_BYTES - NONCELEN - 1;
    if (do_sign(sig + 1, sig + 1 + NONCELEN, &vlen, m, mlen, sk) < 0) {
        return -1;
    }
    sig[0] = 0x30 + FALCON_LOGN;
    *siglen = 1 + NONCELEN + vlen;
    return 0;
}

/* see api.h */
int
PQCLEAN_FALCON1024_AARCH64_crypto_sign_verify(
    const uint8_t *sig, size_t siglen,
    const uint8_t *m, size_t mlen, const uint8_t *pk) {
    if (siglen < 1 + NONCELEN) {
        return -1;
    }
    if (sig[0] != 0x30 + FALCON_LOGN) {
        return -1;
    }
    return do_verify(sig + 1,
                     sig + 1 + NONCELEN, siglen - 1 - NONCELEN, m, mlen, pk);
}

/* see api.h */
int
PQCLEAN_FALCON1024_AARCH64_crypto_sign(
    uint8_t *sm, size_t *smlen,
    const uint8_t *m, size_t mlen, const uint8_t *sk) {
    uint8_t *pm, *sigbuf;
    size_t sigbuflen;

    /*
     * Move the message to its final location; this is a memmove() so
     * it handles overlaps properly.
     */
    memmove(sm + 2 + NONCELEN, m, mlen);
    pm = sm + 2 + NONCELEN;
    sigbuf = pm + 1 + mlen;
    sigbuflen = PQCLEAN_FALCON1024_AARCH64_CRYPTO_BYTES - NONCELEN - 3;
    if (do_sign(sm + 2, sigbuf, &sigbuflen, pm, mlen, sk) < 0) {
        return -1;
    }
    pm[mlen] = 0x20 + FALCON_LOGN;
    sigbuflen ++;
    sm[0] = (uint8_t)(sigbuflen >> 8);
    sm[1] = (uint8_t)sigbuflen;
    *smlen = mlen + 2 + NONCELEN + sigbuflen;
    return 0;
}

/* see api.h */
int
PQCLEAN_FALCON1024_AARCH64_crypto_sign_open(
    uint8_t *m, size_t *mlen,
    const uint8_t *sm, size_t smlen, const uint8_t *pk) {
    const uint8_t *sigbuf;
    size_t pmlen, sigbuflen;

    if (smlen < 3 + NONCELEN) {
        return -1;
    }
    sigbuflen = ((size_t)sm[0] << 8) | (size_t)sm[1];
    if (sigbuflen < 2 || sigbuflen > (smlen - NONCELEN - 2)) {
        return -1;
    }
    sigbuflen --;
    pmlen = smlen - NONCELEN - 3 - sigbuflen;
    if (sm[2 + NONCELEN + pmlen] != 0x20 + FALCON_LOGN) {
        return -1;
    }
    sigbuf = sm + 2 + NONCELEN + pmlen + 1;

    /*
     * The 2-byte length header and the one-byte signature header
     * have been verified. Nonce is at sm+2, followed by the message
     * itself. Message length is in pmlen. sigbuf/sigbuflen point to
     * the signature value (excluding the header byte).
     */
    if (do_verify(sm + 2, sigbuf, sigbuflen,
                  sm + 2 + NONCELEN, pmlen, pk) < 0) {
        return -1;
    }

    /*
     * Signature is correct, we just have to copy/move the message
     * to its final destination. The memmove() properly handles
     * overlaps.
     */
    memmove(m, sm + 2 + NONCELEN, pmlen);
    *mlen = pmlen;
    return 0;
}
