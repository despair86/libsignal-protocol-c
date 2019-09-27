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
  * File:   app.h
  * Author: despair
  *
  * Created on September 26, 2019, 7:21 PM
  */

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cdk.h>
#include <signal_protocol.h>
#include <key_helper.h>
#include <ratchet.h>
#include "crypto_provider_mbedtls.h"
#include <mbedtls/platform_util.h>
#include "loki.h"
#include "http.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#ifdef _MSC_VER
#include <malloc.h>
#endif
#endif
#ifdef __sun
#include <alloca.h>
#endif

	enum RESULT {
		RESTORE_EXISTING_SEED,
		CREATE_NEW_SEED
	};
	CDKLABEL* title;
	CDKSCREEN* cdkscreen;

	void set_window_title(const char*);

	/* We can serialise this to file if we wanted to export */
	typedef struct {
		ratchet_identity_key_pair* identity_key_pair;
		char pubHex[65];
		char secretHex[65];
		char* pubB64, * secretB64;
		signal_buffer* pub_key;
		signal_buffer* secret_key;
		uint32_t registration_id;
		signal_protocol_key_helper_pre_key_list_node* pre_keys_head;
		session_signed_pre_key* signed_pre_key;
	} loki_user_ctx;

	/* we generate a new identity each time we start, for new users */
	loki_user_ctx* new_user_ctx;

	void splash();
#ifndef _EXPORT_BUILD
	void export_warning();
#endif
	int create_or_restore_seed();
	void restore_seed();
	void new_user();

#ifdef __cplusplus
}
#endif

#endif /* APP_H */

