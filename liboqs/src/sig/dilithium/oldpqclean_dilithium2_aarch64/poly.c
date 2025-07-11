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
 * This file was originally licensed
 * under Apache 2.0 (https://www.apache.org/licenses/LICENSE-2.0.html) or
 * public domain at https://github.com/pq-crystals/dilithium/tree/master/ref
 *
 * We choose
 * CC0 1.0 Universal or the following MIT License
 *
 * MIT License
 *
 * Copyright (c) 2023: Hanno Becker, Vincent Hwang, Matthias J. Kannwischer, Bo-Yin Yang, and Shang-Yi Yang
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "params.h"
#include "poly.h"
#include "reduce.h"
#include "rounding.h"
#include "symmetric.h"
#include <stdint.h>

#include "fips202x2.h"

#include "NTT_params.h"
#include "ntt.h"

static const int32_t montgomery_const[4] = {
    DILITHIUM_Q, DILITHIUM_QINV
};

#define DBENCH_START()
#define DBENCH_STOP(t)

/*************************************************
* Name:        poly_reduce
*
* Description: Inplace reduction of all coefficients of polynomial to
*              representative in [-6283009,6283007].
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
extern void PQCLEAN_DILITHIUM2_AARCH64__asm_poly_reduce(int32_t *, const int32_t *);
void poly_reduce(poly *a) {
    DBENCH_START();

    PQCLEAN_DILITHIUM2_AARCH64__asm_poly_reduce(a->coeffs, montgomery_const);

    DBENCH_STOP(*tred);
}

/*************************************************
* Name:        poly_caddq
*
* Description: For all coefficients of in/out polynomial add Q if
*              coefficient is negative.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
extern void PQCLEAN_DILITHIUM2_AARCH64__asm_poly_caddq(int32_t *, const int32_t *);
void poly_caddq(poly *a) {
    DBENCH_START();

    PQCLEAN_DILITHIUM2_AARCH64__asm_poly_caddq(a->coeffs, montgomery_const);

    DBENCH_STOP(*tred);
}

/*************************************************
* Name:        poly_freeze
*
* Description: Inplace reduction of all coefficients of polynomial to
*              standard representatives.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
extern void PQCLEAN_DILITHIUM2_AARCH64__asm_poly_freeze(int32_t *, const int32_t *);
void poly_freeze(poly *a) {
    DBENCH_START();

    PQCLEAN_DILITHIUM2_AARCH64__asm_poly_freeze(a->coeffs, montgomery_const);

    DBENCH_STOP(*tred);
}

/*************************************************
* Name:        poly_add
*
* Description: Add polynomials. No modular reduction is performed.
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const poly *a: pointer to first summand
*              - const poly *b: pointer to second summand
**************************************************/
void poly_add(poly *c, const poly *a, const poly *b)  {
    unsigned int i;
    DBENCH_START();

    for (i = 0; i < N; ++i) {
        c->coeffs[i] = a->coeffs[i] + b->coeffs[i];
    }

    DBENCH_STOP(*tadd);
}

/*************************************************
* Name:        poly_sub
*
* Description: Subtract polynomials. No modular reduction is
*              performed.
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const poly *a: pointer to first input polynomial
*              - const poly *b: pointer to second input polynomial to be
*                               subtraced from first input polynomial
**************************************************/
void poly_sub(poly *c, const poly *a, const poly *b) {
    unsigned int i;
    DBENCH_START();

    for (i = 0; i < N; ++i) {
        c->coeffs[i] = a->coeffs[i] - b->coeffs[i];
    }

    DBENCH_STOP(*tadd);
}

/*************************************************
* Name:        poly_shiftl
*
* Description: Multiply polynomial by 2^D without modular reduction. Assumes
*              input coefficients to be less than 2^{31-D} in absolute value.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_shiftl(poly *a) {
    unsigned int i;
    DBENCH_START();

    for (i = 0; i < N; ++i) {
        a->coeffs[i] <<= D;
    }

    DBENCH_STOP(*tmul);
}

/*************************************************
* Name:        poly_ntt
*
* Description: Inplace forward NTT. Coefficients can grow by
*              8*Q in absolute value.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_ntt(poly *a) {
    DBENCH_START();

    ntt(a->coeffs);

    DBENCH_STOP(*tmul);
}

/*************************************************
* Name:        poly_invntt_tomont
*
* Description: Inplace inverse NTT and multiplication by 2^{32}.
*              Input coefficients need to be less than Q in absolute
*              value and output coefficients are again bounded by Q.
*
* Arguments:   - poly *a: pointer to input/output polynomial
**************************************************/
void poly_invntt_tomont(poly *a) {
    DBENCH_START();

    invntt_tomont(a->coeffs);

    DBENCH_STOP(*tmul);
}

/*************************************************
* Name:        poly_pointwise_montgomery
*
* Description: Pointwise multiplication of polynomials in NTT domain
*              representation and multiplication of resulting polynomial
*              by 2^{-32}.
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const poly *a: pointer to first input polynomial
*              - const poly *b: pointer to second input polynomial
**************************************************/
extern void PQCLEAN_DILITHIUM2_AARCH64__asm_poly_pointwise_montgomery(int32_t *des, const int32_t *src1, const int32_t *src2, const int32_t *table);
void poly_pointwise_montgomery(poly *c, const poly *a, const poly *b) {
    DBENCH_START();

    PQCLEAN_DILITHIUM2_AARCH64__asm_poly_pointwise_montgomery(c->coeffs, a->coeffs, b->coeffs, montgomery_const);

    DBENCH_STOP(*tmul);
}

/*************************************************
* Name:        poly_power2round
*
* Description: For all coefficients c of the input polynomial,
*              compute c0, c1 such that c mod Q = c1*2^D + c0
*              with -2^{D-1} < c0 <= 2^{D-1}. Assumes coefficients to be
*              standard representatives.
*
* Arguments:   - poly *a1: pointer to output polynomial with coefficients c1
*              - poly *a0: pointer to output polynomial with coefficients c0
*              - const poly *a: pointer to input polynomial
**************************************************/
extern void PQCLEAN_DILITHIUM2_AARCH64__asm_poly_power2round(int32_t *, int32_t *, const int32_t *);
void poly_power2round(poly *a1, poly *a0, const poly *a) {
    DBENCH_START();

    PQCLEAN_DILITHIUM2_AARCH64__asm_poly_power2round(a1->coeffs, a0->coeffs, a->coeffs);

    DBENCH_STOP(*tround);
}

/*************************************************
* Name:        poly_decompose
*
* Description: For all coefficients c of the input polynomial,
*              compute high and low bits c0, c1 such c mod Q = c1*ALPHA + c0
*              with -ALPHA/2 < c0 <= ALPHA/2 except c1 = (Q-1)/ALPHA where we
*              set c1 = 0 and -ALPHA/2 <= c0 = c mod Q - Q < 0.
*              Assumes coefficients to be standard representatives.
*
* Arguments:   - poly *a1: pointer to output polynomial with coefficients c1
*              - poly *a0: pointer to output polynomial with coefficients c0
*              - const poly *a: pointer to input polynomial
**************************************************/
void poly_decompose(poly *a1, poly *a0, const poly *a) {
    unsigned int i;
    DBENCH_START();

    for (i = 0; i < N; ++i) {
        a1->coeffs[i] = decompose(&a0->coeffs[i], a->coeffs[i]);
    }

    DBENCH_STOP(*tround);
}

/*************************************************
* Name:        poly_make_hint
*
* Description: Compute hint polynomial. The coefficients of which indicate
*              whether the low bits of the corresponding coefficient of
*              the input polynomial overflow into the high bits.
*
* Arguments:   - poly *h: pointer to output hint polynomial
*              - const poly *a0: pointer to low part of input polynomial
*              - const poly *a1: pointer to high part of input polynomial
*
* Returns number of 1 bits.
**************************************************/
unsigned int poly_make_hint(poly *h, const poly *a0, const poly *a1) {
    unsigned int i, s = 0;
    DBENCH_START();

    for (i = 0; i < N; ++i) {
        h->coeffs[i] = make_hint(a0->coeffs[i], a1->coeffs[i]);
        s += h->coeffs[i];
    }

    DBENCH_STOP(*tround);
    return s;
}

/*************************************************
* Name:        poly_use_hint
*
* Description: Use hint polynomial to correct the high bits of a polynomial.
*
* Arguments:   - poly *b: pointer to output polynomial with corrected high bits
*              - const poly *a: pointer to input polynomial
*              - const poly *h: pointer to input hint polynomial
**************************************************/
void poly_use_hint(poly *b, const poly *a, const poly *h) {
    unsigned int i;
    DBENCH_START();

    for (i = 0; i < N; ++i) {
        b->coeffs[i] = use_hint(a->coeffs[i], h->coeffs[i]);
    }

    DBENCH_STOP(*tround);
}

/*************************************************
* Name:        poly_chknorm
*
* Description: Check infinity norm of polynomial against given bound.
*              Assumes input coefficients were reduced by reduce32().
*
* Arguments:   - const poly *a: pointer to polynomial
*              - int32_t B: norm bound
*
* Returns 0 if norm is strictly smaller than B <= (Q-1)/8 and 1 otherwise.
**************************************************/
int poly_chknorm(const poly *a, int32_t B) {
    unsigned int i;
    int32_t t;
    DBENCH_START();

    if (B > (DILITHIUM_Q - 1) / 8) {
        return 1;
    }

    /* It is ok to leak which coefficient violates the bound since
       the probability for each coefficient is independent of secret
       data but we must not leak the sign of the centralized representative. */
    for (i = 0; i < N; ++i) {
        /* Absolute value */
        t = a->coeffs[i] >> 31;
        t = a->coeffs[i] - (t & 2 * a->coeffs[i]);

        if (t >= B) {
            DBENCH_STOP(*tsample);
            return 1;
        }
    }

    DBENCH_STOP(*tsample);
    return 0;
}

/*************************************************
* Name:        rej_uniform
*
* Description: Sample uniformly random coefficients in [0, Q-1] by
*              performing rejection sampling on array of random bytes.
*
* Arguments:   - int32_t *a: pointer to output array (allocated)
*              - unsigned int len: number of coefficients to be sampled
*              - const uint8_t *buf: array of random bytes
*              - unsigned int buflen: length of array of random bytes
*
* Returns number of sampled coefficients. Can be smaller than len if not enough
* random bytes were given.
**************************************************/
static unsigned int rej_uniform(int32_t *a,
                                unsigned int len,
                                const uint8_t *buf,
                                unsigned int buflen) {
    unsigned int ctr, pos;
    uint32_t t;
    DBENCH_START();

    ctr = pos = 0;
    while (ctr < len && pos + 3 <= buflen) {
        t  = buf[pos++];
        t |= (uint32_t)buf[pos++] << 8;
        t |= (uint32_t)buf[pos++] << 16;
        t &= 0x7FFFFF;

        if (t < DILITHIUM_Q) {
            a[ctr++] = t;
        }
    }

    DBENCH_STOP(*tsample);
    return ctr;
}

/*************************************************
* Name:        poly_uniform
*
* Description: Sample polynomial with uniformly random coefficients
*              in [0,Q-1] by performing rejection sampling on the
*              output stream of SHAKE256(seed|nonce) or AES256CTR(seed,nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed of length SEEDBYTES
*              - uint16_t nonce: 2-byte nonce
**************************************************/

#define POLY_UNIFORM_NBLOCKS ((768 + STREAM128_BLOCKBYTES - 1)/STREAM128_BLOCKBYTES)
void poly_uniform(poly *a,
                  const uint8_t seed[SEEDBYTES],
                  uint16_t nonce) {
    unsigned int i, ctr, off;
    unsigned int buflen = POLY_UNIFORM_NBLOCKS * STREAM128_BLOCKBYTES;
    uint8_t buf[POLY_UNIFORM_NBLOCKS * STREAM128_BLOCKBYTES + 2];
    stream128_state state;

    stream128_init(&state, seed, nonce);
    stream128_squeezeblocks(buf, POLY_UNIFORM_NBLOCKS, &state);

    ctr = rej_uniform(a->coeffs, N, buf, buflen);

    while (ctr < N) {
        off = buflen % 3;
        for (i = 0; i < off; ++i) {
            buf[i] = buf[buflen - off + i];
        }

        stream128_squeezeblocks(buf + off, 1, &state);
        buflen = STREAM128_BLOCKBYTES + off;
        ctr += rej_uniform(a->coeffs + ctr, N - ctr, buf, buflen);
    }
    stream128_release(&state);
}

void poly_uniformx2(poly *a0, poly *a1,
                    const uint8_t seed[SEEDBYTES],
                    uint16_t nonce0, uint16_t nonce1) {
    unsigned int ctr0, ctr1;
    unsigned int buflen = POLY_UNIFORM_NBLOCKS * STREAM128_BLOCKBYTES;
    uint8_t buf0[POLY_UNIFORM_NBLOCKS * STREAM128_BLOCKBYTES + 2];
    uint8_t buf1[POLY_UNIFORM_NBLOCKS * STREAM128_BLOCKBYTES + 2];

    keccakx2_state statex2;
    dilithium_shake128x2_stream_init(&statex2, seed, nonce0, nonce1);
    shake128x2_squeezeblocks(buf0, buf1, POLY_UNIFORM_NBLOCKS, &statex2);

    ctr0 = rej_uniform(a0->coeffs, N, buf0, buflen);
    ctr1 = rej_uniform(a1->coeffs, N, buf1, buflen);

    while (ctr0 < N || ctr1 < N) {
        shake128x2_squeezeblocks(buf0, buf1, 1, &statex2);
        ctr0 += rej_uniform(a0->coeffs + ctr0, N - ctr0, buf0, buflen);
        ctr1 += rej_uniform(a1->coeffs + ctr1, N - ctr1, buf1, buflen);
    }


}

/*************************************************
* Name:        rej_eta
*
* Description: Sample uniformly random coefficients in [-ETA, ETA] by
*              performing rejection sampling on array of random bytes.
*
* Arguments:   - int32_t *a: pointer to output array (allocated)
*              - unsigned int len: number of coefficients to be sampled
*              - const uint8_t *buf: array of random bytes
*              - unsigned int buflen: length of array of random bytes
*
* Returns number of sampled coefficients. Can be smaller than len if not enough
* random bytes were given.
**************************************************/
static unsigned int rej_eta(int32_t *a,
                            unsigned int len,
                            const uint8_t *buf,
                            unsigned int buflen) {
    unsigned int ctr, pos;
    uint32_t t0, t1;
    DBENCH_START();

    ctr = pos = 0;
    while (ctr < len && pos < buflen) {
        t0 = buf[pos] & 0x0F;
        t1 = buf[pos++] >> 4;


        if (t0 < 15) {
            t0 = t0 - (205 * t0 >> 10) * 5;
            a[ctr++] = 2 - t0;
        }
        if (t1 < 15 && ctr < len) {
            t1 = t1 - (205 * t1 >> 10) * 5;
            a[ctr++] = 2 - t1;
        }



    }

    DBENCH_STOP(*tsample);
    return ctr;
}

/*************************************************
* Name:        poly_uniform_eta
*
* Description: Sample polynomial with uniformly random coefficients
*              in [-ETA,ETA] by performing rejection sampling on the
*              output stream from SHAKE256(seed|nonce) or AES256CTR(seed,nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed of length CRHBYTES
*              - uint16_t nonce: 2-byte nonce
**************************************************/
#define POLY_UNIFORM_ETA_NBLOCKS ((136 + STREAM256_BLOCKBYTES - 1)/STREAM256_BLOCKBYTES)
void poly_uniform_eta(poly *a,
                      const uint8_t seed[CRHBYTES],
                      uint16_t nonce) {
    unsigned int ctr;
    unsigned int buflen = POLY_UNIFORM_ETA_NBLOCKS * STREAM256_BLOCKBYTES;
    uint8_t buf[POLY_UNIFORM_ETA_NBLOCKS * STREAM256_BLOCKBYTES];
    stream256_state state;

    stream256_init(&state, seed, nonce);
    stream256_squeezeblocks(buf, POLY_UNIFORM_ETA_NBLOCKS, &state);

    ctr = rej_eta(a->coeffs, N, buf, buflen);

    while (ctr < N) {
        stream256_squeezeblocks(buf, 1, &state);
        ctr += rej_eta(a->coeffs + ctr, N - ctr, buf, STREAM256_BLOCKBYTES);
    }
    stream256_release(&state);
}

void poly_uniform_etax2(poly *a0, poly *a1,
                        const uint8_t seed[CRHBYTES],
                        uint16_t nonce0, uint16_t nonce1) {
    unsigned int ctr0, ctr1;
    unsigned int buflen = POLY_UNIFORM_ETA_NBLOCKS * STREAM256_BLOCKBYTES;

    uint8_t buf0[POLY_UNIFORM_ETA_NBLOCKS * STREAM256_BLOCKBYTES];
    uint8_t buf1[POLY_UNIFORM_ETA_NBLOCKS * STREAM256_BLOCKBYTES];

    keccakx2_state statex2;

    dilithium_shake256x2_stream_init(&statex2, seed, nonce0, nonce1);
    shake256x2_squeezeblocks(buf0, buf1, POLY_UNIFORM_ETA_NBLOCKS, &statex2);

    ctr0 = rej_eta(a0->coeffs, N, buf0, buflen);
    ctr1 = rej_eta(a1->coeffs, N, buf1, buflen);

    while (ctr0 < N || ctr1 < N) {
        shake256x2_squeezeblocks(buf0, buf1, 1, &statex2);
        ctr0 += rej_eta(a0->coeffs + ctr0, N - ctr0, buf0, STREAM256_BLOCKBYTES);
        ctr1 += rej_eta(a1->coeffs + ctr1, N - ctr1, buf1, STREAM256_BLOCKBYTES);
    }
}

/*************************************************
* Name:        poly_uniform_gamma1m1
*
* Description: Sample polynomial with uniformly random coefficients
*              in [-(GAMMA1 - 1), GAMMA1] by unpacking output stream
*              of SHAKE256(seed|nonce) or AES256CTR(seed,nonce).
*
* Arguments:   - poly *a: pointer to output polynomial
*              - const uint8_t seed[]: byte array with seed of length CRHBYTES
*              - uint16_t nonce: 16-bit nonce
**************************************************/
#define POLY_UNIFORM_GAMMA1_NBLOCKS ((POLYZ_PACKEDBYTES + STREAM256_BLOCKBYTES - 1)/STREAM256_BLOCKBYTES)
void poly_uniform_gamma1(poly *a,
                         const uint8_t seed[CRHBYTES],
                         uint16_t nonce) {
    uint8_t buf[POLY_UNIFORM_GAMMA1_NBLOCKS * STREAM256_BLOCKBYTES];
    stream256_state state;

    stream256_init(&state, seed, nonce);
    stream256_squeezeblocks(buf, POLY_UNIFORM_GAMMA1_NBLOCKS, &state);
    stream256_release(&state);
    polyz_unpack(a, buf);
}

void poly_uniform_gamma1x2(poly *a0, poly *a1,
                           const uint8_t seed[CRHBYTES],
                           uint16_t nonce0, uint16_t nonce1) {

    uint8_t buf0[POLY_UNIFORM_GAMMA1_NBLOCKS * STREAM256_BLOCKBYTES];
    uint8_t buf1[POLY_UNIFORM_GAMMA1_NBLOCKS * STREAM256_BLOCKBYTES];

    keccakx2_state statex2;

    dilithium_shake256x2_stream_init(&statex2, seed, nonce0, nonce1);
    shake256x2_squeezeblocks(buf0, buf1, POLY_UNIFORM_GAMMA1_NBLOCKS, &statex2);

    polyz_unpack(a0, buf0);
    polyz_unpack(a1, buf1);

}

/*************************************************
* Name:        challenge
*
* Description: Implementation of H. Samples polynomial with TAU nonzero
*              coefficients in {-1,1} using the output stream of
*              SHAKE256(seed).
*
* Arguments:   - poly *c: pointer to output polynomial
*              - const uint8_t mu[]: byte array containing seed of length SEEDBYTES
**************************************************/
void poly_challenge(poly *c, const uint8_t seed[SEEDBYTES]) {
    unsigned int i, b, pos;
    uint64_t signs;
    uint8_t buf[SHAKE256_RATE];
    shake256incctx state;

    shake256_inc_init(&state);
    shake256_inc_absorb(&state, seed, SEEDBYTES);
    shake256_inc_finalize(&state);
    shake256_inc_squeeze(buf, sizeof buf, &state);

    signs = 0;
    for (i = 0; i < 8; ++i) {
        signs |= (uint64_t)buf[i] << 8 * i;
    }
    pos = 8;

    for (i = 0; i < N; ++i) {
        c->coeffs[i] = 0;
    }
    for (i = N - TAU; i < N; ++i) {
        do {
            if (pos >= SHAKE256_RATE) {
                shake256_inc_squeeze(buf, sizeof buf, &state);
                pos = 0;
            }

            b = buf[pos++];
        } while (b > i);

        c->coeffs[i] = c->coeffs[b];
        c->coeffs[b] = 1 - 2 * (signs & 1);
        signs >>= 1;
    }
    shake256_inc_ctx_release(&state);
}

/*************************************************
* Name:        polyeta_pack
*
* Description: Bit-pack polynomial with coefficients in [-ETA,ETA].
*
* Arguments:   - uint8_t *r: pointer to output byte array with at least
*                            POLYETA_PACKEDBYTES bytes
*              - const poly *a: pointer to input polynomial
**************************************************/
void polyeta_pack(uint8_t *r, const poly *a) {
    unsigned int i;
    uint8_t t[8];
    DBENCH_START();


    for (i = 0; i < N / 8; ++i) {
        t[0] = ETA - a->coeffs[8 * i + 0];
        t[1] = ETA - a->coeffs[8 * i + 1];
        t[2] = ETA - a->coeffs[8 * i + 2];
        t[3] = ETA - a->coeffs[8 * i + 3];
        t[4] = ETA - a->coeffs[8 * i + 4];
        t[5] = ETA - a->coeffs[8 * i + 5];
        t[6] = ETA - a->coeffs[8 * i + 6];
        t[7] = ETA - a->coeffs[8 * i + 7];

        r[3 * i + 0]  = (t[0] >> 0) | (t[1] << 3) | (t[2] << 6);
        r[3 * i + 1]  = (t[2] >> 2) | (t[3] << 1) | (t[4] << 4) | (t[5] << 7);
        r[3 * i + 2]  = (t[5] >> 1) | (t[6] << 2) | (t[7] << 5);
    }


    DBENCH_STOP(*tpack);
}

/*************************************************
* Name:        polyeta_unpack
*
* Description: Unpack polynomial with coefficients in [-ETA,ETA].
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *a: byte array with bit-packed polynomial
**************************************************/
void polyeta_unpack(poly *r, const uint8_t *a) {
    unsigned int i;
    DBENCH_START();


    for (i = 0; i < N / 8; ++i) {
        r->coeffs[8 * i + 0] =  (a[3 * i + 0] >> 0) & 7;
        r->coeffs[8 * i + 1] =  (a[3 * i + 0] >> 3) & 7;
        r->coeffs[8 * i + 2] = ((a[3 * i + 0] >> 6) | (a[3 * i + 1] << 2)) & 7;
        r->coeffs[8 * i + 3] =  (a[3 * i + 1] >> 1) & 7;
        r->coeffs[8 * i + 4] =  (a[3 * i + 1] >> 4) & 7;
        r->coeffs[8 * i + 5] = ((a[3 * i + 1] >> 7) | (a[3 * i + 2] << 1)) & 7;
        r->coeffs[8 * i + 6] =  (a[3 * i + 2] >> 2) & 7;
        r->coeffs[8 * i + 7] =  (a[3 * i + 2] >> 5) & 7;

        r->coeffs[8 * i + 0] = ETA - r->coeffs[8 * i + 0];
        r->coeffs[8 * i + 1] = ETA - r->coeffs[8 * i + 1];
        r->coeffs[8 * i + 2] = ETA - r->coeffs[8 * i + 2];
        r->coeffs[8 * i + 3] = ETA - r->coeffs[8 * i + 3];
        r->coeffs[8 * i + 4] = ETA - r->coeffs[8 * i + 4];
        r->coeffs[8 * i + 5] = ETA - r->coeffs[8 * i + 5];
        r->coeffs[8 * i + 6] = ETA - r->coeffs[8 * i + 6];
        r->coeffs[8 * i + 7] = ETA - r->coeffs[8 * i + 7];
    }


    DBENCH_STOP(*tpack);
}

/*************************************************
* Name:        polyt1_pack
*
* Description: Bit-pack polynomial t1 with coefficients fitting in 10 bits.
*              Input coefficients are assumed to be standard representatives.
*
* Arguments:   - uint8_t *r: pointer to output byte array with at least
*                            POLYT1_PACKEDBYTES bytes
*              - const poly *a: pointer to input polynomial
**************************************************/
void polyt1_pack(uint8_t *r, const poly *a) {
    unsigned int i;
    DBENCH_START();

    for (i = 0; i < N / 4; ++i) {
        r[5 * i + 0] = (uint8_t) (a->coeffs[4 * i + 0] >> 0);
        r[5 * i + 1] = (uint8_t) ((a->coeffs[4 * i + 0] >> 8) | (a->coeffs[4 * i + 1] << 2));
        r[5 * i + 2] = (uint8_t) ((a->coeffs[4 * i + 1] >> 6) | (a->coeffs[4 * i + 2] << 4));
        r[5 * i + 3] = (uint8_t) ((a->coeffs[4 * i + 2] >> 4) | (a->coeffs[4 * i + 3] << 6));
        r[5 * i + 4] = (uint8_t) (a->coeffs[4 * i + 3] >> 2);
    }

    DBENCH_STOP(*tpack);
}

/*************************************************
* Name:        polyt1_unpack
*
* Description: Unpack polynomial t1 with 10-bit coefficients.
*              Output coefficients are standard representatives.
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *a: byte array with bit-packed polynomial
**************************************************/
extern void PQCLEAN_DILITHIUM2_AARCH64__asm_10_to_32(int32_t *, const uint8_t *);
void polyt1_unpack(poly *r, const uint8_t *a) {
    DBENCH_START();

    PQCLEAN_DILITHIUM2_AARCH64__asm_10_to_32(r->coeffs, a);

    DBENCH_STOP(*tpack);
}

/*************************************************
* Name:        polyt0_pack
*
* Description: Bit-pack polynomial t0 with coefficients in ]-2^{D-1}, 2^{D-1}].
*
* Arguments:   - uint8_t *r: pointer to output byte array with at least
*                            POLYT0_PACKEDBYTES bytes
*              - const poly *a: pointer to input polynomial
**************************************************/
void polyt0_pack(uint8_t *r, const poly *a) {
    unsigned int i;
    uint32_t t[8];
    DBENCH_START();

    for (i = 0; i < N / 8; ++i) {
        t[0] = (1 << (D - 1)) - a->coeffs[8 * i + 0];
        t[1] = (1 << (D - 1)) - a->coeffs[8 * i + 1];
        t[2] = (1 << (D - 1)) - a->coeffs[8 * i + 2];
        t[3] = (1 << (D - 1)) - a->coeffs[8 * i + 3];
        t[4] = (1 << (D - 1)) - a->coeffs[8 * i + 4];
        t[5] = (1 << (D - 1)) - a->coeffs[8 * i + 5];
        t[6] = (1 << (D - 1)) - a->coeffs[8 * i + 6];
        t[7] = (1 << (D - 1)) - a->coeffs[8 * i + 7];

        r[13 * i + 0]  =  (uint8_t) t[0];
        r[13 * i + 1]  =  (uint8_t) (t[0] >>  8);
        r[13 * i + 1] |=  (uint8_t) (t[1] <<  5);
        r[13 * i + 2]  =  (uint8_t) (t[1] >>  3);
        r[13 * i + 3]  =  (uint8_t) (t[1] >> 11);
        r[13 * i + 3] |=  (uint8_t) (t[2] <<  2);
        r[13 * i + 4]  =  (uint8_t) (t[2] >>  6);
        r[13 * i + 4] |=  (uint8_t) (t[3] <<  7);
        r[13 * i + 5]  =  (uint8_t) (t[3] >>  1);
        r[13 * i + 6]  =  (uint8_t) (t[3] >>  9);
        r[13 * i + 6] |=  (uint8_t) (t[4] <<  4);
        r[13 * i + 7]  =  (uint8_t) (t[4] >>  4);
        r[13 * i + 8]  =  (uint8_t) (t[4] >> 12);
        r[13 * i + 8] |=  (uint8_t) (t[5] <<  1);
        r[13 * i + 9]  =  (uint8_t) (t[5] >>  7);
        r[13 * i + 9] |=  (uint8_t) (t[6] <<  6);
        r[13 * i + 10]  =  (uint8_t) (t[6] >>  2);
        r[13 * i + 11]  =  (uint8_t) (t[6] >> 10);
        r[13 * i + 11] |=  (uint8_t) (t[7] <<  3);
        r[13 * i + 12]  =  (uint8_t) (t[7] >>  5);
    }

    DBENCH_STOP(*tpack);
}

/*************************************************
* Name:        polyt0_unpack
*
* Description: Unpack polynomial t0 with coefficients in ]-2^{D-1}, 2^{D-1}].
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *a: byte array with bit-packed polynomial
**************************************************/
void polyt0_unpack(poly *r, const uint8_t *a) {
    unsigned int i;
    DBENCH_START();

    for (i = 0; i < N / 8; ++i) {
        r->coeffs[8 * i + 0]  = a[13 * i + 0];
        r->coeffs[8 * i + 0] |= (uint32_t)a[13 * i + 1] << 8;
        r->coeffs[8 * i + 0] &= 0x1FFF;

        r->coeffs[8 * i + 1]  = a[13 * i + 1] >> 5;
        r->coeffs[8 * i + 1] |= (uint32_t)a[13 * i + 2] << 3;
        r->coeffs[8 * i + 1] |= (uint32_t)a[13 * i + 3] << 11;
        r->coeffs[8 * i + 1] &= 0x1FFF;

        r->coeffs[8 * i + 2]  = a[13 * i + 3] >> 2;
        r->coeffs[8 * i + 2] |= (uint32_t)a[13 * i + 4] << 6;
        r->coeffs[8 * i + 2] &= 0x1FFF;

        r->coeffs[8 * i + 3]  = a[13 * i + 4] >> 7;
        r->coeffs[8 * i + 3] |= (uint32_t)a[13 * i + 5] << 1;
        r->coeffs[8 * i + 3] |= (uint32_t)a[13 * i + 6] << 9;
        r->coeffs[8 * i + 3] &= 0x1FFF;

        r->coeffs[8 * i + 4]  = a[13 * i + 6] >> 4;
        r->coeffs[8 * i + 4] |= (uint32_t)a[13 * i + 7] << 4;
        r->coeffs[8 * i + 4] |= (uint32_t)a[13 * i + 8] << 12;
        r->coeffs[8 * i + 4] &= 0x1FFF;

        r->coeffs[8 * i + 5]  = a[13 * i + 8] >> 1;
        r->coeffs[8 * i + 5] |= (uint32_t)a[13 * i + 9] << 7;
        r->coeffs[8 * i + 5] &= 0x1FFF;

        r->coeffs[8 * i + 6]  = a[13 * i + 9] >> 6;
        r->coeffs[8 * i + 6] |= (uint32_t)a[13 * i + 10] << 2;
        r->coeffs[8 * i + 6] |= (uint32_t)a[13 * i + 11] << 10;
        r->coeffs[8 * i + 6] &= 0x1FFF;

        r->coeffs[8 * i + 7]  = a[13 * i + 11] >> 3;
        r->coeffs[8 * i + 7] |= (uint32_t)a[13 * i + 12] << 5;
        r->coeffs[8 * i + 7] &= 0x1FFF;

        r->coeffs[8 * i + 0] = (1 << (D - 1)) - r->coeffs[8 * i + 0];
        r->coeffs[8 * i + 1] = (1 << (D - 1)) - r->coeffs[8 * i + 1];
        r->coeffs[8 * i + 2] = (1 << (D - 1)) - r->coeffs[8 * i + 2];
        r->coeffs[8 * i + 3] = (1 << (D - 1)) - r->coeffs[8 * i + 3];
        r->coeffs[8 * i + 4] = (1 << (D - 1)) - r->coeffs[8 * i + 4];
        r->coeffs[8 * i + 5] = (1 << (D - 1)) - r->coeffs[8 * i + 5];
        r->coeffs[8 * i + 6] = (1 << (D - 1)) - r->coeffs[8 * i + 6];
        r->coeffs[8 * i + 7] = (1 << (D - 1)) - r->coeffs[8 * i + 7];
    }

    DBENCH_STOP(*tpack);
}

/*************************************************
* Name:        polyz_pack
*
* Description: Bit-pack polynomial with coefficients
*              in [-(GAMMA1 - 1), GAMMA1].
*
* Arguments:   - uint8_t *r: pointer to output byte array with at least
*                            POLYZ_PACKEDBYTES bytes
*              - const poly *a: pointer to input polynomial
**************************************************/
void polyz_pack(uint8_t *r, const poly *a) {
    unsigned int i;
    uint32_t t[4];
    DBENCH_START();

    #if GAMMA1 == (1 << 17)

    for (i = 0; i < N / 4; ++i) {
        t[0] = GAMMA1 - a->coeffs[4 * i + 0];
        t[1] = GAMMA1 - a->coeffs[4 * i + 1];
        t[2] = GAMMA1 - a->coeffs[4 * i + 2];
        t[3] = GAMMA1 - a->coeffs[4 * i + 3];

        r[9 * i + 0]  = t[0];
        r[9 * i + 1]  = t[0] >> 8;
        r[9 * i + 2]  = t[0] >> 16;
        r[9 * i + 2] |= t[1] << 2;
        r[9 * i + 3]  = t[1] >> 6;
        r[9 * i + 4]  = t[1] >> 14;
        r[9 * i + 4] |= t[2] << 4;
        r[9 * i + 5]  = t[2] >> 4;
        r[9 * i + 6]  = t[2] >> 12;
        r[9 * i + 6] |= t[3] << 6;
        r[9 * i + 7]  = t[3] >> 2;
        r[9 * i + 8]  = t[3] >> 10;
    }

    #elif GAMMA1 == (1 << 19)

    for (i = 0; i < N / 2; ++i) {
        t[0] = GAMMA1 - a->coeffs[2 * i + 0];
        t[1] = GAMMA1 - a->coeffs[2 * i + 1];

        r[5 * i + 0]  = t[0];
        r[5 * i + 1]  = t[0] >> 8;
        r[5 * i + 2]  = t[0] >> 16;
        r[5 * i + 2] |= t[1] << 4;
        r[5 * i + 3]  = t[1] >> 4;
        r[5 * i + 4]  = t[1] >> 12;
    }

    #else

#error "No parameter specified!"

    #endif

    DBENCH_STOP(*tpack);
}

/*************************************************
* Name:        polyz_unpack
*
* Description: Unpack polynomial z with coefficients
*              in [-(GAMMA1 - 1), GAMMA1].
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *a: byte array with bit-packed polynomial
**************************************************/
void polyz_unpack(poly *r, const uint8_t *a) {
    unsigned int i;
    DBENCH_START();

    #if GAMMA1 == (1 << 17)

    for (i = 0; i < N / 4; ++i) {
        r->coeffs[4 * i + 0]  = a[9 * i + 0];
        r->coeffs[4 * i + 0] |= (uint32_t)a[9 * i + 1] << 8;
        r->coeffs[4 * i + 0] |= (uint32_t)a[9 * i + 2] << 16;
        r->coeffs[4 * i + 0] &= 0x3FFFF;

        r->coeffs[4 * i + 1]  = a[9 * i + 2] >> 2;
        r->coeffs[4 * i + 1] |= (uint32_t)a[9 * i + 3] << 6;
        r->coeffs[4 * i + 1] |= (uint32_t)a[9 * i + 4] << 14;
        r->coeffs[4 * i + 1] &= 0x3FFFF;

        r->coeffs[4 * i + 2]  = a[9 * i + 4] >> 4;
        r->coeffs[4 * i + 2] |= (uint32_t)a[9 * i + 5] << 4;
        r->coeffs[4 * i + 2] |= (uint32_t)a[9 * i + 6] << 12;
        r->coeffs[4 * i + 2] &= 0x3FFFF;

        r->coeffs[4 * i + 3]  = a[9 * i + 6] >> 6;
        r->coeffs[4 * i + 3] |= (uint32_t)a[9 * i + 7] << 2;
        r->coeffs[4 * i + 3] |= (uint32_t)a[9 * i + 8] << 10;
        r->coeffs[4 * i + 3] &= 0x3FFFF;

        r->coeffs[4 * i + 0] = GAMMA1 - r->coeffs[4 * i + 0];
        r->coeffs[4 * i + 1] = GAMMA1 - r->coeffs[4 * i + 1];
        r->coeffs[4 * i + 2] = GAMMA1 - r->coeffs[4 * i + 2];
        r->coeffs[4 * i + 3] = GAMMA1 - r->coeffs[4 * i + 3];
    }

    #elif GAMMA1 == (1 << 19)

    for (i = 0; i < N / 2; ++i) {
        r->coeffs[2 * i + 0]  = a[5 * i + 0];
        r->coeffs[2 * i + 0] |= (uint32_t)a[5 * i + 1] << 8;
        r->coeffs[2 * i + 0] |= (uint32_t)a[5 * i + 2] << 16;
        r->coeffs[2 * i + 0] &= 0xFFFFF;

        r->coeffs[2 * i + 1]  = a[5 * i + 2] >> 4;
        r->coeffs[2 * i + 1] |= (uint32_t)a[5 * i + 3] << 4;
        r->coeffs[2 * i + 1] |= (uint32_t)a[5 * i + 4] << 12;
        r->coeffs[2 * i + 0] &= 0xFFFFF;

        r->coeffs[2 * i + 0] = GAMMA1 - r->coeffs[2 * i + 0];
        r->coeffs[2 * i + 1] = GAMMA1 - r->coeffs[2 * i + 1];
    }

    #else

#error "No parameter specified!"

    #endif

    DBENCH_STOP(*tpack);
}

/*************************************************
* Name:        polyw1_pack
*
* Description: Bit-pack polynomial w1 with coefficients in [0,15] or [0,43].
*              Input coefficients are assumed to be standard representatives.
*
* Arguments:   - uint8_t *r: pointer to output byte array with at least
*                            POLYW1_PACKEDBYTES bytes
*              - const poly *a: pointer to input polynomial
**************************************************/
void polyw1_pack(uint8_t *r, const poly *a) {
    unsigned int i;
    DBENCH_START();

    #if GAMMA2 == (DILITHIUM_Q-1)/88

    for (i = 0; i < N / 4; ++i) {
        r[3 * i + 0]  = a->coeffs[4 * i + 0];
        r[3 * i + 0] |= a->coeffs[4 * i + 1] << 6;
        r[3 * i + 1]  = a->coeffs[4 * i + 1] >> 2;
        r[3 * i + 1] |= a->coeffs[4 * i + 2] << 4;
        r[3 * i + 2]  = a->coeffs[4 * i + 2] >> 4;
        r[3 * i + 2] |= a->coeffs[4 * i + 3] << 2;
    }

    #elif GAMMA2 == (DILITHIUM_Q-1)/32

    for (i = 0; i < N / 2; ++i) {
        r[i] = a->coeffs[2 * i + 0] | (a->coeffs[2 * i + 1] << 4);
    }

    #else

#error "No parameter specified!"

    #endif

    DBENCH_STOP(*tpack);
}
