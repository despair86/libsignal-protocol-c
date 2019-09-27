/*
 * Copyright (C) 2019 Rick V. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CRYPTO_PROVIDER_MBEDTLS_H
#define CRYPTO_PROVIDER_MBEDTLS_H

#include <limits.h>

#include "signal_protocol.h"

 /* mbedtls crypto provider */
int mbedtls_random_generator(uint8_t* data, size_t len, void* user_data);
int mbedtls_hmac_sha256_init(void** hmac_context, const uint8_t* key, size_t key_len, void* user_data);
int mbedtls_hmac_sha256_update(void* hmac_context, const uint8_t* data, size_t data_len, void* user_data);
int mbedtls_hmac_sha256_final(void* hmac_context, signal_buffer** output, void* user_data);
void mbedtls_hmac_sha256_cleanup(void* hmac_context, void* user_data);
int mbedtls_sha512_digest_init(void** digest_context, void* user_data);
int mbedtls_sha512_digest_update(void* digest_context, const uint8_t* data, size_t data_len, void* user_data);
int mbedtls_sha512_digest_final(void* digest_context, signal_buffer** output, void* user_data);
void mbedtls_sha512_digest_cleanup(void* digest_context, void* user_data);

int mbedtls_encrypt(signal_buffer** output,
	int cipher,
	const uint8_t* key, size_t key_len,
	const uint8_t* iv, size_t iv_len,
	const uint8_t* plaintext, size_t plaintext_len,
	void* user_data);
int mbedtls_decrypt(signal_buffer** output,
	int cipher,
	const uint8_t* key, size_t key_len,
	const uint8_t* iv, size_t iv_len,
	const uint8_t* ciphertext, size_t ciphertext_len,
	void* user_data);

#endif
