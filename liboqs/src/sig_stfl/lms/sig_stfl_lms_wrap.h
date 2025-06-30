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

// SPDX-License-Identifier: MIT

#ifndef OQS_SIG_STFL_LMS_WRAP_H
#define OQS_SIG_STFL_LMS_WRAP_H

//#include <oqs/oqs.h>
#include "external/hss.h"
#include "external/hss_sign_inc.h"

/**
 * @brief OQS_LMS_KEY object for HSS key pair
 */
typedef struct OQS_LMS_KEY_DATA oqs_lms_key_data;

typedef struct OQS_LMS_SIG_DATA oqs_lms_sig_data;

typedef struct OQS_LMS_SIG_DATA {

	/* message buffer */
	unsigned char *message;

	/* Length of msg buffer */
	size_t len_msg_buf;

	/* signature buffer */
	unsigned char *signature;

	/* Length of sig buffer */
	size_t len_sig_buf;

} oqs_lms_sig_data;

#endif //OQS_SIG_STFL_LMS_H
