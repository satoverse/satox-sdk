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

#include <stdlib.h>

#include <oqs/sig_snova.h>

#if defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4)
OQS_SIG *OQS_SIG_snova_SNOVA_60_10_4_new(void) {

	OQS_SIG *sig = OQS_MEM_malloc(sizeof(OQS_SIG));
	if (sig == NULL) {
		return NULL;
	}
	sig->method_name = OQS_SIG_alg_snova_SNOVA_60_10_4;
	sig->alg_version = "round2";

	sig->claimed_nist_level = 5;
	sig->euf_cma = true;
	sig->suf_cma = false;
	sig->sig_with_ctx_support = false;

	sig->length_public_key = OQS_SIG_snova_SNOVA_60_10_4_length_public_key;
	sig->length_secret_key = OQS_SIG_snova_SNOVA_60_10_4_length_secret_key;
	sig->length_signature = OQS_SIG_snova_SNOVA_60_10_4_length_signature;

	sig->keypair = OQS_SIG_snova_SNOVA_60_10_4_keypair;
	sig->sign = OQS_SIG_snova_SNOVA_60_10_4_sign;
	sig->verify = OQS_SIG_snova_SNOVA_60_10_4_verify;
	sig->sign_with_ctx_str = OQS_SIG_snova_SNOVA_60_10_4_sign_with_ctx_str;
	sig->verify_with_ctx_str = OQS_SIG_snova_SNOVA_60_10_4_verify_with_ctx_str;

	return sig;
}

extern int _snova_60_10_4_aes_ssk_opt_keypair(uint8_t *pk, uint8_t *sk);
extern int _snova_60_10_4_aes_ssk_opt_sign(uint8_t *sig, size_t *siglen, const uint8_t *m, size_t mlen, const uint8_t *sk);
extern int _snova_60_10_4_aes_ssk_opt_verify(const uint8_t *sig, size_t siglen, const uint8_t *m, size_t mlen, const uint8_t *pk);

#if defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4_avx2)
extern int _snova_60_10_4_aes_ssk_avx2_keypair(uint8_t *pk, uint8_t *sk);
extern int _snova_60_10_4_aes_ssk_avx2_sign(uint8_t *sig, size_t *siglen, const uint8_t *m, size_t mlen, const uint8_t *sk);
extern int _snova_60_10_4_aes_ssk_avx2_verify(const uint8_t *sig, size_t siglen, const uint8_t *m, size_t mlen, const uint8_t *pk);
#endif

#if defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4_neon)
extern int _snova_60_10_4_aes_ssk_neon_keypair(uint8_t *pk, uint8_t *sk);
extern int _snova_60_10_4_aes_ssk_neon_sign(uint8_t *sig, size_t *siglen, const uint8_t *m, size_t mlen, const uint8_t *sk);
extern int _snova_60_10_4_aes_ssk_neon_verify(const uint8_t *sig, size_t siglen, const uint8_t *m, size_t mlen, const uint8_t *pk);
#endif

OQS_API OQS_STATUS OQS_SIG_snova_SNOVA_60_10_4_keypair(uint8_t *public_key, uint8_t *secret_key) {
#if defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4_avx2)
#if defined(OQS_DIST_BUILD)
	if (OQS_CPU_has_extension(OQS_CPU_EXT_AVX2)) {
#endif /* OQS_DIST_BUILD */
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_avx2_keypair(public_key, secret_key);
#if defined(OQS_DIST_BUILD)
	} else {
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_keypair(public_key, secret_key);
	}
#endif /* OQS_DIST_BUILD */
#elif defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4_neon)
#if defined(OQS_DIST_BUILD)
	if (OQS_CPU_has_extension(OQS_CPU_EXT_ARM_NEON)) {
#endif /* OQS_DIST_BUILD */
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_neon_keypair(public_key, secret_key);
#if defined(OQS_DIST_BUILD)
	} else {
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_keypair(public_key, secret_key);
	}
#endif /* OQS_DIST_BUILD */
#else
	return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_keypair(public_key, secret_key);
#endif
}

OQS_API OQS_STATUS OQS_SIG_snova_SNOVA_60_10_4_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key) {
#if defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4_avx2)
#if defined(OQS_DIST_BUILD)
	if (OQS_CPU_has_extension(OQS_CPU_EXT_AVX2)) {
#endif /* OQS_DIST_BUILD */
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_avx2_sign(signature, signature_len, message, message_len, secret_key);
#if defined(OQS_DIST_BUILD)
	} else {
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_sign(signature, signature_len, message, message_len, secret_key);
	}
#endif /* OQS_DIST_BUILD */
#elif defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4_neon)
#if defined(OQS_DIST_BUILD)
	if (OQS_CPU_has_extension(OQS_CPU_EXT_ARM_NEON)) {
#endif /* OQS_DIST_BUILD */
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_neon_sign(signature, signature_len, message, message_len, secret_key);
#if defined(OQS_DIST_BUILD)
	} else {
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_sign(signature, signature_len, message, message_len, secret_key);
	}
#endif /* OQS_DIST_BUILD */
#else
	return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_sign(signature, signature_len, message, message_len, secret_key);
#endif
}

OQS_API OQS_STATUS OQS_SIG_snova_SNOVA_60_10_4_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key) {
#if defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4_avx2)
#if defined(OQS_DIST_BUILD)
	if (OQS_CPU_has_extension(OQS_CPU_EXT_AVX2)) {
#endif /* OQS_DIST_BUILD */
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_avx2_verify(signature, signature_len, message, message_len, public_key);
#if defined(OQS_DIST_BUILD)
	} else {
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_verify(signature, signature_len, message, message_len, public_key);
	}
#endif /* OQS_DIST_BUILD */
#elif defined(OQS_ENABLE_SIG_snova_SNOVA_60_10_4_neon)
#if defined(OQS_DIST_BUILD)
	if (OQS_CPU_has_extension(OQS_CPU_EXT_ARM_NEON)) {
#endif /* OQS_DIST_BUILD */
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_neon_verify(signature, signature_len, message, message_len, public_key);
#if defined(OQS_DIST_BUILD)
	} else {
		return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_verify(signature, signature_len, message, message_len, public_key);
	}
#endif /* OQS_DIST_BUILD */
#else
	return (OQS_STATUS) _snova_60_10_4_aes_ssk_opt_verify(signature, signature_len, message, message_len, public_key);
#endif
}

OQS_API OQS_STATUS OQS_SIG_snova_SNOVA_60_10_4_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx_str, size_t ctx_str_len, const uint8_t *secret_key) {
	if (ctx_str == NULL && ctx_str_len == 0) {
		return OQS_SIG_snova_SNOVA_60_10_4_sign(signature, signature_len, message, message_len, secret_key);
	} else {
		return OQS_ERROR;
	}
}

OQS_API OQS_STATUS OQS_SIG_snova_SNOVA_60_10_4_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx_str, size_t ctx_str_len, const uint8_t *public_key) {
	if (ctx_str == NULL && ctx_str_len == 0) {
		return OQS_SIG_snova_SNOVA_60_10_4_verify(message, message_len, signature, signature_len, public_key);
	} else {
		return OQS_ERROR;
	}
}
#endif
