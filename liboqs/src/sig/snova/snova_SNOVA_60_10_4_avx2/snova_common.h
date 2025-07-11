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
 * Constant time functions and miscellaneous.
 * Put in a separate file to prevent compiler over-optimisation.
 *
 * See "Compiler-introduced timing leak in Kyber reference implementation"
 * https://groups.google.com/a/list.nist.gov/g/pqc-forum/c/hqbtIGFKIpU
 */

#ifndef SNOVA_COMMON_H
#define SNOVA_COMMON_H

#include <stdint.h>
#include <stddef.h>

#include "snova.h"

int ct_is_negative(int val);
uint32_t ct_xgf16_is_not_zero(uint32_t val);
uint32_t ct_gf16_is_not_zero(uint8_t val);

void snova_set_zero(void *ptr, size_t size);
#define SNOVA_CLEAR(x) snova_set_zero(x, sizeof(x));

#define SNOVA_CLEAR_BYTE(x, byte) snova_set_zero(x, byte);

void convert_bytes_to_GF16s(const uint8_t *byte_array, uint8_t *gf16_array, int num_of_GF16s);
void convert_GF16s_to_bytes(uint8_t *byte_array, const uint8_t *gf16_array, int num_of_GF16s);
void convert_bytes_to_GF16s_cut_in_half(const uint8_t *byte_array, uint8_t *gf16_array, int num_of_GF16s);
void convert_GF16s_to_bytes_merger_in_half(uint8_t *byte_array, uint8_t *gf16_array, int num_of_GF16s);

void shake256(const uint8_t *pt_seed_array, int input_bytes, uint8_t *pt_output_array, int output_bytes);
void snova_shake(const uint8_t *seed, size_t input_bytes, uint64_t *data, size_t num_bytes);

#endif
