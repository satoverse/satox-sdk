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

#include <mayo.h>

#ifdef ENABLE_PARAMS_DYNAMIC
static const unsigned char f_tail_64[] = F_TAIL_64;
static const unsigned char f_tail_78[] = F_TAIL_78;
static const unsigned char f_tail_108[] = F_TAIL_108;
static const unsigned char f_tail_142[] = F_TAIL_142;

#define MAYO_GEN_PARAMS(nm) \
  const mayo_params_t nm = { \
    .m = PARAM_JOIN2(nm, m), \
    .n = PARAM_JOIN2(nm, n), \
    .o = PARAM_JOIN2(nm, o), \
    .k = PARAM_JOIN2(nm, k), \
    .q = PARAM_JOIN2(nm, q), \
    .f_tail = PARAM_JOIN2(nm, f_tail_arr), \
    .m_bytes = PARAM_JOIN2(nm, m_bytes), \
    .O_bytes = PARAM_JOIN2(nm, O_bytes), \
    .v_bytes = PARAM_JOIN2(nm, v_bytes), \
    .r_bytes = PARAM_JOIN2(nm, r_bytes), \
    .P1_bytes = PARAM_JOIN2(nm, P1_bytes), \
    .P2_bytes = PARAM_JOIN2(nm, P2_bytes), \
    .P3_bytes = PARAM_JOIN2(nm, P3_bytes), \
    .csk_bytes = PARAM_JOIN2(nm, csk_bytes), \
    .cpk_bytes = PARAM_JOIN2(nm, cpk_bytes), \
    .sig_bytes = PARAM_JOIN2(nm, sig_bytes), \
    .salt_bytes = PARAM_JOIN2(nm, salt_bytes), \
    .sk_seed_bytes = PARAM_JOIN2(nm, sk_seed_bytes), \
    .digest_bytes = PARAM_JOIN2(nm, digest_bytes), \
    .pk_seed_bytes = PARAM_JOIN2(nm, pk_seed_bytes), \
    .m_vec_limbs = PARAM_JOIN2(nm, m_vec_limbs), \
    .name = #nm \
  };

MAYO_GEN_PARAMS(MAYO_1);
MAYO_GEN_PARAMS(MAYO_2);
MAYO_GEN_PARAMS(MAYO_3);
MAYO_GEN_PARAMS(MAYO_5);
#endif

