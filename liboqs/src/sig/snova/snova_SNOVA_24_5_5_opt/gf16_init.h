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

#include <stdint.h>
#ifndef GF16_INIT_H
#define GF16_INIT_H
#include "gf16.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * init gf16 tables
 */
static void init_gf16_tables(void) {
	static int gf16_tables_is_init = 0;
	if (gf16_tables_is_init) {
		return;
	}
	gf16_tables_is_init = 1;
	uint8_t F_star[15] = {1, 2,  4, 8,  3,  6,  12, 11,
	                      5, 10, 7, 14, 15, 13, 9
	                     };  // Z2[x]/(x^4+x+1)
	for (int i = 0; i < 16; ++i) {
		mt(0, i) = mt(i, 0) = 0;
	}

	for (int i = 0; i < 15; ++i)
		for (int j = 0; j < 15; ++j) {
			mt(F_star[i], F_star[j]) = F_star[(i + j) % 15];
		}
	{
		int g = F_star[1], g_inv = F_star[14], gn = 1, gn_inv = 1;
		inv4b[0] = 0;
		inv4b[1] = 1;
		for (int index = 0; index < 14; index++) {
			inv4b[(gn = mt(gn, g))] = (gn_inv = mt(gn_inv, g_inv));
		}
	}
}

#ifdef __cplusplus
}
#endif

#endif
