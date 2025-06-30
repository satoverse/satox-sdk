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
 * functions
 */

#include <stddef.h>
#include <assert.h>

#include "snova.h"

#if defined(SNOVA_LIBOQS)
#include <aes.h>

int AES_128_CTR(unsigned char *output, size_t outputByteLen,
                const unsigned char *input, size_t inputByteLen) {
	assert(inputByteLen > 0);
	const uint8_t iv[16] = {0};

	void *state;
	OQS_AES128_CTR_inc_init(input, &state);
	OQS_AES128_CTR_inc_stream_iv(iv, 12, state, output, outputByteLen);
	OQS_AES128_free_schedule(state);
	return (int)outputByteLen;

}

void AES_256_ECB(const unsigned char *key, const uint8_t *input, unsigned char *output) {
	void *schedule = NULL;
	OQS_AES256_ECB_load_schedule(key, &schedule);
	OQS_AES256_ECB_enc_sch(input, 16, schedule, output);
	OQS_AES256_free_schedule(schedule);
}

#else
#include "aes/aes_local.h"

#if __AVX2__

int AES_128_CTR(unsigned char *output, size_t outputByteLen,
                const unsigned char *input, size_t inputByteLen) {
	assert(inputByteLen > 0);
	const uint8_t iv[16] = {0};
	void *schedule = NULL;
	oqs_aes128_load_schedule_ni(input, &schedule);
	oqs_aes128_ctr_enc_sch_ni(iv, 16, schedule, output, outputByteLen);
	oqs_aes128_free_schedule_ni(schedule);
	return (int)outputByteLen;
}

void AES_256_ECB(const unsigned char *key, const uint8_t *input, unsigned char *output) {
	void *schedule = NULL;
	oqs_aes256_load_schedule_ni(key, &schedule);
	oqs_aes256_ecb_enc_sch_ni(input, 16, schedule, output);
	oqs_aes256_free_schedule_ni(schedule);
}

#elif __ARM_ARCH

int AES_128_CTR(unsigned char *output, size_t outputByteLen,
                const unsigned char *input, size_t inputByteLen) {
	assert(inputByteLen > 0);
	const uint8_t iv[16] = {0};
	void *schedule = NULL;
	oqs_aes128_load_schedule_no_bitslice(input, &schedule);
	oqs_aes128_ctr_enc_sch_armv8(iv, 16, schedule, output, outputByteLen);
	oqs_aes128_free_schedule_no_bitslice(schedule);
	return (int)outputByteLen;
}

void AES_256_ECB(const unsigned char *key, const uint8_t *input, unsigned char *output) {
	void *schedule = NULL;
	oqs_aes256_load_schedule_no_bitslice(key, &schedule);
	oqs_aes256_ecb_enc_sch_armv8(input, 16, schedule, output);
	oqs_aes256_free_schedule_no_bitslice(schedule);
}

#else

int AES_128_CTR(unsigned char *output, size_t outputByteLen,
                const unsigned char *input, size_t inputByteLen) {
	assert(inputByteLen > 0);
	const uint8_t iv[16] = {0};
	void *schedule = NULL;
	oqs_aes128_load_schedule_c(input, &schedule);
	oqs_aes128_ctr_enc_sch_c(iv, 16, schedule, output, outputByteLen);
	oqs_aes128_free_schedule_c(schedule);
	return (int)outputByteLen;
}

void AES_256_ECB(const unsigned char *key, const uint8_t *input, unsigned char *output) {
	void *schedule = NULL;
	oqs_aes256_load_schedule_c(key, &schedule);
	oqs_aes256_ecb_enc_sch_c(input, 16, schedule, output);
	oqs_aes256_free_schedule_c(schedule);
}

#endif
#endif

