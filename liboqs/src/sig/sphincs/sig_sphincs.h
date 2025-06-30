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

#ifndef OQS_SIG_SPHINCS_H
#define OQS_SIG_SPHINCS_H

#include <oqs/oqs.h>

#if defined(OQS_ENABLE_SIG_sphincs_sha2_128f_simple)
#define OQS_SIG_sphincs_sha2_128f_simple_length_public_key 32
#define OQS_SIG_sphincs_sha2_128f_simple_length_secret_key 64
#define OQS_SIG_sphincs_sha2_128f_simple_length_signature 17088

OQS_SIG *OQS_SIG_sphincs_sha2_128f_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128f_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128f_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128f_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128f_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128f_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_sha2_128s_simple)
#define OQS_SIG_sphincs_sha2_128s_simple_length_public_key 32
#define OQS_SIG_sphincs_sha2_128s_simple_length_secret_key 64
#define OQS_SIG_sphincs_sha2_128s_simple_length_signature 7856

OQS_SIG *OQS_SIG_sphincs_sha2_128s_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128s_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128s_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128s_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128s_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_128s_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_sha2_192f_simple)
#define OQS_SIG_sphincs_sha2_192f_simple_length_public_key 48
#define OQS_SIG_sphincs_sha2_192f_simple_length_secret_key 96
#define OQS_SIG_sphincs_sha2_192f_simple_length_signature 35664

OQS_SIG *OQS_SIG_sphincs_sha2_192f_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192f_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192f_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192f_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192f_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192f_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_sha2_192s_simple)
#define OQS_SIG_sphincs_sha2_192s_simple_length_public_key 48
#define OQS_SIG_sphincs_sha2_192s_simple_length_secret_key 96
#define OQS_SIG_sphincs_sha2_192s_simple_length_signature 16224

OQS_SIG *OQS_SIG_sphincs_sha2_192s_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192s_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192s_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192s_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192s_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_192s_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_sha2_256f_simple)
#define OQS_SIG_sphincs_sha2_256f_simple_length_public_key 64
#define OQS_SIG_sphincs_sha2_256f_simple_length_secret_key 128
#define OQS_SIG_sphincs_sha2_256f_simple_length_signature 49856

OQS_SIG *OQS_SIG_sphincs_sha2_256f_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256f_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256f_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256f_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256f_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256f_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_sha2_256s_simple)
#define OQS_SIG_sphincs_sha2_256s_simple_length_public_key 64
#define OQS_SIG_sphincs_sha2_256s_simple_length_secret_key 128
#define OQS_SIG_sphincs_sha2_256s_simple_length_signature 29792

OQS_SIG *OQS_SIG_sphincs_sha2_256s_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256s_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256s_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256s_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256s_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_sha2_256s_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_shake_128f_simple)
#define OQS_SIG_sphincs_shake_128f_simple_length_public_key 32
#define OQS_SIG_sphincs_shake_128f_simple_length_secret_key 64
#define OQS_SIG_sphincs_shake_128f_simple_length_signature 17088

OQS_SIG *OQS_SIG_sphincs_shake_128f_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128f_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128f_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128f_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128f_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128f_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_shake_128s_simple)
#define OQS_SIG_sphincs_shake_128s_simple_length_public_key 32
#define OQS_SIG_sphincs_shake_128s_simple_length_secret_key 64
#define OQS_SIG_sphincs_shake_128s_simple_length_signature 7856

OQS_SIG *OQS_SIG_sphincs_shake_128s_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128s_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128s_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128s_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128s_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_128s_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_shake_192f_simple)
#define OQS_SIG_sphincs_shake_192f_simple_length_public_key 48
#define OQS_SIG_sphincs_shake_192f_simple_length_secret_key 96
#define OQS_SIG_sphincs_shake_192f_simple_length_signature 35664

OQS_SIG *OQS_SIG_sphincs_shake_192f_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192f_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192f_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192f_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192f_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192f_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_shake_192s_simple)
#define OQS_SIG_sphincs_shake_192s_simple_length_public_key 48
#define OQS_SIG_sphincs_shake_192s_simple_length_secret_key 96
#define OQS_SIG_sphincs_shake_192s_simple_length_signature 16224

OQS_SIG *OQS_SIG_sphincs_shake_192s_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192s_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192s_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192s_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192s_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_192s_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_shake_256f_simple)
#define OQS_SIG_sphincs_shake_256f_simple_length_public_key 64
#define OQS_SIG_sphincs_shake_256f_simple_length_secret_key 128
#define OQS_SIG_sphincs_shake_256f_simple_length_signature 49856

OQS_SIG *OQS_SIG_sphincs_shake_256f_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256f_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256f_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256f_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256f_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256f_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_sphincs_shake_256s_simple)
#define OQS_SIG_sphincs_shake_256s_simple_length_public_key 64
#define OQS_SIG_sphincs_shake_256s_simple_length_secret_key 128
#define OQS_SIG_sphincs_shake_256s_simple_length_signature 29792

OQS_SIG *OQS_SIG_sphincs_shake_256s_simple_new(void);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256s_simple_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256s_simple_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256s_simple_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256s_simple_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_sphincs_shake_256s_simple_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#endif
