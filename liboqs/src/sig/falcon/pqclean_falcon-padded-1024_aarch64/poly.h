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

#ifndef POLY_H
#define POLY_H

#include "inner.h"
#include "params.h"

typedef enum ntt_domain {
    NTT_NONE = 0,
    NTT_MONT = 1,
    NTT_MONT_INV = 2,
} ntt_domain_t;

typedef enum invntt_domain {
    INVNTT_NONE = 0,
    INVNTT_NINV = 1,
} invntt_domain_t;

void PQCLEAN_FALCONPADDED1024_AARCH64_poly_ntt(int16_t a[FALCON_N], ntt_domain_t mont);

void PQCLEAN_FALCONPADDED1024_AARCH64_poly_invntt(int16_t a[FALCON_N], invntt_domain_t ninv);

void PQCLEAN_FALCONPADDED1024_AARCH64_poly_int8_to_int16(int16_t out[FALCON_N], const int8_t in[FALCON_N]);

void PQCLEAN_FALCONPADDED1024_AARCH64_poly_div_12289(int16_t f[FALCON_N], const int16_t g[FALCON_N]);

void PQCLEAN_FALCONPADDED1024_AARCH64_poly_convert_to_unsigned(int16_t f[FALCON_N]);

uint16_t PQCLEAN_FALCONPADDED1024_AARCH64_poly_compare_with_zero(int16_t f[FALCON_N]);

void PQCLEAN_FALCONPADDED1024_AARCH64_poly_montmul_ntt(int16_t f[FALCON_N], const int16_t g[FALCON_N]);

void PQCLEAN_FALCONPADDED1024_AARCH64_poly_sub_barrett(int16_t f[FALCON_N], const int16_t g[FALCON_N], const int16_t s[FALCON_N]);

int PQCLEAN_FALCONPADDED1024_AARCH64_poly_int16_to_int8(int8_t G[FALCON_N], const int16_t t[FALCON_N]);

int PQCLEAN_FALCONPADDED1024_AARCH64_poly_check_bound_int8(const int8_t t[FALCON_N],
        const int8_t low, const int8_t high);

int PQCLEAN_FALCONPADDED1024_AARCH64_poly_check_bound_int16(const int16_t t[FALCON_N],
        const int16_t low, const int16_t high);

#endif
