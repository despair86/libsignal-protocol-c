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

/* Web client unit test */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __sun
#include <alloca.h>
#endif
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include "http.h"

const char *test_uri = "https://motherfuckingwebsite.com/";
const char *test_uri_insecure = "http://www.gnu.org";

main(argc, argv)
char** argv;
{
    bool b, tests_passed[2];
    size_t size;
    int r;
    unsigned char* out;

    printf("Web Client Unit Test\n");
    b = http_client_init();
    out = alloca(16384);
	tests_passed[0] = false;
	tests_passed[1] = false;
    size = 16384;
    memset(out, 0, 16384);

    if (!b)
    {
        printf("Failed to start web client\n");
        return -1;
    }

    r = http_request(test_uri, NULL, NULL, GET, HTTP_NONE, 0, out, &size, true);
    if (r == 200)
    {
#ifdef _MSC_VER
        printf("Status: %d\nSize: %d\nResponse:\n--->%s<---\n", r, size, out);
#else
        printf("Status: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, size, out);
#endif
        printf("Simple HTTPS GET unit test passed without error.\n");
        tests_passed[0] = true;
    }
    else
    {
#ifdef _MSC_VER
        printf("Status: %d\nSize: %d\nResponse:\n--->%s<---\n", r, size, out);
#else
        printf("Status: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, size, out);
#endif
        if (r > 0)
        {
            printf("Unit test proceeded with errors.\n");
            tests_passed[0] = true;
        }
        else
            printf("Unit test failed!\n");
    }

    memset(out, 0, 16384);
    size = 16384;
    r = http_request(test_uri_insecure, NULL, NULL, GET, HTTP_NONE, 0, out, &size, true);
    if (r == 200)
    {
#ifdef _MSC_VER
        printf("Status: %d\nSize: %d\nResponse:\n--->%s<---\n", r, size, out);
#else
        printf("Status: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, size, out);
#endif
        printf("Simple HTTP GET unit test passed without error.\n");
        tests_passed[1] = true;
    }
    else
    {
#ifdef _MSC_VER
        printf("Status: %d\nSize: %d\nResponse:\n--->%s<---\n", r, size, out);
#else
        printf("Status: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, size, out);
#endif
        if (r > 0)
        {
            printf("Unit test proceeded with errors.\n");
            tests_passed[1] = true;
        }
        else
            printf("Unit test failed!\n");
    }

    http_client_cleanup();
    if (tests_passed[0] && tests_passed[1])
    {
        printf("All unit tests passed.\n");
        return 0;
    }
    else
    {
        printf("One or more unit tests failed, see output for details.\n");
        return -1;
    }
}