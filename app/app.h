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
#include "loki.h"
#include "http.h"

    enum RESULT {
        RESTORE_EXISTING_SEED,
        CREATE_NEW_SEED
    };
    CDKLABEL *title;
    CDKSCREEN *cdkscreen;

    void set_window_title(const char*);

    /* We can serialise this to file if we wanted to export */
    typedef struct {
        ratchet_identity_key_pair *identity_key_pair;
        uint32_t registration_id;
        signal_protocol_key_helper_pre_key_list_node *pre_keys_head;
        session_signed_pre_key *signed_pre_key;
    } signal_user_ctx;

#ifndef _EXPORT_BUILD
    void export_warning();
#endif
    void splash();
    void restore_seed();
    int create_or_restore_seed();

#ifdef __cplusplus
}
#endif

#endif /* APP_H */

