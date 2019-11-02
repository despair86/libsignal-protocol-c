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
 *
 * mbed TLS cryptographic service provider for Signal Protocol
 */

#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include <mbedtls/config.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/cipher.h>
#include <mbedtls/md.h>
#include "mbedtls/md_internal.h"
#include <mbedtls/platform_util.h>

#include "../src/signal_protocol.h"
#include "crypto_provider_mbedtls.h"

static mbedtls_ctr_drbg_context drbg_ctx;
static mbedtls_entropy_context rnd_ctx;
static int randumb_active = 0;
/* A so-called "device specific id" to seed the internal RNG - here it's app-specific */
static const unsigned char* APP_SEED_RNG = "mbedtls crypto provider for Session (lite)";

static int init_mbedtls_randumb()
{
	int r;

	mbedtls_ctr_drbg_init(&drbg_ctx);
	mbedtls_entropy_init(&rnd_ctx);
	r = mbedtls_ctr_drbg_seed(&drbg_ctx, mbedtls_entropy_func, &rnd_ctx, APP_SEED_RNG, strlen((char*)APP_SEED_RNG));
	if (r)
	{
		printf("failed to seed RNG!\n");
		mbedtls_ctr_drbg_free(&drbg_ctx);
		mbedtls_entropy_free(&rnd_ctx);
		return 0;
	}

	randumb_active = 1;
	return 1;
}

static const mbedtls_cipher_info_t* aes_cipher_select(int cipher, size_t key_len)
{
	if (cipher == SG_CIPHER_AES_CBC_PKCS5)
	{
		switch (key_len)
		{
		case 16:
			return mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CBC);
		case 24:
			return mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_192_CBC);
		case 32:
			return mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CBC);
		}
	}
	else if (cipher == SG_CIPHER_AES_CTR_NOPADDING)
	{
		switch (key_len)
		{
		case 16:
			return mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CTR);
		case 24:
			return mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_192_CTR);
		case 32:
			return mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CTR);
		}
	}
	return 0;
}

int mbedtls_random_generator(uint8_t* data, size_t len, void* user_data)
{
	if (!randumb_active)
		init_mbedtls_randumb();

	if (!mbedtls_ctr_drbg_random(&drbg_ctx, data, len))
	{
		return 0;
	}
	else
	{
		return SG_ERR_UNKNOWN;
	}
}

int mbedtls_hmac_sha256_init(void** hmac_context, const uint8_t* key, size_t key_len, void* user_data)
{
	mbedtls_md_context_t* ctx = malloc(sizeof(mbedtls_md_context_t));
	const mbedtls_md_info_t* md_info;
	if (!ctx)
		return SG_ERR_NOMEM;
	mbedtls_md_init(ctx);

	*hmac_context = ctx;

	md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
	if (mbedtls_md_setup(ctx, md_info, 1))
	{
		return SG_ERR_UNKNOWN;
	}
	if (mbedtls_md_hmac_starts(ctx, key, key_len))
		return SG_ERR_UNKNOWN;

	return 0;
}

int mbedtls_hmac_sha256_update(void* hmac_context, const uint8_t* data, size_t data_len, void* user_data)
{
	mbedtls_md_context_t* ctx = hmac_context;
	int result = mbedtls_md_hmac_update(ctx, data, data_len);
	return (!result) ? 0 : -1;
}

int mbedtls_hmac_sha256_final(void* hmac_context, signal_buffer** output, void* user_data)
{
	int result = 0;
	unsigned char md[MBEDTLS_MD_MAX_SIZE];
	unsigned int len;
	signal_buffer* output_buffer;
	mbedtls_md_context_t* ctx = hmac_context;
	len = ctx->md_info->size;

	if (mbedtls_md_hmac_finish(ctx, md))
	{
		return SG_ERR_UNKNOWN;
	}

	output_buffer = signal_buffer_create(md, len);
	if (!output_buffer)
	{
		result = SG_ERR_NOMEM;
		goto complete;
	}

	*output = output_buffer;

complete:
	return result;
}

void mbedtls_hmac_sha256_cleanup(void* hmac_context, void* user_data)
{
	if (hmac_context)
	{
		mbedtls_md_context_t* ctx = hmac_context;
		mbedtls_md_free(ctx);
		free(ctx);
	}
}

int mbedtls_sha512_digest_init(void** digest_context, void* user_data)
{
	int result = 0;
	const mbedtls_md_info_t* md_info;

	mbedtls_md_context_t* ctx = malloc(sizeof(mbedtls_md_context_t));
	if (!ctx)
	{
		result = SG_ERR_NOMEM;
		goto complete;
	}
	mbedtls_md_init(ctx);

	md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);

	if (mbedtls_md_setup(ctx, md_info, 0))
	{
		result = SG_ERR_UNKNOWN;
	}
	else
		result = SG_SUCCESS;

complete:
	if (result)
	{
		if (ctx)
		{
			mbedtls_md_free(ctx);
			free(ctx);
		}
	}
	else
	{
		mbedtls_md_starts(ctx);
		*digest_context = ctx;
	}
	return result;
}

int mbedtls_sha512_digest_update(void* digest_context, const uint8_t* data, size_t data_len, void* user_data)
{
	mbedtls_md_context_t* ctx = digest_context;
	int result = mbedtls_md_update(ctx, data, data_len);
	return (!result) ? SG_SUCCESS : SG_ERR_UNKNOWN;
}

int mbedtls_sha512_digest_final(void* digest_context, signal_buffer** output, void* user_data)
{
	int result = 0;
	unsigned char md[MBEDTLS_MD_MAX_SIZE];
	unsigned int len = MBEDTLS_MD_MAX_SIZE;
	signal_buffer* output_buffer;
	mbedtls_md_context_t* ctx = digest_context;

	result = mbedtls_md_finish(ctx, md);
	if (!result)
	{
		result = SG_SUCCESS;
	}
	else
	{
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	result = mbedtls_md_starts(ctx);
	if (!result)
	{
		result = SG_SUCCESS;
	}
	else
	{
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	output_buffer = signal_buffer_create(md, len);
	if (!output_buffer)
	{
		result = SG_ERR_NOMEM;
		goto complete;
	}

	*output = output_buffer;

complete:
	return result;
}

void mbedtls_sha512_digest_cleanup(void* digest_context, void* user_data)
{
	mbedtls_md_context_t* ctx = digest_context;
	mbedtls_md_free(ctx);
	free(ctx);
}

int mbedtls_encrypt(signal_buffer** output,
	int cipher,
	const uint8_t* key, size_t key_len,
	const uint8_t* iv, size_t iv_len,
	const uint8_t* plaintext, size_t plaintext_len,
	void* user_data)
{
	int result = 0;
	mbedtls_cipher_context_t* ctx = 0;
	uint8_t* out_buf = 0;
	size_t out_len, final_len;

	const mbedtls_cipher_info_t* evp_cipher = aes_cipher_select(cipher, key_len);
	if (!evp_cipher)
	{
		fprintf(stderr, "invalid AES mode or key size: %zu\n", key_len);
		return SG_ERR_UNKNOWN;
	}

	if (iv_len != 16)
	{
		fprintf(stderr, "invalid AES IV size: %zu\n", iv_len);
		return SG_ERR_UNKNOWN;
	}

	if (plaintext_len > INT_MAX - evp_cipher->block_size)
	{
		fprintf(stderr, "invalid plaintext length: %zu\n", plaintext_len);
		return SG_ERR_UNKNOWN;
	}

	ctx = malloc(sizeof(mbedtls_cipher_context_t));
	if (!ctx)
	{
		result = SG_ERR_NOMEM;
		goto complete;
	}
	mbedtls_cipher_init(ctx);

	result = mbedtls_cipher_setup(ctx, evp_cipher);
	result = mbedtls_cipher_set_iv(ctx, iv, iv_len);
	result = mbedtls_cipher_setkey(ctx, key, key_len * 8, MBEDTLS_ENCRYPT);
	if (result)
	{
		fprintf(stderr, "cannot initialize cipher\n");
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	if (cipher == SG_CIPHER_AES_CTR_NOPADDING)
	{
		result = mbedtls_cipher_set_padding_mode(ctx, MBEDTLS_PADDING_NONE);
		if (result)
		{
			fprintf(stderr, "cannot set padding\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}
	}

	out_buf = malloc(plaintext_len + evp_cipher->block_size);
	if (!out_buf)
	{
		fprintf(stderr, "cannot allocate output buffer\n");
		result = SG_ERR_NOMEM;
		goto complete;
	}

	out_len = 0;
	result = mbedtls_cipher_update(ctx, plaintext, plaintext_len, out_buf, &out_len);
	if (result)
	{
		fprintf(stderr, "cannot encrypt plaintext\n");
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	final_len = 0;
	result = mbedtls_cipher_finish(ctx, out_buf + out_len, &final_len);
	if (result)
	{
		fprintf(stderr, "cannot finish encrypting plaintext\n");
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	*output = signal_buffer_create(out_buf, out_len + final_len);

complete:
	if (ctx)
	{
		mbedtls_cipher_free(ctx);
		free(ctx);
	}
	if (out_buf)
	{
		free(out_buf);
	}
	return result;
}

int mbedtls_decrypt(signal_buffer** output,
	int cipher,
	const uint8_t* key, size_t key_len,
	const uint8_t* iv, size_t iv_len,
	const uint8_t* ciphertext, size_t ciphertext_len,
	void* user_data)
{
	int result = 0;
	mbedtls_cipher_context_t* ctx = 0;
	uint8_t* out_buf = 0;
	size_t out_len, final_len;

	const mbedtls_cipher_info_t* evp_cipher = aes_cipher_select(cipher, key_len);
	if (!evp_cipher)
	{
		fprintf(stderr, "invalid AES mode or key size: %zu\n", key_len);
		return SG_ERR_UNKNOWN;
	}

	if (iv_len != 16)
	{
		fprintf(stderr, "invalid AES IV size: %zu\n", iv_len);
		return SG_ERR_UNKNOWN;
	}

	if (ciphertext_len > INT_MAX - evp_cipher->block_size)
	{
		fprintf(stderr, "invalid ciphertext length: %zu\n", ciphertext_len);
		return SG_ERR_UNKNOWN;
	}

	ctx = malloc(sizeof(mbedtls_cipher_context_t));
	if (!ctx)
	{
		result = SG_ERR_NOMEM;
		goto complete;
	}
	mbedtls_cipher_init(ctx);

	result = mbedtls_cipher_setup(ctx, evp_cipher);
	result = mbedtls_cipher_set_iv(ctx, iv, iv_len);
	result = mbedtls_cipher_setkey(ctx, key, key_len * 8, MBEDTLS_DECRYPT);
	if (result)
	{
		fprintf(stderr, "cannot initialise cipher\n");
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	if (cipher == SG_CIPHER_AES_CTR_NOPADDING)
	{
		result = mbedtls_cipher_set_padding_mode(ctx, MBEDTLS_PADDING_NONE);
		if (result)
		{
			fprintf(stderr, "cannot set padding\n");
			result = SG_ERR_UNKNOWN;
			goto complete;
		}
	}

	out_buf = malloc(ciphertext_len + evp_cipher->block_size);
	if (!out_buf)
	{
		fprintf(stderr, "cannot allocate output buffer\n");
		result = SG_ERR_NOMEM;
		goto complete;
	}

	out_len = 0;
	result = mbedtls_cipher_update(ctx, ciphertext, ciphertext_len, out_buf, &out_len);
	if (result)
	{
		fprintf(stderr, "cannot decrypt ciphertext\n");
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	final_len = 0;
	result = mbedtls_cipher_finish(ctx, out_buf + out_len, &final_len);
	if (result)
	{
		fprintf(stderr, "cannot finish decrypting ciphertext\n");
		result = SG_ERR_UNKNOWN;
		goto complete;
	}

	*output = signal_buffer_create(out_buf, out_len + final_len);

complete:
	if (ctx)
	{
		mbedtls_cipher_free(ctx);
		free(ctx);
	}
	if (out_buf)
	{
		free(out_buf);
	}
	return result;
}

signal_crypto_provider mbedtls_signal_csp = {
	mbedtls_random_generator,
	mbedtls_hmac_sha256_init,
	mbedtls_hmac_sha256_update,
	mbedtls_hmac_sha256_final,
	mbedtls_hmac_sha256_cleanup,
	mbedtls_sha512_digest_init,
	mbedtls_sha512_digest_update,
	mbedtls_sha512_digest_final,
	mbedtls_sha512_digest_cleanup,
	mbedtls_encrypt,
	mbedtls_decrypt,
	NULL
};
