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
#include "endianness.h"
#include "mnemonic.h"
#include "arraylist.h"
#include <curve.h>
#include <utarray.h>
#include <mbedtls/platform_util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson.h>

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#define bool int
#define true 1
#define false 0
#endif

#if defined(_WIN32) || !defined(__sun)
#ifndef BSD
size_t strlcat(char *dst, const char *src, size_t dsize);
size_t strlcpy(char *dst, const char *src, size_t dsize);
#endif
#endif

#ifdef _MSC_VER
char* strtok_r(char* __restrict s, const char* __restrict delim, char** __restrict last);
#endif


/**/
static get_prefix_length(l)
language_code l;
{
	switch (l)
	{
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
	switch (lc)
	{
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
		if (!value)
		{
			while (w->words.count)
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
			if (!value)
			{
				while (w->words.count)
				{
					value = ARRAYLIST_POP(w->words);
					free(value);
				}
				ARRAYLIST_FREE(w->words);
				while (w->truncated_words.count)
				{
					value = ARRAYLIST_POP(w->truncated_words);
					free(value);
				}
				ARRAYLIST_FREE(w->truncated_words);
				return NULL;
			}
			memset(value, 0, l);
			strncpy(value, e->valuestring, w->prefix_length);
			ARRAYLIST_PUSH(w->truncated_words, value);
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

static void list_push(l, value)
stringList *l;
char *value;
{
	ARRAYLIST_PUSH((*l), value);
}

static strsort(_a, _b)
const void* _a, *_b;
{
	const char *a = *(const char**) _a;
	const char *b = *(const char**) _b;
	return strcmp(a, b);
}

/* Karl Malbrain's compact CRC-32. See "A compact CCITT crc16 and crc32 C
 * implementation that balances processor cache usage against speed": 
 * http://www.geocities.com/malbrain/
 * to use this one, define USE_TINY_CRC32 in CPPFLAGS or CMAKE_C_FLAGS
 */
#ifdef USE_TINY_CRC32
static unsigned long crc32(crc, ptr, buf_len)
unsigned long crc;
const uint8_t* ptr;
size_t buf_len;
{
	static const uint32_t s_crc32[16] = {0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
		0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};
	uint32_t crcu32 = (uint32_t) crc;
	if (!ptr)
		return 0;
	crcu32 = ~crcu32;
	while (buf_len--)
	{
		uint8_t b = *ptr++;
		crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)];
		crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)];
	}
	return ~crcu32;
}
#else
/* Faster, but larger CPU cache footprint.
 */
static unsigned long crc32(crc, ptr, buf_len)
unsigned long crc;
const uint8_t* ptr;
size_t buf_len;
{
	static const uint32_t s_crc_table[256] = {
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535,
		0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD,
		0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D,
		0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
		0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4,
		0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
		0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC,
		0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
		0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB,
		0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F,
		0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB,
		0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
		0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA,
		0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE,
		0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A,
		0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409,
		0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
		0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739,
		0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
		0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268,
		0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0,
		0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8,
		0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
		0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF,
		0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703,
		0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7,
		0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
		0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE,
		0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
		0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6,
		0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D,
		0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5,
		0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605,
		0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
		0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	};

	uint32_t crc32 = (uint32_t) crc ^ 0xFFFFFFFF;
	const uint8_t *pByte_buf = (const uint8_t *) ptr;

	while (buf_len >= 4)
	{
		crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[0]) & 0xFF];
		crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[1]) & 0xFF];
		crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[2]) & 0xFF];
		crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[3]) & 0xFF];
		pByte_buf += 4;
		buf_len -= 4;
	}

	while (buf_len)
	{
		crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[0]) & 0xFF];
		++pByte_buf;
		--buf_len;
	}

	return ~crc32;
}
#endif

uint32_t create_checksum_index(list, word_list)
const stringList* list;
wordlist* word_list;
{
	char *tmp, *word, **p;
	stringList tmp_list;
	UT_array *trimmed_wordlist;
	unsigned long crc;
	int pl, i;

	/* get our unique prefix length */
	pl = word_list->prefix_length;
	memset(&tmp_list, 0, sizeof(stringList));
	
	if (!pl)
		goto encode;
	
	/* Load truncated wordlist from master wordlist and place into searchable 
	 * array
	 */
	utarray_new(trimmed_wordlist, &ut_str_icd);
	for (i = 0; i < word_list->truncated_words.count; i++)
	{
		/* makes a copy of the string */
		tmp = ARRAYLIST_GET(word_list->truncated_words, i);
		utarray_push_back(trimmed_wordlist, &tmp);
	}

	/* We only need to sort this to do the word check. */
	utarray_sort(trimmed_wordlist, strsort);

	/* truncate the mnemonic words to the prefix length 
	 * then add to list on stack 
	 */
encode:
	for (i = 0; i < list->count; i++)
	{
		word = ARRAYLIST_GET((*list), i);
		if (word[0] == ' ')
		{
			ARRAYLIST_PUSH(tmp_list, word);
			continue;
		}
		if (!pl)
			pl = strlen(word);
		tmp = malloc(pl + 1);
		snprintf(tmp, pl, "%s", word);
		tmp[pl] = 0;
		ARRAYLIST_PUSH(tmp_list, tmp);
		/* clear prefix length if electrum so we can get 
		 * the full string on the next iteration
		 */
		if (word_list->lc == LANGUAGE_EN_ELECTRUM)
			pl = 0;
	}

	if (word_list->lc == LANGUAGE_EN_ELECTRUM)
		pl = 0;

	/* make sure the mnemonic words are actually valid */
	if (pl)
	{
		for (i = 0; i < tmp_list.count; i++)
		{
			tmp = ARRAYLIST_GET(tmp_list, i);
			/* skip over spaces */
			if (tmp[0] == ' ')
				continue;
			p = utarray_find(trimmed_wordlist, &tmp, strsort);
			if (!p)
			{
				printf("word %s not found in trimmed word map: language code: %d\n", tmp, word_list->lc);
				goto cleanup;
			}
		}
	}

	/* render the trimmed mnemonic key as a string */
	tmp = malloc(8192); // *crosses fingers*
	if (!tmp)
		fatal_error("out of memory!");

	for (i = 0; i < tmp_list.count; i++)
	{
		word = ARRAYLIST_GET(tmp_list, i);
		strlcat(tmp, word, 8192);
	}
	
	/* get the CRC-32 of our string */
	crc = crc32(0L, tmp, strlen(tmp));
	
	/* clean up */
	free(tmp);
	
cleanup:
	while (tmp_list.count)
	{
		tmp = ARRAYLIST_POP(tmp_list);
		/* scrub the truncated mnemonics, these are secrets! */
		mbedtls_platform_zeroize(tmp, strlen(tmp)+1);
		free(tmp);
	}
	ARRAYLIST_FREE(tmp_list);
	/* we can dump the trimmed wordlist now */
	if (pl)
		utarray_free(trimmed_wordlist);

	return crc % word_list->words.count;
}

/* returns a string with the encoded key 
 * caller must scrub the mnemonic after use
 */
char* mnemonic_encode(secret_key, word_list)
uint8_t* secret_key;
wordlist* word_list;
{
	size_t n, i, str_len;
	char *tmp, *word;
	stringList *result;

	str_len = strlen(secret_key);

	if (str_len % 4 != 0 || str_len == 0)
		return NULL;
	if (!word_list)
		return NULL;

	n = word_list->words.count;
	result = malloc(sizeof (wordlist));
	
	if (!result)
		return NULL;
	
	memset(result, 0, sizeof (wordlist));
	// 4 bytes -> 3 words.  8 digits base 16 -> 3 digits base 1626
	for (i = 0; i < str_len / 4; i++, list_push(result, " "))
	{
		uint32_t w[4];

		w[0] = SWAP32LE(*(const uint32_t*) (secret_key + (i * 4)));

		w[1] = w[0] % n;
		w[2] = ((w[0] / n) + w[1]) % n;
		w[3] = (((w[0] / n) / n) + w[2]) % n;

		/* words += n[w[1]]; */
		tmp = ARRAYLIST_GET(word_list->words, w[1]);
		ARRAYLIST_PUSH((*result), strdup(tmp));
		list_push(result, " ");
		/* words += n[w[2]]; */
		tmp = ARRAYLIST_GET(word_list->words, w[2]);
		ARRAYLIST_PUSH((*result), strdup(tmp));
		list_push(result, " ");
		/* words += n[w[3]]; */
		tmp = ARRAYLIST_GET(word_list->words, w[3]);
		ARRAYLIST_PUSH((*result), strdup(tmp));

		mbedtls_platform_zeroize(w, sizeof (w));
	}
	/* add checksum word to the end of the word_list */
	tmp = ARRAYLIST_GET(word_list->words, create_checksum_index(result, word_list));
	ARRAYLIST_PUSH((*result), strdup(tmp));
	
	/* render the mnemonic key as a string */
	tmp = malloc(8192); // *crosses fingers*
	for (i = 0; i < result->count; i++)
	{
		word = ARRAYLIST_GET((*result), i);
		strlcat(tmp, word, 8192);
	}
	
	/* clean up temp array */
	while (result->count)
	{
		word = ARRAYLIST_POP((*result));
		/* the string pointing to spaces isn't on 
		 * the heap and cannot be touched 
		 */
		if (word[0] != ' ')
		{
			mbedtls_platform_zeroize(word, strlen(word) + 1);
			free(word);
		}
	}
	ARRAYLIST_FREE((*result));
	free(result);
	return tmp;
}
