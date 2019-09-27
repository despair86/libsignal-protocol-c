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
  * File:   loki.h
  * Author: despair
  * API for Loki blockchain integration
  *
  * Created on September 16, 2019, 7:58 PM
  */

#ifndef LOKI_H
#define LOKI_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
	uint64_t CURRENT_NET_DIFFICULTY;
#ifdef _WIN32
#include <windows.h>
	CRITICAL_SECTION global_mutex;
#else
#include <pthread.h>
	pthread_mutex_t global_mutex;
#endif

	/* User must manually scrub+free returned buffer */
	unsigned char* calcPoW(int64_t timestamp, int32_t ttl, const unsigned char* pubKey, const unsigned char* data, size_t data_size);

	/* lock/unlock */
	void loki_lock(void*);
	void loki_unlock(void*);

	/* Test utility functions */
	unsigned char* bufferToBase64(void* buf, size_t size);
	void printHex(char* hex, unsigned char* key);
	void print_public_key(const char* prefix, ec_public_key* key);
	void print_buffer(const char* prefix, signal_buffer* buffer);
	void shuffle_buffers(signal_buffer** array, size_t n);
	void shuffle_ec_public_keys(ec_public_key** array, size_t n);
	ec_public_key* create_test_ec_public_key(signal_context* context);
	ec_private_key* create_test_ec_private_key(signal_context* context);

	/* Logging */
	void loki_log(int level, const char* message, size_t len, void* user_data);

	/* Data store context */
	void setup_loki_store_context(signal_protocol_store_context** context, signal_context* global_context);

	/* session store */
	int loki_session_store_load_session(signal_buffer** record, signal_buffer** user_record, const signal_protocol_address* address, void* user_data);
	int loki_session_store_get_sub_device_sessions(signal_int_list** sessions, const char* name, size_t name_len, void* user_data);
	int loki_session_store_store_session(const signal_protocol_address* address, uint8_t* record, size_t record_len, uint8_t* user_record_data, size_t user_record_len, void* user_data);
	int loki_session_store_contains_session(const signal_protocol_address* address, void* user_data);
	int loki_session_store_delete_session(const signal_protocol_address* address, void* user_data);
	int loki_session_store_delete_all_sessions(const char* name, size_t name_len, void* user_data);
	void loki_session_store_destroy(void* user_data);
	void setup_loki_session_store(signal_protocol_store_context* context);

	/* pre-key store */
	int loki_pre_key_store_load_pre_key(signal_buffer** record, uint32_t pre_key_id, void* user_data);
	int loki_pre_key_store_store_pre_key(uint32_t pre_key_id, uint8_t* record, size_t record_len, void* user_data);
	int loki_pre_key_store_contains_pre_key(uint32_t pre_key_id, void* user_data);
	int loki_pre_key_store_remove_pre_key(uint32_t pre_key_id, void* user_data);
	void loki_pre_key_store_destroy(void* user_data);
	void setup_loki_pre_key_store(signal_protocol_store_context* context);

	/* signed pre-key store */
	int loki_signed_pre_key_store_load_signed_pre_key(signal_buffer** record, uint32_t signed_pre_key_id, void* user_data);
	int loki_signed_pre_key_store_store_signed_pre_key(uint32_t signed_pre_key_id, uint8_t* record, size_t record_len, void* user_data);
	int loki_signed_pre_key_store_contains_signed_pre_key(uint32_t signed_pre_key_id, void* user_data);
	int loki_signed_pre_key_store_remove_signed_pre_key(uint32_t signed_pre_key_id, void* user_data);
	void loki_signed_pre_key_store_destroy(void* user_data);
	void setup_loki_signed_pre_key_store(signal_protocol_store_context* context);

	/* identity key store */
	int loki_identity_key_store_get_identity_key_pair(signal_buffer** public_data, signal_buffer** private_data, void* user_data);
	int loki_identity_key_store_get_local_registration_id(void* user_data, uint32_t* registration_id);
	int loki_identity_key_store_save_identity(const signal_protocol_address* address, uint8_t* key_data, size_t key_len, void* user_data);
	int loki_identity_key_store_is_trusted_identity(const signal_protocol_address* address, uint8_t* key_data, size_t key_len, void* user_data);
	void loki_identity_key_store_destroy(void* user_data);
	void setup_loki_identity_key_store(signal_protocol_store_context* context, signal_context* global_context);

	/* sender key store */
	int loki_sender_key_store_store_sender_key(const signal_protocol_sender_key_name* sender_key_name, uint8_t* record, size_t record_len, uint8_t* user_record_data, size_t user_record_len, void* user_data);
	int loki_sender_key_store_load_sender_key(signal_buffer** record, signal_buffer** user_record, const signal_protocol_sender_key_name* sender_key_name, void* user_data);
	void loki_sender_key_store_destroy(void* user_data);
	void setup_loki_sender_key_store(signal_protocol_store_context* context, signal_context* global_context);

	/* Portability */
#ifndef __OpenBSD__
/* OpenBSD extension */
	void srand_deterministic(unsigned int seed);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LOKI_H */

