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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cjson.h"

 /* TODO: add test vectors for object types expected by the pager */
const char* test_json = "{\"name\": \"despair\"}";

struct test
{
	char* name;
};

main(argc, argv)
char** argv;
{
	struct test t;
	char* pretty_json;
	cJSON* obj;

	puts("JSON parser unit test");
	printf("JSON test vector: %s\n", test_json);
	obj = cJSON_Parse(test_json);
	if (!obj)
		return -1;
	pretty_json = cJSON_Print(obj);
	printf("%s\n", pretty_json);
	t.name = strdup(cJSON_GetObjectItemCaseSensitive(obj, "name")->valuestring);
	cJSON_Delete(obj);
	printf("name: %s\n", t.name);
	free(t.name);
	free(pretty_json);
	return 0;
}
