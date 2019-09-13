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

/* buf: a sized buffer type. */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "buf.h"

buf_t * buf_size(buf_t *buf, size_t len)
{
        if (buf == NULL)
        {
                buf = malloc(sizeof(buf_t));
                assert(buf);
                buf->data = NULL;
                buf->len = 0;
        }

        buf->data = realloc(buf->data, len);
        assert(buf->data);

        if (buf->len > len)
                buf->len = len;
        buf->limit = len;

        return buf;
}

void buf_push(buf_t *buf, uint8_t c)
{
        assert(buf);

        assert(buf->len < buf->limit);

        buf->data[buf->len++] = c;
}

void buf_concat(buf_t *dst, uint8_t *src, size_t len)
{
        assert(dst);
        assert(src);

        log_assert(dst->len + len <= dst->limit);

        for (size_t i = 0; i < len; i++)
                dst->data[dst->len++] = src[i];
}

char * buf_tostr(buf_t *buf)
{
        assert(buf);

        char *str = malloc(buf->len + 1);
        assert(str);

        memcpy(str, buf->data, buf->len);
        str[buf->len] = '\0';

        return str;
}
