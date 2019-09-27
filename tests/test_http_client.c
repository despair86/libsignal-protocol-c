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

char* test_uri = "https://motherfuckingwebsite.com/";
char* test_uri_insecure = "http://www.gnu.org";

main(argc, argv)
char** argv;
{
	bool b, tests_passed[2];
	int r;
	struct HttpResponse rsp;
	struct HttpRequest req;

	printf("Web Client Unit Test\n");
	b = http_client_init();
	tests_passed[0] = false;
	tests_passed[1] = false;
	req.c_type = HTTP_NONE;
	req.headers = NULL;
	req.rq_data = NULL;
	req.size = 0;
	req.uri = test_uri;
	req.verb = GET;

	if (!b)
	{
		printf("Failed to start web client\n");
		return -1;
	}

	r = http_request(&req, &rsp, true);
	if (r == 200)
	{
		printHeaders(&rsp.headers);
#ifdef _MSC_VER
		printf("---\nStatus: %d\nSize: %d\nResponse:\n--->%s<---\n", r, rsp.size, rsp.body);
#else
		printf("---\nStatus: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, rsp.size, rsp.body);
#endif
		printf("Simple HTTPS GET unit test passed without error.\n");
		tests_passed[0] = true;
		free(rsp.body);
		freeHeaders(&rsp.headers);
		rsp.headers.next = NULL;
	}
	else
	{
#ifdef _MSC_VER
		printf("Status: %d\nSize: %d\nResponse:\n--->%s<---\n", r, rsp.size, rsp.body);
#else
		printf("---\nStatus: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, rsp.size, rsp.body);
#endif
		if (r > 0)
		{
			printf("Unit test proceeded with errors.\n");
			printHeaders(&rsp.headers);
			tests_passed[0] = true;
			free(rsp.body);
			freeHeaders(&rsp.headers);
			rsp.headers.next = NULL;
		}
		else
			printf("Unit test failed!\n");
	}

	req.c_type = HTTP_NONE;
	req.headers = NULL;
	req.rq_data = NULL;
	req.size = 0;
	req.uri = test_uri_insecure;
	req.verb = GET;
	r = http_request(&req, &rsp, true);
	if (r == 200)
	{
		printHeaders(&rsp.headers);
#ifdef _MSC_VER
		printf("---\nStatus: %d\nSize: %d\nResponse:\n--->%s<---\n", r, rsp.size, rsp.body);
#else
		printf("---\nStatus: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, rsp.size, rsp.body);
#endif
		printf("Simple HTTP GET unit test passed without error.\n");
		tests_passed[1] = true;
		free(rsp.body);
		freeHeaders(&rsp.headers);
		rsp.headers.next = NULL;
	}
	else
	{
#ifdef _MSC_VER
		printf("---\nStatus: %d\nSize: %d\nResponse:\n--->%s<---\n", r, rsp.size, rsp.body);
#else
		printf("---\nStatus: %d\nSize: %zu\nResponse:\n--->%s<---\n", r, rsp.size, rsp.body);
#endif
		if (r > 0)
		{
			printf("Unit test proceeded with errors.\n");
			printHeaders(&rsp.headers);
			tests_passed[1] = true;
			free(rsp.body);
			freeHeaders(&rsp.headers);
			rsp.headers.next = NULL;
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