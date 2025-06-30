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

#ifndef OQS_SIG_UOV_H
#define OQS_SIG_UOV_H

#include <oqs/oqs.h>

#if defined(OQS_ENABLE_SIG_uov_ov_Is)
#define OQS_SIG_uov_ov_Is_length_public_key 412160
#define OQS_SIG_uov_ov_Is_length_secret_key 348704
#define OQS_SIG_uov_ov_Is_length_signature 96

OQS_SIG *OQS_SIG_uov_ov_Is_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_Ip)
#define OQS_SIG_uov_ov_Ip_length_public_key 278432
#define OQS_SIG_uov_ov_Ip_length_secret_key 237896
#define OQS_SIG_uov_ov_Ip_length_signature 128

OQS_SIG *OQS_SIG_uov_ov_Ip_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_III)
#define OQS_SIG_uov_ov_III_length_public_key 1225440
#define OQS_SIG_uov_ov_III_length_secret_key 1044320
#define OQS_SIG_uov_ov_III_length_signature 200

OQS_SIG *OQS_SIG_uov_ov_III_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_V)
#define OQS_SIG_uov_ov_V_length_public_key 2869440
#define OQS_SIG_uov_ov_V_length_secret_key 2436704
#define OQS_SIG_uov_ov_V_length_signature 260

OQS_SIG *OQS_SIG_uov_ov_V_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_Is_pkc)
#define OQS_SIG_uov_ov_Is_pkc_length_public_key 66576
#define OQS_SIG_uov_ov_Is_pkc_length_secret_key 348704
#define OQS_SIG_uov_ov_Is_pkc_length_signature 96

OQS_SIG *OQS_SIG_uov_ov_Is_pkc_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_Ip_pkc)
#define OQS_SIG_uov_ov_Ip_pkc_length_public_key 43576
#define OQS_SIG_uov_ov_Ip_pkc_length_secret_key 237896
#define OQS_SIG_uov_ov_Ip_pkc_length_signature 128

OQS_SIG *OQS_SIG_uov_ov_Ip_pkc_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_III_pkc)
#define OQS_SIG_uov_ov_III_pkc_length_public_key 189232
#define OQS_SIG_uov_ov_III_pkc_length_secret_key 1044320
#define OQS_SIG_uov_ov_III_pkc_length_signature 200

OQS_SIG *OQS_SIG_uov_ov_III_pkc_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_V_pkc)
#define OQS_SIG_uov_ov_V_pkc_length_public_key 446992
#define OQS_SIG_uov_ov_V_pkc_length_secret_key 2436704
#define OQS_SIG_uov_ov_V_pkc_length_signature 260

OQS_SIG *OQS_SIG_uov_ov_V_pkc_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_Is_pkc_skc)
#define OQS_SIG_uov_ov_Is_pkc_skc_length_public_key 66576
#define OQS_SIG_uov_ov_Is_pkc_skc_length_secret_key 32
#define OQS_SIG_uov_ov_Is_pkc_skc_length_signature 96

OQS_SIG *OQS_SIG_uov_ov_Is_pkc_skc_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_skc_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_skc_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_skc_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_skc_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Is_pkc_skc_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_Ip_pkc_skc)
#define OQS_SIG_uov_ov_Ip_pkc_skc_length_public_key 43576
#define OQS_SIG_uov_ov_Ip_pkc_skc_length_secret_key 32
#define OQS_SIG_uov_ov_Ip_pkc_skc_length_signature 128

OQS_SIG *OQS_SIG_uov_ov_Ip_pkc_skc_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_skc_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_skc_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_skc_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_skc_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_Ip_pkc_skc_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_III_pkc_skc)
#define OQS_SIG_uov_ov_III_pkc_skc_length_public_key 189232
#define OQS_SIG_uov_ov_III_pkc_skc_length_secret_key 32
#define OQS_SIG_uov_ov_III_pkc_skc_length_signature 200

OQS_SIG *OQS_SIG_uov_ov_III_pkc_skc_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_skc_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_skc_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_skc_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_skc_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_III_pkc_skc_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#if defined(OQS_ENABLE_SIG_uov_ov_V_pkc_skc)
#define OQS_SIG_uov_ov_V_pkc_skc_length_public_key 446992
#define OQS_SIG_uov_ov_V_pkc_skc_length_secret_key 32
#define OQS_SIG_uov_ov_V_pkc_skc_length_signature 260

OQS_SIG *OQS_SIG_uov_ov_V_pkc_skc_new(void);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_skc_keypair(uint8_t *public_key, uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_skc_sign(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_skc_verify(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *public_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_skc_sign_with_ctx_str(uint8_t *signature, size_t *signature_len, const uint8_t *message, size_t message_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *secret_key);
OQS_API OQS_STATUS OQS_SIG_uov_ov_V_pkc_skc_verify_with_ctx_str(const uint8_t *message, size_t message_len, const uint8_t *signature, size_t signature_len, const uint8_t *ctx, size_t ctxlen, const uint8_t *public_key);
#endif

#endif
