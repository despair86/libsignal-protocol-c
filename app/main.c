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

/* Entry point for Loki Pager text-mode client for Loki communications network */
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

#include "app.h"

/* Global variables and fixed app-specific data */
char *XCursesProgramName = "Loki Pager";

static bool http_start = false;
static signal_context *loki_signal_ctx;
extern signal_crypto_provider mbedtls_signal_csp;
/* we generate a new identity each time we start, for new users */
signal_user_ctx *new_user_ctx;

static bool boot_signal()
{
    int r;
#ifdef _WIN32
    __time64_t timestamp;
    InitializeCriticalSection(&global_mutex);
    _time64(&timestamp);
#else
    time_t timestamp;
    time(&timestamp);
#endif
    r = signal_context_create(&loki_signal_ctx, NULL);
    if (r)
        return false;
    r = signal_context_set_crypto_provider(loki_signal_ctx, &mbedtls_signal_csp);
    if (r)
        return false;
    r = signal_context_set_locking_functions(loki_signal_ctx, loki_lock, loki_unlock);
    if (r)
        return false;
    new_user_ctx = malloc(sizeof (signal_user_ctx));
    if (!new_user_ctx)
        return false;
    r = signal_protocol_key_helper_generate_identity_key_pair(&new_user_ctx->identity_key_pair, loki_signal_ctx);
    if (r)
        return false;
    r = signal_protocol_key_helper_generate_registration_id(&new_user_ctx->registration_id, 0, loki_signal_ctx);
    if (r)
        return false;
    r = signal_protocol_key_helper_generate_pre_keys(&new_user_ctx->pre_keys_head, 1, 100, loki_signal_ctx);
    if (r)
        return false;
    r = signal_protocol_key_helper_generate_signed_pre_key(&new_user_ctx->signed_pre_key, new_user_ctx->identity_key_pair, 5, timestamp, loki_signal_ctx);
    if (r)
        return false;
    return true;
}

static void new_user()
{
    CDKLABEL *label;
    bool r;
    char *msg[3];
    char pubHex[65];
    char secretHex[65];
    unsigned char *data;
    signal_buffer *pub_key;
    signal_buffer *secret_key;
    ec_public_key *pub;
    ec_private_key *secret;

    char *msg2[] = {"Failed to run Signal"};
    r = boot_signal();
    if (r)
    {
        pub = ratchet_identity_key_pair_get_public(new_user_ctx->identity_key_pair);
        secret = ratchet_identity_key_pair_get_private(new_user_ctx->identity_key_pair);
        ec_public_key_serialize(&pub_key, pub);
        ec_private_key_serialize(&secret_key, secret);
        data = signal_buffer_data(pub_key);
        printHex(pubHex, data + 1);
        data = signal_buffer_data(secret_key);
        printHex(secretHex, data);
        msg[0] = "Save these keys somewhere safe!";
        msg[1] = alloca(512);
        msg[2] = alloca(512);
        snprintf(msg[1], 512, "Public Key:  %s", pubHex);
        snprintf(msg[2], 512, "Private Key: %s", secretHex);
        label = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2) msg, 3, TRUE, FALSE);
    }
    else
        label = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2) msg2, 1, TRUE, FALSE);
    set_window_title("<C>New User Registration");
    refreshCDKScreen(cdkscreen);
    waitCDKLabel(label, 0);
}

main(argc, argv)
char** argv;
{
    int status;
    CDK_PARAMS params;
    enum RESULT r;
    char *window_text[1];

    CDKparseParams(argc, argv, &params, "s:" CDK_CLI_PARAMS);

    /* start http */
    http_start = http_client_init();
    
    if (!http_start)
        return -1;

    /* Start curses. */
    cdkscreen = initCDKScreen(NULL);
    initCDKColor();
    curs_set(0);
    window_text[0] = "Welcome to Loki Pager";

    /* title bar */
    title = newCDKLabel(cdkscreen, CENTER, 0,
                        (CDK_CSTRING2) window_text, 1,
                        FALSE, FALSE);

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_GREEN);
    init_pair(3, COLOR_WHITE, COLOR_RED);

    /* Box our window. */
    box(stdscr, ACS_VLINE, ACS_HLINE);

    /* Display the first window */
    splash();
    refreshCDKScreen(cdkscreen);
#ifndef _EXPORT_BUILD
    export_warning();
#endif
    r = create_or_restore_seed();
    switch (r)
    {
    case RESTORE_EXISTING_SEED:
        restore_seed();
        break;
    case CREATE_NEW_SEED:
        new_user();
        break;
    default:
        break;
    };

    http_client_cleanup();

    destroyCDKLabel(title);
    destroyCDKScreen(cdkscreen);
    endCDK();
    status = 0;
#ifdef _WIN32
    DeleteCriticalSection(&global_mutex);
#endif

    return status;
}
