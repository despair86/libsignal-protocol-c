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
 * create-restore seed mnemonic
 */

/* RFC: should I just tie this to whatever LC_ALL returns? */
#include "mnemonic.h"
#include "arraylist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson.h>

/**/
static get_prefix_length(l)
language_code l;
{
	switch (l) {
	case LANGUAGE_ZH_TW:
		return 1;
	case LANGUAGE_EN_ELECTRUM:
		return 0;
	case LANGUAGE_EN:
	case LANGUAGE_DEFAULT:
	case LANGUAGE_JP:
		return 3;
	default:
		return 4;
	}
}

/* Runs on demand, and by default the app will initialise only the
 * default language (en) 
 */
wordlist* initialise_wordlist(lc)
language_code lc;
{
	cJSON* raw_word_list, *e;
	FILE* json;
	int list_size;
	char* data;
	wordlist* w = NULL;

	w = malloc(sizeof (wordlist));
	data = malloc(65536);
	if (!w || !data)
		return NULL;
	memset(w, 0, sizeof (wordlist));
	switch (lc) {
	case LANGUAGE_DEFAULT:
	case LANGUAGE_EN:
		json = fopen("mnemonics/english.json", "rb");
		break;
	case LANGUAGE_ZH_TW:
		json = fopen("mnemonics/chinese_simplified.json", "rb");
		break;
	case LANGUAGE_NL:
		json = fopen("mnemonics/dutch.json", "rb");
		break;
	case LANGUAGE_EN_ELECTRUM:
		json = fopen("mnemonics/electrum.json", "rb");
		break;
	case LANGUAGE_EO:
		json = fopen("mnemonics/esperanto.json", "rb");
		break;
	case LANGUAGE_FR:
		json = fopen("mnemonics/french.json", "rb");
		break;
	case LANGUAGE_DE:
		json = fopen("mnemonics/german.json", "rb");
		break;
	case LANGUAGE_IT:
		json = fopen("mnemonics/italian.json", "rb");
		break;
	case LANGUAGE_JP:
		json = fopen("mnemonics/japanese.json", "rb");
		break;
	case LANGUAGE_LOJBAN:
		json = fopen("mnemonics/lojban.json", "rb");
		break;
	case LANGUAGE_PT:
		json = fopen("mnemonics/portuguese.json", "rb");
		break;
	case LANGUAGE_SU:
		json = fopen("mnemonics/russian.json", "rb");
		break;
	case LANGUAGE_ES:
		json = fopen("mnemonics/spanish.json", "rb");
		break;
	default:
		return NULL;
	}
	if (!json)
		return NULL;
	fread(data, sizeof (char), 65536, json);
	fclose(json);
	raw_word_list = cJSON_Parse(data);
	w->lc = lc;
	w->prefix_length = get_prefix_length(lc);
	list_size = cJSON_GetArraySize(raw_word_list);
	ARRAYLIST_RESIZE(w->words, list_size);
	if (lc != LANGUAGE_EN_ELECTRUM)
		ARRAYLIST_RESIZE(w->truncated_words, list_size);

	cJSON_ArrayForEach(e, raw_word_list)
	{
		size_t l = strlen(e->valuestring);
		char* value = malloc(l + 1);
		if (!value) {
			while(w->words.count)
			{
				value = ARRAYLIST_POP(w->words);
				free(value);
			}
			ARRAYLIST_FREE(w->words);
			return NULL;
		}
		strncpy(value, e->valuestring, l + 1);
		ARRAYLIST_PUSH(w->words, value);
	}
	if (lc != LANGUAGE_EN_ELECTRUM) 
	{
		cJSON_ArrayForEach(e, raw_word_list)
		{
			size_t l = strlen(e->valuestring);
			char* value = malloc(l + 1);
			if (!value) {
				while(w->words.count)
				{
					value = ARRAYLIST_POP(w->words);
					free(value);
				}
				ARRAYLIST_FREE(w->words);
				while(w->truncated_words.count)
				{
					value = ARRAYLIST_POP(w->truncated_words);
					free(value);
				}
				ARRAYLIST_FREE(w->truncated_words);
				return NULL;
			}
			memset(value, 0, l);
			strncpy(value, e->valuestring, w->prefix_length);
			ARRAYLIST_PUSH(w->words, value);
		}
	}
	cJSON_Delete(raw_word_list);
	free(data);
	return w;
}

void destroy_wordlist(w)
wordlist* w;
{
	char *tmp;
	while (w->words.count) 
	{
		tmp = ARRAYLIST_POP(w->words);
		free(tmp);
	}
	ARRAYLIST_FREE(w->words);
	if (w->lc != LANGUAGE_EN_ELECTRUM) 
	{
		while (w->truncated_words.count)
		{
			tmp = ARRAYLIST_POP(w->truncated_words);
			free(tmp);
		}
	}
	ARRAYLIST_FREE(w->truncated_words);
	w->lc = 0;
	w->prefix_length = 0;
	free(w);
}