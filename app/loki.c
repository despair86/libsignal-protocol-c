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
 * Proof-of-work implementation, see
 * loki-messenger:clearnet/libloki/proof-of-work.js for reference.
 */

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#include <signal_protocol.h>
#include <curve.h>
#include <uthash.h>
#include "loki.h"
#include <mbedtls/base64.h>
#include <mbedtls/base64.h>
#include <mbedtls/sha512.h>
#include <mbedtls/platform_util.h>

#define NONCE_SIZE 8

uint64_t CURRENT_NET_DIFFICULTY = 10;

/* User must free() returned buffer when finished */
unsigned char* bufferToBase64(buf, size)
void* buf;
size_t size;
{
	unsigned char* tmp;
	size_t out_size;
	int r;

	/* Get the size of the output */
	tmp = NULL;
	mbedtls_base64_encode(tmp, 0, &out_size, (unsigned char*)buf, size);

	tmp = malloc(out_size);
	assert(tmp);
	r = mbedtls_base64_encode(tmp, out_size, &out_size, (unsigned char*)buf, size);
	if (r)
	{
		fprintf(stderr, "Failed to encode binary data\n");
		/* May contain nuts, crush them before returning  */
		mbedtls_platform_zeroize(tmp, out_size);
		free(tmp);
		return NULL;
	}
	return tmp;
}

static uint64_t getDiffTgt(ttl, payload_size)
int32_t ttl;
size_t payload_size;
{
	uint64_t x1, x2, x3, x4, x5;
	size_t sz;
	int32_t ttlInSecs;

	x1 = (1ULL << 16) - 1;
	x2 = UINT64_MAX - 1;
	sz = payload_size + NONCE_SIZE;
	ttlInSecs = ttl / 1000;
	x3 = ((uint64_t)ttlInSecs * sz) / x1;
	x4 = sz + x3;
	/* CURRENT_NET_DIFFICULTY is a global that is updated every time
	 * a message is sent to a DM channel. Initially 10
	 */
	x5 = CURRENT_NET_DIFFICULTY * x4;
	return x2 / x5;
}

/* The nice thing about writing out function definitions K&R style
 * is that prototypes can be made to fit within 80 columns.
 * Case in point: this function ran off the edge of the screen in ASA style
 * Now it doesn't.
 */

 /* Proof-of-work algorithm (not used yet) */
unsigned char* calcPoW(timestamp, ttl, pubKey, data, data_size)
int64_t timestamp;
int32_t ttl;
const unsigned char* pubKey, * data;
size_t data_size;
{
#ifndef _MSC_VER
	unsigned char payload[data_size + 16384];
#else
	unsigned char* payload;
#endif
	uint64_t diffTgt, ctr;
	int64_t nonce;
	unsigned char initial_hash[64], hash[64], tmp[NONCE_SIZE + 64];

#ifdef _MSC_VER
	payload = alloca(data_size + 16384);
#endif
	/* scrub everything before we start, some platforms flip out on writes to uninitialised buffers */
	mbedtls_platform_zeroize(payload, data_size + 16384);
	snprintf((char*)payload, data_size + 16384, "%" PRIi64 "%d%s%s", timestamp, ttl, pubKey, data);
	diffTgt = getDiffTgt(ttl, strlen((char*)payload));
	ctr = UINT64_MAX;
	nonce = 0;
	mbedtls_sha512_ret(payload, strlen((char*)payload), initial_hash, 0);
	do
	{
		nonce += 1;
		memcpy(tmp, &nonce, NONCE_SIZE);
		memcpy(tmp + NONCE_SIZE, initial_hash, 64);
		mbedtls_sha512_ret(tmp, NONCE_SIZE + 64, hash, 0);
		memcpy(&ctr, hash, NONCE_SIZE);
	} while (ctr > diffTgt);
	mbedtls_platform_zeroize(payload, data_size + 16384);
	return bufferToBase64(&nonce, NONCE_SIZE);
}

/* Hex representation of a given key of 33 bytes */
void printPubHex(hex, key)
char* hex;
unsigned char* key;
{
	int i;

	for (i = 0; i < 33; ++i)
		sprintf(&hex[i * 2], "%02x", key[i]);
	hex[65] = 0;
}

void printSecretHex(hex, key)
char* hex;
unsigned char* key;
{
	int i;

	for (i = 0; i < 32; ++i)
		sprintf(&hex[i * 2], "%02x", key[i]);
	hex[64] = 0;
}

#ifndef _WIN32
pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
void loki_lock(user_data)
void* user_data;
{
#ifndef _WIN32
	pthread_mutex_lock(&global_mutex);
#else
	EnterCriticalSection(&global_mutex);
#endif
}

void loki_unlock(user_data)
void* user_data;
{
#ifndef _WIN32
	pthread_mutex_unlock(&global_mutex);
#else
	LeaveCriticalSection(&global_mutex);
#endif
}

/*
 * This is an implementation of Jenkin's "One-at-a-Time" hash.
 *
 * http://www.burtleburtle.net/bob/hash/doobs.html
 *
 * It is used to simplify using our new string recipient IDs
 * as part of our keys without having to significantly modify the
 * testing-only implementations of our data stores.
 */
int64_t jenkins_hash(key, len)
const char* key;
size_t len;
{
	uint64_t hash, i;
	for (hash = i = 0; i < len; ++i) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

void print_buffer(prefix, buffer)
const char* prefix;
signal_buffer* buffer;
{
	uint8_t* data;
	int len, i;

	fprintf(stderr, "%s ", prefix);
	data = signal_buffer_data(buffer);
	len = signal_buffer_len(buffer);
	for (i = 0; i < len; i++) {
		if (i > 0 && (i % 40) == 0) {
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "%02X", data[i]);
	}
	fprintf(stderr, "\n");
}

ec_public_key * create_test_ec_public_key(context)
signal_context * context;
{
	ec_public_key* public_key;
	ec_key_pair* key_pair;

	curve_generate_key_pair(context, &key_pair);

	public_key = ec_key_pair_get_public(key_pair);
	SIGNAL_REF(public_key);
	SIGNAL_UNREF(key_pair);
	return public_key;
}

ec_private_key* create_test_ec_private_key(context)
signal_context* context;
{
	ec_key_pair* key_pair;
	ec_private_key* private_key;

	curve_generate_key_pair(context, &key_pair);

	private_key = ec_key_pair_get_private(key_pair);
	SIGNAL_REF(private_key);
	SIGNAL_UNREF(key_pair);
	return private_key;
}

void loki_log(level, message, len, user_data)
const char* message;
size_t len;
void* user_data;
{
	switch (level) {
	case SG_LOG_ERROR:
		fprintf(stderr, "[ERROR] %s\n", message);
		break;
	case SG_LOG_WARNING:
		fprintf(stderr, "[WARNING] %s\n", message);
		break;
	case SG_LOG_NOTICE:
		fprintf(stderr, "[NOTICE] %s\n", message);
		break;
	case SG_LOG_INFO:
		fprintf(stderr, "[INFO] %s\n", message);
		break;
	case SG_LOG_DEBUG:
		fprintf(stderr, "[DEBUG] %s\n", message);
		break;
	default:
		fprintf(stderr, "[%d] %s\n", level, message);
		break;
	}
}

void setup_loki_store_context(context, global_context)
signal_protocol_store_context** context;
signal_context* global_context;
{
	signal_protocol_store_context* store_context = 0;
	signal_protocol_store_context_create(&store_context, global_context);

	setup_loki_session_store(store_context);
	setup_loki_pre_key_store(store_context);
	setup_loki_signed_pre_key_store(store_context);
	setup_loki_identity_key_store(store_context, global_context);
	setup_loki_sender_key_store(store_context, global_context);

	*context = store_context;
}

/*------------------------------------------------------------------------*/

typedef struct {
	int64_t recipient_id;
	int32_t device_id;
} loki_session_store_session_key;

typedef struct {
	loki_session_store_session_key key;
	signal_buffer* record;
	UT_hash_handle hh;
} loki_session_store_session;

typedef struct {
	loki_session_store_session* sessions;
} loki_session_store_data;

loki_session_store_load_session(record, user_record, address, user_data)
signal_buffer** record, ** user_record;
const signal_protocol_address* address;
void* user_data;
{
	loki_session_store_data* data = user_data;
	loki_session_store_session* s;
	loki_session_store_session l;
	signal_buffer* result;

	memset(&l, 0, sizeof(loki_session_store_session));
	l.key.recipient_id = jenkins_hash(address->name, address->name_len);
	l.key.device_id = address->device_id;
	HASH_FIND(hh, data->sessions, &l.key, sizeof(loki_session_store_session_key), s);

	if (!s) {
		return 0;
	}
	result = signal_buffer_copy(s->record);
	if (!result) {
		return SG_ERR_NOMEM;
	}
	*record = result;
	return 1;
}

loki_session_store_get_sub_device_sessions(sessions, name, name_len, user_data)
signal_int_list** sessions;
const char* name;
size_t name_len;
void* user_data;
{
	loki_session_store_data* data = user_data;
	loki_session_store_session* cur_node;
	loki_session_store_session* tmp_node;
	int64_t recipient_hash;

	signal_int_list* result = signal_int_list_alloc();
	if (!result) {
		return SG_ERR_NOMEM;
	}

	recipient_hash = jenkins_hash(name, name_len);
	HASH_ITER(hh, data->sessions, cur_node, tmp_node) {
		if (cur_node->key.recipient_id == recipient_hash) {
			signal_int_list_push_back(result, cur_node->key.device_id);
		}
	}

	*sessions = result;
	return 0;
}

loki_session_store_store_session(address, record, record_len, user_record_data, user_record_len, user_data)
const signal_protocol_address* address;
uint8_t* record, * user_record_data;
size_t record_len, user_record_len;
void* user_data;
{
	loki_session_store_data* data = user_data;
	loki_session_store_session* s;
	loki_session_store_session l;
	signal_buffer* record_buf;

	memset(&l, 0, sizeof(loki_session_store_session));
	l.key.recipient_id = jenkins_hash(address->name, address->name_len);
	l.key.device_id = address->device_id;

	record_buf = signal_buffer_create(record, record_len);
	if (!record_buf) {
		return SG_ERR_NOMEM;
	}

	HASH_FIND(hh, data->sessions, &l.key, sizeof(loki_session_store_session_key), s);

	if (s) {
		signal_buffer_free(s->record);
		s->record = record_buf;
	}
	else {
		s = malloc(sizeof(loki_session_store_session));
		if (!s) {
			signal_buffer_free(record_buf);
			return SG_ERR_NOMEM;
		}
		memset(s, 0, sizeof(loki_session_store_session));
		s->key.recipient_id = jenkins_hash(address->name, address->name_len);
		s->key.device_id = address->device_id;
		s->record = record_buf;
		HASH_ADD(hh, data->sessions, key, sizeof(loki_session_store_session_key), s);
	}

	return 0;
}

loki_session_store_contains_session(address, user_data)
const signal_protocol_address* address;
void* user_data;
{
	loki_session_store_data* data = user_data;
	loki_session_store_session* s;
	loki_session_store_session l;

	memset(&l, 0, sizeof(loki_session_store_session));
	l.key.recipient_id = jenkins_hash(address->name, address->name_len);
	l.key.device_id = address->device_id;

	HASH_FIND(hh, data->sessions, &l.key, sizeof(loki_session_store_session_key), s);

	return (s == 0) ? 0 : 1;
}

loki_session_store_delete_session(address, user_data)
const signal_protocol_address* address;
void* user_data;
{
	int result = 0;
	loki_session_store_data* data = user_data;
	loki_session_store_session* s;
	loki_session_store_session l;

	memset(&l, 0, sizeof(loki_session_store_session));
	l.key.recipient_id = jenkins_hash(address->name, address->name_len);
	l.key.device_id = address->device_id;

	HASH_FIND(hh, data->sessions, &l.key, sizeof(loki_session_store_session_key), s);

	if (s) {
		HASH_DEL(data->sessions, s);
		signal_buffer_free(s->record);
		free(s);
		result = 1;
	}
	return result;
}

loki_session_store_delete_all_sessions(name, name_len, user_data)
const char* name;
size_t name_len;
void* user_data;
{
	int result = 0;
	loki_session_store_data* data = user_data;
	loki_session_store_session* cur_node;
	loki_session_store_session* tmp_node;
	int64_t recipient_hash;

	recipient_hash = jenkins_hash(name, name_len);
	HASH_ITER(hh, data->sessions, cur_node, tmp_node) {
		if (cur_node->key.recipient_id == recipient_hash) {
			HASH_DEL(data->sessions, cur_node);
			signal_buffer_free(cur_node->record);
			free(cur_node);
			result++;
		}
	}

	return result;
}

void loki_session_store_destroy(user_data)
void* user_data;
{
	loki_session_store_data* data = user_data;
	loki_session_store_session* cur_node;
	loki_session_store_session* tmp_node;

	HASH_ITER(hh, data->sessions, cur_node, tmp_node) {
		HASH_DEL(data->sessions, cur_node);
		signal_buffer_free(cur_node->record);
		free(cur_node);
	}

	free(data);
}

void setup_loki_session_store(context)
signal_protocol_store_context* context;
{
	loki_session_store_data* data = malloc(sizeof(loki_session_store_data));

	signal_protocol_session_store store = {
		loki_session_store_load_session,
		loki_session_store_get_sub_device_sessions,
		loki_session_store_store_session,
		loki_session_store_contains_session,
		loki_session_store_delete_session,
		loki_session_store_delete_all_sessions,
		loki_session_store_destroy,
		data
	};

	memset(data, 0, sizeof(loki_session_store_data));
	signal_protocol_store_context_set_session_store(context, &store);
}

/*------------------------------------------------------------------------*/

typedef struct {
	uint32_t key_id;
	signal_buffer* key_record;
	UT_hash_handle hh;
} loki_pre_key_store_key;

typedef struct {
	loki_pre_key_store_key* keys;
} loki_pre_key_store_data;

loki_pre_key_store_load_pre_key(record, pre_key_id, user_data)
signal_buffer** record;
uint32_t pre_key_id;
void* user_data;
{
	loki_pre_key_store_data* data = user_data;
	loki_pre_key_store_key* s;

	HASH_FIND(hh, data->keys, &pre_key_id, sizeof(uint32_t), s);
	if (s) {
		*record = signal_buffer_copy(s->key_record);
		return SG_SUCCESS;
	}
	else {
		return SG_ERR_INVALID_KEY_ID;
	}
}

loki_pre_key_store_store_pre_key(pre_key_id, record, record_len, user_data)
uint32_t pre_key_id;
uint8_t* record;
size_t record_len;
void* user_data;
{
	loki_pre_key_store_data* data = user_data;

	loki_pre_key_store_key* s;

	signal_buffer* key_buf = signal_buffer_create(record, record_len);
	if (!key_buf) {
		return SG_ERR_NOMEM;
	}

	HASH_FIND(hh, data->keys, &pre_key_id, sizeof(uint32_t), s);
	if (s) {
		signal_buffer_free(s->key_record);
		s->key_record = key_buf;
	}
	else {
		s = malloc(sizeof(loki_pre_key_store_key));
		if (!s) {
			signal_buffer_free(key_buf);
			return SG_ERR_NOMEM;
		}
		memset(s, 0, sizeof(loki_pre_key_store_key));
		s->key_id = pre_key_id;
		s->key_record = key_buf;
		HASH_ADD(hh, data->keys, key_id, sizeof(uint32_t), s);
	}

	return 0;
}

loki_pre_key_store_contains_pre_key(pre_key_id, user_data)
uint32_t pre_key_id;
void* user_data;
{
	loki_pre_key_store_data* data = user_data;

	loki_pre_key_store_key* s;
	HASH_FIND(hh, data->keys, &pre_key_id, sizeof(uint32_t), s);

	return (s == 0) ? 0 : 1;
}

loki_pre_key_store_remove_pre_key(pre_key_id, user_data)
uint32_t pre_key_id;
void* user_data;
{
	loki_pre_key_store_data* data = user_data;

	loki_pre_key_store_key* s;
	HASH_FIND(hh, data->keys, &pre_key_id, sizeof(uint32_t), s);
	if (s) {
		HASH_DEL(data->keys, s);
		signal_buffer_free(s->key_record);
		free(s);
	}

	return 0;
}

void loki_pre_key_store_destroy(user_data)
void* user_data;
{
	loki_pre_key_store_data* data = user_data;

	loki_pre_key_store_key* cur_node;
	loki_pre_key_store_key* tmp_node;
	HASH_ITER(hh, data->keys, cur_node, tmp_node) {
		HASH_DEL(data->keys, cur_node);
		signal_buffer_free(cur_node->key_record);
		free(cur_node);
	}
	free(data);
}

void setup_loki_pre_key_store(context)
signal_protocol_store_context* context;
{
	loki_pre_key_store_data* data = malloc(sizeof(loki_pre_key_store_data));

	signal_protocol_pre_key_store store = {
		loki_pre_key_store_load_pre_key,
		loki_pre_key_store_store_pre_key,
		loki_pre_key_store_contains_pre_key,
		loki_pre_key_store_remove_pre_key,
		loki_pre_key_store_destroy,
		data
	};

	memset(data, 0, sizeof(loki_pre_key_store_data));
	signal_protocol_store_context_set_pre_key_store(context, &store);
}

/*------------------------------------------------------------------------*/

typedef struct {
	uint32_t key_id;
	signal_buffer* key_record;
	UT_hash_handle hh;
} loki_signed_pre_key_store_key;

typedef struct {
	loki_signed_pre_key_store_key* keys;
} loki_signed_pre_key_store_data;


loki_signed_pre_key_store_load_signed_pre_key(record, signed_pre_key_id, user_data)
signal_buffer** record;
uint32_t signed_pre_key_id;
void* user_data;
{
	loki_signed_pre_key_store_data* data = user_data;
	loki_signed_pre_key_store_key* s;

	HASH_FIND(hh, data->keys, &signed_pre_key_id, sizeof(uint32_t), s);
	if (s) {
		*record = signal_buffer_copy(s->key_record);
		return SG_SUCCESS;
	}
	else {
		return SG_ERR_INVALID_KEY_ID;
	}
}

loki_signed_pre_key_store_store_signed_pre_key(signed_pre_key_id, record, record_len, user_data)
uint32_t signed_pre_key_id;
uint8_t* record;
size_t record_len;
void* user_data;
{
	loki_signed_pre_key_store_data* data = user_data;
	loki_signed_pre_key_store_key* s;

	signal_buffer* key_buf = signal_buffer_create(record, record_len);
	if (!key_buf) {
		return SG_ERR_NOMEM;
	}

	HASH_FIND(hh, data->keys, &signed_pre_key_id, sizeof(uint32_t), s);
	if (s) {
		signal_buffer_free(s->key_record);
		s->key_record = key_buf;
	}
	else {
		s = malloc(sizeof(loki_signed_pre_key_store_key));
		if (!s) {
			signal_buffer_free(key_buf);
			return SG_ERR_NOMEM;
		}
		memset(s, 0, sizeof(loki_signed_pre_key_store_key));
		s->key_id = signed_pre_key_id;
		s->key_record = key_buf;
		HASH_ADD(hh, data->keys, key_id, sizeof(uint32_t), s);
	}

	return 0;
}

loki_signed_pre_key_store_contains_signed_pre_key(signed_pre_key_id, user_data)
uint32_t signed_pre_key_id;
void* user_data;
{
	loki_signed_pre_key_store_data* data = user_data;

	loki_signed_pre_key_store_key* s;
	HASH_FIND(hh, data->keys, &signed_pre_key_id, sizeof(uint32_t), s);

	return (s == 0) ? 0 : 1;
}

loki_signed_pre_key_store_remove_signed_pre_key(signed_pre_key_id, user_data)
uint32_t signed_pre_key_id;
void* user_data;
{
	loki_signed_pre_key_store_data* data = user_data;

	loki_signed_pre_key_store_key* s;
	HASH_FIND(hh, data->keys, &signed_pre_key_id, sizeof(uint32_t), s);
	if (s) {
		HASH_DEL(data->keys, s);
		signal_buffer_free(s->key_record);
		free(s);
	}

	return 0;
}

void loki_signed_pre_key_store_destroy(user_data)
void* user_data;
{
	loki_signed_pre_key_store_data* data = user_data;

	loki_signed_pre_key_store_key* cur_node;
	loki_signed_pre_key_store_key* tmp_node;
	HASH_ITER(hh, data->keys, cur_node, tmp_node) {
		HASH_DEL(data->keys, cur_node);
		signal_buffer_free(cur_node->key_record);
		free(cur_node);
	}
	free(data);
}

void setup_loki_signed_pre_key_store(context)
signal_protocol_store_context* context;
{
	loki_signed_pre_key_store_data* data = malloc(sizeof(loki_signed_pre_key_store_data));

	signal_protocol_signed_pre_key_store store = {
			loki_signed_pre_key_store_load_signed_pre_key,
			loki_signed_pre_key_store_store_signed_pre_key,
			loki_signed_pre_key_store_contains_signed_pre_key,
			loki_signed_pre_key_store_remove_signed_pre_key,
			loki_signed_pre_key_store_destroy,
			data
	};

	memset(data, 0, sizeof(loki_signed_pre_key_store_data));
	signal_protocol_store_context_set_signed_pre_key_store(context, &store);
}

/*------------------------------------------------------------------------*/

typedef struct {
	int64_t recipient_id;
	signal_buffer* identity_key;
	UT_hash_handle hh;
} loki_identity_store_key;

typedef struct {
	loki_identity_store_key* keys;
	signal_buffer* identity_key_public;
	signal_buffer* identity_key_private;
	uint32_t local_registration_id;
} loki_identity_store_data;

loki_identity_key_store_get_identity_key_pair(public_data, private_data, user_data)
signal_buffer** public_data, ** private_data;
void* user_data;
{
	loki_identity_store_data* data = user_data;
	*public_data = signal_buffer_copy(data->identity_key_public);
	*private_data = signal_buffer_copy(data->identity_key_private);
	return 0;
}

loki_identity_key_store_get_local_registration_id(user_data, registration_id)
void* user_data;
uint32_t* registration_id;
{
	loki_identity_store_data* data = user_data;
	*registration_id = data->local_registration_id;
	return 0;
}

loki_identity_key_store_save_identity(address, key_data, key_len, user_data)
const signal_protocol_address* address;
uint8_t* key_data;
size_t key_len;
void* user_data;
{
	loki_identity_store_data* data = user_data;
	loki_identity_store_key* s;
	signal_buffer* key_buf = signal_buffer_create(key_data, key_len);
	int64_t recipient_hash;

	if (!key_buf) {
		return SG_ERR_NOMEM;
	}

	recipient_hash = jenkins_hash(address->name, address->name_len);

	HASH_FIND(hh, data->keys, &recipient_hash, sizeof(int64_t), s);
	if (s) {
		signal_buffer_free(s->identity_key);
		s->identity_key = key_buf;
	}
	else {
		s = malloc(sizeof(loki_identity_store_key));
		if (!s) {
			signal_buffer_free(key_buf);
			return SG_ERR_NOMEM;
		}
		memset(s, 0, sizeof(loki_identity_store_key));
		s->recipient_id = recipient_hash;
		s->identity_key = key_buf;
		HASH_ADD(hh, data->keys, recipient_id, sizeof(int64_t), s);
	}

	return 0;
}

loki_identity_key_store_is_trusted_identity(address, key_data, key_len, user_data)
const signal_protocol_address* address;
uint8_t* key_data;
size_t key_len;
void* user_data;
{
	loki_identity_store_data* data = user_data;

	int64_t recipient_hash = jenkins_hash(address->name, address->name_len);

	loki_identity_store_key* s;
	HASH_FIND(hh, data->keys, &recipient_hash, sizeof(int64_t), s);

	if (s) {
		uint8_t* store_data = signal_buffer_data(s->identity_key);
		size_t store_len = signal_buffer_len(s->identity_key);
		if (store_len != key_len) {
			return 0;
		}
		if (memcmp(key_data, store_data, key_len) == 0) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 1;
	}
}

void loki_identity_key_store_destroy(user_data)
void* user_data;
{
	loki_identity_store_data* data = user_data;

	loki_identity_store_key* cur_node;
	loki_identity_store_key* tmp_node;
	HASH_ITER(hh, data->keys, cur_node, tmp_node) {
		HASH_DEL(data->keys, cur_node);
		signal_buffer_free(cur_node->identity_key);
		free(cur_node);
	}
	signal_buffer_free(data->identity_key_public);
	signal_buffer_free(data->identity_key_private);
	free(data);
}

void setup_loki_identity_key_store(context, global_context)
signal_protocol_store_context* context;
signal_context* global_context;
{
	loki_identity_store_data* data = malloc(sizeof(loki_identity_store_data));
	ec_key_pair* identity_key_pair_keys = 0;
	ec_public_key* identity_key_public;
	ec_private_key* identity_key_private;

	signal_protocol_identity_key_store store = {
			loki_identity_key_store_get_identity_key_pair,
			loki_identity_key_store_get_local_registration_id,
			loki_identity_key_store_save_identity,
			loki_identity_key_store_is_trusted_identity,
			loki_identity_key_store_destroy,
			data
	};

	memset(data, 0, sizeof(loki_identity_store_data));
	curve_generate_key_pair(global_context, &identity_key_pair_keys);
	identity_key_public = ec_key_pair_get_public(identity_key_pair_keys);
	identity_key_private = ec_key_pair_get_private(identity_key_pair_keys);

	ec_public_key_serialize(&data->identity_key_public, identity_key_public);
	ec_private_key_serialize(&data->identity_key_private, identity_key_private);
	SIGNAL_UNREF(identity_key_pair_keys);

	data->local_registration_id = (rand() % 16380) + 1;

	signal_protocol_store_context_set_identity_key_store(context, &store);
}

/*------------------------------------------------------------------------*/

typedef struct {
	int64_t group_id;
	int64_t recipient_id;
	int32_t device_id;
} loki_sender_key_store_key;

typedef struct {
	loki_sender_key_store_key key;
	signal_buffer* record;
	UT_hash_handle hh;
} loki_sender_key_store_record;

typedef struct {
	loki_sender_key_store_record* records;
} loki_sender_key_store_data;

loki_sender_key_store_store_sender_key(sender_key_name, record, record_len, user_record_data, user_record_len, user_data)
const signal_protocol_sender_key_name* sender_key_name;
uint8_t* record, * user_record_data;
size_t record_len, user_record_len;
void* user_data;
{
	loki_sender_key_store_data* data = user_data;
	loki_sender_key_store_record* s;
	loki_sender_key_store_record l;
	signal_buffer* record_buf;

	memset(&l, 0, sizeof(loki_sender_key_store_record));
	l.key.group_id = jenkins_hash(sender_key_name->group_id, sender_key_name->group_id_len);
	l.key.recipient_id = jenkins_hash(sender_key_name->sender.name, sender_key_name->sender.name_len);
	l.key.device_id = sender_key_name->sender.device_id;

	record_buf = signal_buffer_create(record, record_len);
	if (!record_buf) {
		return SG_ERR_NOMEM;
	}

	HASH_FIND(hh, data->records, &l.key, sizeof(loki_sender_key_store_key), s);

	if (s) {
		signal_buffer_free(s->record);
		s->record = record_buf;
	}
	else {
		s = malloc(sizeof(loki_sender_key_store_record));
		if (!s) {
			signal_buffer_free(record_buf);
			return SG_ERR_NOMEM;
		}
		memset(s, 0, sizeof(loki_sender_key_store_record));
		s->key.group_id = jenkins_hash(sender_key_name->group_id, sender_key_name->group_id_len);
		s->key.recipient_id = jenkins_hash(sender_key_name->sender.name, sender_key_name->sender.name_len);
		s->key.device_id = sender_key_name->sender.device_id;
		s->record = record_buf;
		HASH_ADD(hh, data->records, key, sizeof(loki_sender_key_store_key), s);
	}

	return 0;
}

loki_sender_key_store_load_sender_key(record, user_record, sender_key_name, user_data)
signal_buffer** record, ** user_record;
const signal_protocol_sender_key_name* sender_key_name;
void* user_data;
{
	loki_sender_key_store_data* data = user_data;
	loki_sender_key_store_record* s;
	loki_sender_key_store_record l;
	signal_buffer* result;

	memset(&l, 0, sizeof(loki_sender_key_store_record));
	l.key.group_id = jenkins_hash(sender_key_name->group_id, sender_key_name->group_id_len);
	l.key.recipient_id = jenkins_hash(sender_key_name->sender.name, sender_key_name->sender.name_len);
	l.key.device_id = sender_key_name->sender.device_id;
	HASH_FIND(hh, data->records, &l.key, sizeof(loki_sender_key_store_key), s);

	if (!s) {
		return 0;
	}
	result = signal_buffer_copy(s->record);
	if (!result) {
		return SG_ERR_NOMEM;
	}
	*record = result;
	return 1;
}

void loki_sender_key_store_destroy(user_data)
void* user_data;
{
	loki_sender_key_store_data* data = user_data;
	loki_sender_key_store_record* cur_node;
	loki_sender_key_store_record* tmp_node;

	HASH_ITER(hh, data->records, cur_node, tmp_node) {
		HASH_DEL(data->records, cur_node);
		signal_buffer_free(cur_node->record);
		free(cur_node);
	}
	free(data);
}

void setup_loki_sender_key_store(context, global_context)
signal_protocol_store_context* context;
signal_context* global_context;
{
	loki_sender_key_store_data* data = malloc(sizeof(loki_sender_key_store_data));

	signal_protocol_sender_key_store store = {
		loki_sender_key_store_store_sender_key,
		loki_sender_key_store_load_sender_key,
		loki_sender_key_store_destroy,
		data
	};

	memset(data, 0, sizeof(loki_sender_key_store_data));
	signal_protocol_store_context_set_sender_key_store(context, &store);
}
