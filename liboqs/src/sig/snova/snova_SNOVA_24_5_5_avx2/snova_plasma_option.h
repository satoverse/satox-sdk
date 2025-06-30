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

#ifndef PLASMA_OPTION_H
#define PLASMA_OPTION_H

#if OPTIMISATION == 2
#include "snova_plasma_vec.h"
#include "snova_plasma_vec_gnl_sign.h"
#include "snova_plasma_vec_gnl_verify.h"
#elif OPTIMISATION == 1
#include "snova_opt.h"
#else
#error
#endif

// ---
#if OPTIMISATION >= 2
#define gen_P22 gen_P22_vtl
#define gen_F gen_F_vtl
#define sign_digest_core sign_digest_core_gnl_vtl
#define verify_core verify_signture_vtl
#define verify_pkx_core verify_signture_pkx_vtl
#elif OPTIMISATION == 1
#define gen_F gen_F_opt
#define gen_P22 gen_P22_opt
#define sign_digest_core sign_digest_core_opt
#define verify_core verify_signture_opt
#define verify_pkx_core verify_signture_pkx_opt
#endif

static void snova_plasma_init(void) {
	static int first_plasma_time = 1;
	if (first_plasma_time) {
		first_plasma_time = 0;
#if OPTIMISATION == 2
		init_vector_table();
#endif
	}
}

#endif
