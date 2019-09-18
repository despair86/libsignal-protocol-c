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

/* 
 * File:   loki.h
 * Author: despair
 * API for Loki blockchain integration
 *
 * Created on September 16, 2019, 7:58 PM
 */

#ifndef LOKI_H
#define LOKI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

    /* User must manually scrub+free returned buffer */
    unsigned char* calcPoW(int64_t timestamp, int32_t ttl, const unsigned char *pubKey, const unsigned char *data, size_t data_size);
    /* this is probably useful elsewhere, libloki as such will contain these util routines */
    unsigned char* bufferToBase64(void* buf, size_t size);


#ifdef __cplusplus
}
#endif

#endif /* LOKI_H */

