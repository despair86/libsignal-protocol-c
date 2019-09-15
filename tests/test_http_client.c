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

/* Web client unit test*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef __sun
#include <alloca.h>
#endif
#include "http.h"

const char *test_uri = "https://motherfuckingwebsite.com/";

main(argc, argv)
char** argv;
{
    bool b;
    size_t size;
    int r;

    printf("Web Client Unit Test\n");
    b = http_client_init();
    unsigned char* out = alloca(8192);
    if (!b)
    {
        printf("Failed to start web client\n");
        return -1;
    }

    r = http_request(test_uri, NULL, NULL, GET, HTTP_NONE, 0, out, &size, true);
    if (r == 200)
    {
        printf("Status: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, size, out);
        printf("Simple HTTP GET unit test passed.\n");
        http_client_cleanup();
        return 0;
    }
    else
    {
        printf("Status: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, size, out);
        http_client_cleanup();
        printf("Unit test failed!\n");
        return -1;
    }
}
