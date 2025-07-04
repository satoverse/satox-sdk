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

#ifndef CONFIG_H
#define CONFIG_H

#include "params.h"

#define FIXED_ABQ (l_SNOVA < 4)

#define seed_length_public 16
#define seed_length_private 32
#define seed_length (seed_length_public + seed_length_private)

#define n_SNOVA  (v_SNOVA + o_SNOVA)
#define m_SNOVA  (o_SNOVA)
#define lsq_SNOVA  (l_SNOVA * l_SNOVA)
#define alpha_SNOVA (l_SNOVA * l_SNOVA + l_SNOVA)

#define GF16s_hash (o_SNOVA * lsq_SNOVA)
#define GF16s_signature (n_SNOVA * lsq_SNOVA)
#define bytes_hash ((GF16s_hash + 1) >> 1)

#define rank (l_SNOVA)
#define sq_rank (rank * rank)   // matrix size

#define bytes_signature ((GF16s_signature + 1) >> 1)
#define bytes_salt 16
#define bytes_sig_with_salt (bytes_signature + bytes_salt)

#define GF16s_prng_public (sq_rank * (2 * (m_SNOVA * alpha_SNOVA) + m_SNOVA * (n_SNOVA * n_SNOVA - m_SNOVA * m_SNOVA)) + rank * 2 * m_SNOVA * alpha_SNOVA)
//                                                     A B ^^^              P11 P12 P21 ^^^                                Q1 Q2 ^^^ ps. Q1 matrix prng is rank, not sq_rank.
#define bytes_prng_public ((GF16s_prng_public + 1) >> 1)

#define GF16s_prng_private (v_SNOVA * o_SNOVA * rank)
//                            T12 matrix prng is rank, not sq_rank.
#define bytes_prng_private ((GF16s_prng_private + 1) >> 1)

#define bytes_pk (seed_length_public + ((m_SNOVA * o_SNOVA * o_SNOVA * lsq_SNOVA + 1) >> 1))
#define bytes_expend_pk (seed_length_public + ((m_SNOVA *  (n_SNOVA * n_SNOVA + 4 * alpha_SNOVA) * sq_rank) + 1) / 2)
//                                                 P11 P12 P21 P22 ^^^     A B Q1 Q2 ^^^

#define bytes_sk (((sq_rank * (4 * m_SNOVA * alpha_SNOVA + m_SNOVA* (v_SNOVA * v_SNOVA + v_SNOVA * o_SNOVA + o_SNOVA * v_SNOVA) + v_SNOVA * o_SNOVA) + 1) >> 1) + seed_length_public + seed_length_private)
//                                         ABQ ^^^                   F11 F12 F21 ^^^                                               T12 ^^^

#endif
