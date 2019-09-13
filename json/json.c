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
 * Second stage JSON parser
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define JSMN_HEADER
#include "json.h"

#define BUFFER_SIZE 32768
#define JSON_TOKENS 256

jsmntok_t * json_tokenise(char *js)
{
        jsmn_parser parser;
        jsmn_init(&parser);

        unsigned int n = JSON_TOKENS;
        jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * n);
        /* oracle code analyser doesn't know that the assert *is*
         * a check on the return value of malloc(3C) */
        assert(tokens);

        int ret = jsmn_parse(&parser, js, strlen(js), tokens, n);

        while (ret == JSMN_ERROR_NOMEM)
        {
                n = n * 2 + 1;
                tokens = realloc(tokens, sizeof(jsmntok_t) * n);
                /* same here */
                assert(tokens);
                ret = jsmn_parse(&parser, js, strlen(js), tokens, n);
        }

        if (ret == JSMN_ERROR_INVAL)
                fprintf(stderr, "jsmn_parse: invalid JSON string");
        if (ret == JSMN_ERROR_PART)
                fprintf(stderr, "jsmn_parse: truncated JSON string");

        return tokens;
}

bool json_token_streq(char *js, jsmntok_t *t, char *s)
{
        return (strncmp(js + t->start, s, t->end - t->start) == 0
            && strlen(s) == (size_t) (t->end - t->start));
}

char * json_token_tostr(char *js, jsmntok_t *t)
{
        js[t->end] = '\0';
        return js + t->start;
}
