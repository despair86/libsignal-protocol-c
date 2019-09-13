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
 * File:   buf.h
 * Author: despair
 *
 * Created on September 13, 2019, 3:41 AM
 */

#ifndef BUF_H
#define BUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    size_t len;        // current length of buffer (used bytes)
    size_t limit;      // maximum length of buffer (allocated)
    uint8_t *data;     // insert bytes here
} buf_t;


buf_t * buf_size(buf_t *buf, size_t len);
void buf_push(buf_t *buf, uint8_t c);
void buf_concat(buf_t *buf, uint8_t *data, size_t len);
char * buf_tostr(buf_t *buf);


#ifdef __cplusplus
}
#endif

#endif /* BUF_H */

