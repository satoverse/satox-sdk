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
#include "polyvec.h"
#include <stdint.h>

#include "reduce.h"

static const int32_t l_montgomery_const[4] = {
    DILITHIUM_Q, DILITHIUM_QINV
};

/*************************************************
* Name:        expand_mat
*
* Description: Implementation of ExpandA. Generates matrix A with uniformly
*              random coefficients a_{i,j} by performing rejection
*              sampling on the output stream of SHAKE128(rho|j|i)
*              or AES256CTR(rho,j|i).
*
* Arguments:   - polyvecl mat[K]: output matrix
*              - const uint8_t rho[]: byte array containing seed rho
**************************************************/
void polyvec_matrix_expand(polyvecl mat[K], const uint8_t rho[SEEDBYTES]) {
    unsigned int i, j;

    for (j = 0; j < L; ++j) {
        for (i = 0; i < K; i += 2) {
            poly_uniformx2(&mat[i + 0].vec[j], &mat[i + 1].vec[j], rho, (uint16_t) ((i << 8) + j), (uint16_t) (((i + 1) << 8) + j));
        }
    }
}

void polyvec_matrix_pointwise_montgomery(polyveck *t, const polyvecl mat[K], const polyvecl *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        polyvecl_pointwise_acc_montgomery(&t->vec[i], &mat[i], v);
    }
}

/**************************************************************/
/************ Vectors of polynomials of length L **************/
/**************************************************************/

void polyvecl_uniform_eta(polyvecl *v, const uint8_t seed[CRHBYTES], uint16_t nonce) {
    unsigned int i;

    for (i = 0; i < L; ++i) {
        poly_uniform_eta(&v->vec[i], seed, nonce++);
    }
}

void polyvecl_uniform_gamma1(polyvecl *v, const uint8_t seed[CRHBYTES], uint16_t nonce) {
    unsigned int i;

    for (i = 0; i < L - 1; i += 2) {
        poly_uniform_gamma1x2(&v->vec[i + 0], &v->vec[i + 1], seed, (uint16_t) (L * nonce + i + 0), (uint16_t) (L * nonce + i + 1));
    }
    if (L & 1) {
        poly_uniform_gamma1(&v->vec[i], seed, (uint16_t) (L * nonce + L - 1));
    }
}

void polyvecl_reduce(polyvecl *v) {
    unsigned int i;

    for (i = 0; i < L; ++i) {
        poly_reduce(&v->vec[i]);
    }
}

/*************************************************
* Name:        polyvecl_freeze
*
* Description: Reduce coefficients of polynomials in vector of length L
*              to standard representatives.
*
* Arguments:   - polyvecl *v: pointer to input/output vector
**************************************************/
void polyvecl_freeze(polyvecl *v) {
    unsigned int i;

    for (i = 0; i < L; ++i) {
        poly_freeze(&v->vec[i]);
    }
}

/*************************************************
* Name:        polyvecl_add
*
* Description: Add vectors of polynomials of length L.
*              No modular reduction is performed.
*
* Arguments:   - polyvecl *w: pointer to output vector
*              - const polyvecl *u: pointer to first summand
*              - const polyvecl *v: pointer to second summand
**************************************************/
void polyvecl_add(polyvecl *w, const polyvecl *u, const polyvecl *v) {
    unsigned int i;

    for (i = 0; i < L; ++i) {
        poly_add(&w->vec[i], &u->vec[i], &v->vec[i]);
    }
}

/*************************************************
* Name:        polyvecl_ntt
*
* Description: Forward NTT of all polynomials in vector of length L. Output
*              coefficients can be up to 16*Q larger than input coefficients.
*
* Arguments:   - polyvecl *v: pointer to input/output vector
**************************************************/
void polyvecl_ntt(polyvecl *v) {
    unsigned int i;

    for (i = 0; i < L; ++i) {
        poly_ntt(&v->vec[i]);
    }
}

void polyvecl_invntt_tomont(polyvecl *v) {
    unsigned int i;

    for (i = 0; i < L; ++i) {
        poly_invntt_tomont(&v->vec[i]);
    }
}

void polyvecl_pointwise_poly_montgomery(polyvecl *r, const poly *a, const polyvecl *v) {
    unsigned int i;

    for (i = 0; i < L; ++i) {
        poly_pointwise_montgomery(&r->vec[i], a, &v->vec[i]);
    }
}

/*************************************************
* Name:        polyvecl_pointwise_acc_montgomery
*
* Description: Pointwise multiply vectors of polynomials of length L, multiply
*              resulting vector by 2^{-32} and add (accumulate) polynomials
*              in it. Input/output vectors are in NTT domain representation.
*
* Arguments:   - poly *w: output polynomial
*              - const polyvecl *u: pointer to first input vector
*              - const polyvecl *v: pointer to second input vector
**************************************************/
extern void PQCLEAN_DILITHIUM3_AARCH64__asm_polyvecl_pointwise_acc_montgomery(int32_t *, const int32_t *, const int32_t *, const int32_t *);
void polyvecl_pointwise_acc_montgomery(poly *w,
                                       const polyvecl *u,
                                       const polyvecl *v) {
    PQCLEAN_DILITHIUM3_AARCH64__asm_polyvecl_pointwise_acc_montgomery(w->coeffs, u->vec[0].coeffs, v->vec[0].coeffs, l_montgomery_const);
}

/*************************************************
* Name:        polyvecl_chknorm
*
* Description: Check infinity norm of polynomials in vector of length L.
*              Assumes input polyvecl to be reduced by polyvecl_reduce().
*
* Arguments:   - const polyvecl *v: pointer to vector
*              - int32_t B: norm bound
*
* Returns 0 if norm of all polynomials is strictly smaller than B <= (Q-1)/8
* and 1 otherwise.
**************************************************/
int polyvecl_chknorm(const polyvecl *v, int32_t bound)  {
    unsigned int i;

    for (i = 0; i < L; ++i) {
        if (poly_chknorm(&v->vec[i], bound)) {
            return 1;
        }
    }

    return 0;
}

/**************************************************************/
/************ Vectors of polynomials of length K **************/
/**************************************************************/

void polyveck_uniform_eta(polyveck *v, const uint8_t seed[CRHBYTES], uint16_t nonce) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_uniform_eta(&v->vec[i], seed, nonce++);
    }

}

/*************************************************
* Name:        polyveck_reduce
*
* Description: Reduce coefficients of polynomials in vector of length K
*              to representatives in [-6283009,6283007].
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_reduce(polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_reduce(&v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_caddq
*
* Description: For all coefficients of polynomials in vector of length K
*              add Q if coefficient is negative.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_caddq(polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_caddq(&v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_freeze
*
* Description: Reduce coefficients of polynomials in vector of length K
*              to standard representatives.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_freeze(polyveck *v)  {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_freeze(&v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_add
*
* Description: Add vectors of polynomials of length K.
*              No modular reduction is performed.
*
* Arguments:   - polyveck *w: pointer to output vector
*              - const polyveck *u: pointer to first summand
*              - const polyveck *v: pointer to second summand
**************************************************/
void polyveck_add(polyveck *w, const polyveck *u, const polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_add(&w->vec[i], &u->vec[i], &v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_sub
*
* Description: Subtract vectors of polynomials of length K.
*              No modular reduction is performed.
*
* Arguments:   - polyveck *w: pointer to output vector
*              - const polyveck *u: pointer to first input vector
*              - const polyveck *v: pointer to second input vector to be
*                                   subtracted from first input vector
**************************************************/
void polyveck_sub(polyveck *w, const polyveck *u, const polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_sub(&w->vec[i], &u->vec[i], &v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_shiftl
*
* Description: Multiply vector of polynomials of Length K by 2^D without modular
*              reduction. Assumes input coefficients to be less than 2^{31-D}.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_shiftl(polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_shiftl(&v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_ntt
*
* Description: Forward NTT of all polynomials in vector of length K. Output
*              coefficients can be up to 16*Q larger than input coefficients.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_ntt(polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_ntt(&v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_invntt_tomont
*
* Description: Inverse NTT and multiplication by 2^{32} of polynomials
*              in vector of length K. Input coefficients need to be less
*              than 2*Q.
*
* Arguments:   - polyveck *v: pointer to input/output vector
**************************************************/
void polyveck_invntt_tomont(polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_invntt_tomont(&v->vec[i]);
    }
}

void polyveck_pointwise_poly_montgomery(polyveck *r, const poly *a, const polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_pointwise_montgomery(&r->vec[i], a, &v->vec[i]);
    }
}


/*************************************************
* Name:        polyveck_chknorm
*
* Description: Check infinity norm of polynomials in vector of length K.
*              Assumes input polyveck to be reduced by polyveck_reduce().
*
* Arguments:   - const polyveck *v: pointer to vector
*              - int32_t B: norm bound
*
* Returns 0 if norm of all polynomials are strictly smaller than B <= (Q-1)/8
* and 1 otherwise.
**************************************************/
int polyveck_chknorm(const polyveck *v, int32_t bound) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        if (poly_chknorm(&v->vec[i], bound)) {
            return 1;
        }
    }

    return 0;
}

/*************************************************
* Name:        polyveck_power2round
*
* Description: For all coefficients a of polynomials in vector of length K,
*              compute a0, a1 such that a mod^+ Q = a1*2^D + a0
*              with -2^{D-1} < a0 <= 2^{D-1}. Assumes coefficients to be
*              standard representatives.
*
* Arguments:   - polyveck *v1: pointer to output vector of polynomials with
*                              coefficients a1
*              - polyveck *v0: pointer to output vector of polynomials with
*                              coefficients a0
*              - const polyveck *v: pointer to input vector
**************************************************/
void polyveck_power2round(polyveck *v1, polyveck *v0, const polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_power2round(&v1->vec[i], &v0->vec[i], &v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_decompose
*
* Description: For all coefficients a of polynomials in vector of length K,
*              compute high and low bits a0, a1 such a mod^+ Q = a1*ALPHA + a0
*              with -ALPHA/2 < a0 <= ALPHA/2 except a1 = (Q-1)/ALPHA where we
*              set a1 = 0 and -ALPHA/2 <= a0 = a mod Q - Q < 0.
*              Assumes coefficients to be standard representatives.
*
* Arguments:   - polyveck *v1: pointer to output vector of polynomials with
*                              coefficients a1
*              - polyveck *v0: pointer to output vector of polynomials with
*                              coefficients a0
*              - const polyveck *v: pointer to input vector
**************************************************/
void polyveck_decompose(polyveck *v1, polyveck *v0, const polyveck *v) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_decompose(&v1->vec[i], &v0->vec[i], &v->vec[i]);
    }
}

/*************************************************
* Name:        polyveck_make_hint
*
* Description: Compute hint vector.
*
* Arguments:   - polyveck *h: pointer to output vector
*              - const polyveck *v0: pointer to low part of input vector
*              - const polyveck *v1: pointer to high part of input vector
*
* Returns number of 1 bits.
**************************************************/
unsigned int polyveck_make_hint(polyveck *h,
                                const polyveck *v0,
                                const polyveck *v1) {
    unsigned int i, s = 0;

    for (i = 0; i < K; ++i) {
        s += poly_make_hint(&h->vec[i], &v0->vec[i], &v1->vec[i]);
    }

    return s;
}

/*************************************************
* Name:        polyveck_use_hint
*
* Description: Use hint vector to correct the high bits of input vector.
*
* Arguments:   - polyveck *w: pointer to output vector of polynomials with
*                             corrected high bits
*              - const polyveck *u: pointer to input vector
*              - const polyveck *h: pointer to input hint vector
**************************************************/
void polyveck_use_hint(polyveck *w, const polyveck *u, const polyveck *h) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        poly_use_hint(&w->vec[i], &u->vec[i], &h->vec[i]);
    }
}

void polyveck_pack_w1(uint8_t r[K * POLYW1_PACKEDBYTES], const polyveck *w1) {
    unsigned int i;

    for (i = 0; i < K; ++i) {
        polyw1_pack(&r[i * POLYW1_PACKEDBYTES], &w1->vec[i]);
    }
}
