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
 * Proof-of-work implementation, see 
 * loki-messenger:clearnet/libloki/proof-of-work.js for reference.
 */

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <loki.h>
#include <mbedtls/base64.h>
#include <mbedtls/base64.h>
#include <mbedtls/sha512.h>
#include <mbedtls/platform_util.h>

#define NONCE_SIZE 8

uint64_t CURRENT_NET_DIFFICULTY = 10;

/* User must free() returned buffer when finished */
unsigned char* bufferToBase64(void* buf, size_t size)
{
    unsigned char* tmp;
    size_t out_size;
    int r;

    /* Get the size of the output */
    tmp = NULL;
    mbedtls_base64_encode(tmp, 0, &out_size, (unsigned char*) buf, size);

    tmp = malloc(out_size);
    assert(tmp);
    r = mbedtls_base64_encode(tmp, out_size, &out_size, (unsigned char*) buf, size);
    if (r)
    {
        fprintf(stderr, "Failed to encode binary data\n");
        /* May contain nuts, crush them before returning  */
        mbedtls_platform_zeroize(tmp, out_size);
        free(tmp);
        return NULL;
    }
    return tmp;
}

static uint64_t getDiffTgt(int32_t ttl, size_t payload_size)
{
    uint64_t x1, x2, x3, x4, x5;
    size_t sz;
    int32_t ttlInSecs;

    x1 = (1ULL << 16) - 1;
    x2 = UINT64_MAX - 1;
    sz = payload_size + NONCE_SIZE;
    ttlInSecs = ttl / 1000;
    x3 = (ttlInSecs * sz) / x1;
    x4 = sz + x3;
    /* CURRENT_NET_DIFFICULTY is a global that is updated every time 
     * a message is sent to a DM channel. Initially 10
     */
    x5 = CURRENT_NET_DIFFICULTY * x4;
    return x2 / x5;
}

unsigned char* calcPoW(int64_t timestamp, int32_t ttl, const unsigned char *pubKey, const unsigned char *data, size_t data_size)
{
#ifndef _MSC_VER
    unsigned char payload[data_size + 16384];
#else
    unsigned char *payload;
#endif
    uint64_t diffTgt, ctr;
    int64_t nonce;
    unsigned char initial_hash[64], hash[64], tmp[NONCE_SIZE + 64];

#ifdef _MSC_VER
    payload = alloca(data_size + 16384);
#endif
    /* scrub everything before we start, some platforms flip out on writes to uninitialised buffers */
    mbedtls_platform_zeroize(payload, data_size + 16384);
    snprintf(payload, data_size + 16384, "%" PRIi64 "%d%s%s", timestamp, ttl, pubKey, data);
    diffTgt = getDiffTgt(ttl, strlen(payload));
    ctr = UINT64_MAX;
    nonce = 0;
    mbedtls_sha512_ret(payload, strlen(payload), initial_hash, 0);
    do
    {
        nonce += 1;
        memcpy(tmp, nonce, NONCE_SIZE);
        memcpy(tmp + NONCE_SIZE, initial_hash, 64);
        mbedtls_sha512_ret(tmp, NONCE_SIZE + 64, hash, 0);
        memcpy(ctr, hash, NONCE_SIZE);
    }
    while (ctr > diffTgt);
    mbedtls_platform_zeroize(payload, data_size + 16384);
    return bufferToBase64(nonce, NONCE_SIZE);
}
