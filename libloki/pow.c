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

#include <mbedtls/base64.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/* User must free() returned buffer when finished */
unsigned char* bufferToBase64(unsigned char* buf, size_t size)
{
    unsigned char* tmp = malloc(size);
    assert(tmp);
    size_t out_size;
    int r;

    /* Get the size of the output */
    mbedtls_base64_encode(tmp, 0, &out_size, buf, size);

    if (out_size >= size)
        realloc(tmp, out_size + 1);

    assert(tmp);
    r = mbedtls_base64_encode(tmp, out_size, &out_size, buf, size);
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

