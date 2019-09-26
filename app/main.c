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

#include <cdk.h>
#include <signal_protocol.h>
#include <key_helper.h>
#include "crypto_provider_mbedtls.h"
#include "loki.h"
#include "http.h"
#include "curses_window.h"

/* Global variables and fixed app-specific data */
char *XCursesProgramName = "Loki Messenger";
#define __LABEL__ "P A G E R   v e r s i o n   v 0 . 1"

static char *loki_logo[] = {
    "</01>        .o0l.           <!01>",
    "</01>       ;kNMNo.          <!01>",
    "</01>     ;kNMMXd'           <!01>",
    "</01>   ;kNMMXd'             <!01>  .ld:             ,ldxkkkdl,.     'dd;     ,odl.  ;dd",
    "</01> ;kNMMXo.  'ol.         <!01>  ,KMx.          :ONXkollokXN0c.   cNMo   .dNNx'   dMW",
    "</01>dNMMM0,   ;KMMXo.       <!01>  ,KMx.        .oNNx'      .dNWx.  :NMo .cKWk;     dMW",
    "</01>'dXMMNk;  .;ONMMXo'     <!01>  ,KMx.        :NMx.         oWWl  cNWd;ON0:.      oMW",
    "</01>  'dXMMNk;.  ;kNMMXd'   <!01>  ,KMx.        lWWl          :NMd  cNMNNMWd.       dMW",
    "</01>    'dXMMNk;.  ;kNMMXd' <!01>  ,KMx.        :NMx.         oWWl  cNMKolKWO,      dMW",
    "</01>      .oXMMK;   ,0MMMNd.<!01>  ,KMx.        .dNNx'      .dNWx.  cNMo  .dNNd.    dMW",
    "</01>        .lo'  'dXMMNk;. <!01>  ,KMXxdddddl.   :ONNkollokXN0c.   cNMo    ;OWKl.  dMW",
    "</01>            'dXMMNk;    <!01>  .lddddddddo.     ,ldxkkkdl,.     'od,     .cdo;  ;dd",
    "</01>          'dXMMNk;      <!01>",
    "</01>         .oNMNk;        <!01>   " __LABEL__,
    "</01>          .l0l.         <!01>"
};

static CDKSCREEN *cdkscreen;
static bool http_start = false;
static signal_context *loki_signal_ctx;
extern signal_crypto_provider mbedtls_signal_csp;

/* For now, each screen in the user flow is coded as a separate closed
 * function. If anyone has any better ideas, please send in a patch!
 * -rick
 */
static void splash()
{
    CDKLABEL *loki_label, *ua_label, *message_label, *copy_label;
    char *ua_text[1], *message[1], *copy[1];

    message[0] = "</B/02>Press any key to continue<!02>";
    copy[0] = "Copyright (c)2018-2019. All rights reserved.";
    ua_text[0] = client_ua;

    /* loki_logo */
    loki_label = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2) loki_logo, 15, FALSE, FALSE);

    if (http_start)
    {
        ua_label = newCDKLabel(cdkscreen, CENTER, BOTTOM, (CDK_CSTRING2) ua_text, 1, FALSE, FALSE);
        moveCDKLabel(ua_label, 0, -1, TRUE, FALSE);

        message_label = newCDKLabel(cdkscreen, CENTER, BOTTOM, (CDK_CSTRING2) message, 1, TRUE, FALSE);
        moveCDKLabel(message_label, 0, -2, TRUE, FALSE);

        copy_label = newCDKLabel(cdkscreen, CENTER, TOP, (CDK_CSTRING2) copy, 1, FALSE, FALSE);
        moveCDKLabel(copy_label, 0, 2, TRUE, FALSE);

        refreshCDKScreen(cdkscreen);
        waitCDKLabel(message_label, 0);
    }
    else
    {
        printw("failed to start web client\n");
        refreshCDKScreen(cdkscreen);
    }
    destroyCDKLabel(ua_label);
    destroyCDKLabel(message_label);
    destroyCDKLabel(copy_label);
}

/*
static void printHex()
{
    char md5string[33];
    for (int i = 0; i < 16; ++i)
        sprintf(&md5string[i * 2], "%02x", (unsigned int) digest[i]);
}
 */

static bool boot_signal()
{
    signal_user_ctx *new_user_ctx;
#ifdef _WIN32
    __time64_t timestamp;
    InitializeCriticalSection(&global_mutex);
    __time64(&timestamp);
#else
    time_t timestamp;
    time(&timestamp);
#endif
    signal_context_create(&loki_signal_ctx, NULL);
    signal_context_set_crypto_provider(loki_signal_ctx, &mbedtls_signal_csp);
    signal_context_set_locking_functions(loki_signal_ctx, loki_lock, loki_unlock);
    new_user_ctx = malloc(sizeof (signal_user_ctx));
    if (!new_user_ctx)
        return false;
    signal_protocol_key_helper_generate_identity_key_pair(&new_user_ctx->identity_key_pair, loki_signal_ctx);
    signal_protocol_key_helper_generate_registration_id(&new_user_ctx->registration_id, 0, loki_signal_ctx);
    signal_protocol_key_helper_generate_pre_keys(&new_user_ctx->pre_keys_head, 1, 100, loki_signal_ctx);
    signal_protocol_key_helper_generate_signed_pre_key(&new_user_ctx->signed_pre_key, new_user_ctx->identity_key_pair, 5, timestamp, loki_signal_ctx);
    return true;
}

#ifndef _EXPORT_BUILD

static void export_warning()
{
    CDKLABEL *warn_header, *msg;

    char *warning[] = {"</B/03>E X P O R T  W A R N I N G<!03>"};
    char *warn_msg[] = {
        "This distribution includes cryptographic software. The country in which you",
        "currently reside may have restrictions on the import, possession, use,",
        "and/or re-export to another country, of encryption software. BEFORE using",
        "any encryption software, please check your country's laws, regulations and",
        "policies concerning the import, possession, or use, and re-export of",
        "encryption software, to see if this is permitted. See",
        "http://www.wassenaar.org/ for more information.",
        "",
        "The U.S. Government Department of Commerce, Bureau of Industry and Security",
        "(BIS), has classified this software as Export Commodity Control Number",
        "(ECCN) 5D002.C.1, which includes information security software using or",
        "performing cryptographic functions with asymmetric algorithms. The form and",
        "manner of this distribution makes it eligible for export under the License",
        "Exception ENC Technology Software Unrestricted (TSU) exception (see the BIS",
        "Export Administration Regulations, Section 740.13) for both object code and",
        "source code."
    };
    warn_header = newCDKLabel(cdkscreen, CENTER, TOP, (CDK_CSTRING2) warning, 1, TRUE, FALSE);
    moveCDKLabel(warn_header, 0, 1, TRUE, FALSE);
    msg = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2) warn_msg, 16, FALSE, FALSE);
    moveCDKLabel(title, CENTER, 0, FALSE, FALSE);
    refreshCDKScreen(cdkscreen);
    waitCDKLabel(msg, 0);
    destroyCDKLabel(warn_header);
    destroyCDKLabel(msg);
}
#endif

enum RESULT
{
    RESTORE_EXISTING_SEED,
    CREATE_NEW_SEED
};

static void restore_seed()
{
    set_window_title("<C>Restore keys from seed or file");
    moveCDKLabel(title, CENTER, 0, FALSE, TRUE);
    waitCDKLabel(title, 0);
}

static void new_account()
{
    set_window_title("<C>New Account Registration");
    moveCDKLabel(title, CENTER, 0, FALSE, TRUE);
    waitCDKLabel(title, 0);
}

static int create_or_restore_seed()
{
    /* set window title */
    CDKRADIO *choices;
    enum RESULT r;

    char radio_title[] = "Create your Loki Messenger Account";
    char *radio_list[] = {
        "Restore from seed or file",
        "Register a new account"
    };
    set_window_title("<C>Loki Pager Setup");
    moveCDKLabel(title, CENTER, 0, FALSE, TRUE);
    choices = newCDKRadio(cdkscreen, CENTER, CENTER, NONE, 5, 20, radio_title, (CDK_CSTRING2) radio_list, 2, '*' | A_REVERSE, 1, A_REVERSE, TRUE, FALSE);
    r = activateCDKRadio(choices, NULL);
    refreshCDKScreen(cdkscreen);
    if (choices->exitType == vESCAPE_HIT)
    {
        destroyCDKRadio(choices);
        return -1;
    }
    if (choices->exitType == vNORMAL)
    {
        destroyCDKRadio(choices);
        return r;
    }
}

main(argc, argv)
char** argv;
{
    int status;
    CDK_PARAMS params;
    enum RESULT r;
    char *window_text[1];

    CDKparseParams(argc, argv, &params, "s:" CDK_CLI_PARAMS);

    /* Start curses. */
    cdkscreen = initCDKScreen(NULL);
    initCDKColor();
    curs_set(0);
    window_text[0] = "Welcome to Loki Messenger";

    /* title bar */
    title = newCDKLabel(cdkscreen, CENTER, 0,
                        (CDK_CSTRING2) window_text, 1,
                        FALSE, FALSE);

    /* start http */
    http_start = http_client_init();

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
        new_account();
        break;
    default:
        break;
    };

    if (!http_start)
        status = -1;
    else
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
