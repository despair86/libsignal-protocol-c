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

#include <cdk.h>

char *XCursesProgramName = "Loki Messenger";

#include "http.h"
#define __LABEL__ "P A G E R   v e r s i o n   v 0 . 1"

char *loki_logo[15] = {
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
static int http_init = 0;

static void splash()
{
    CDKLABEL *title, *loki_label, *ua_label;
    char *text[1], *ua_text[2];

    /* Box our window. */
    box(stdscr, ACS_VLINE, ACS_HLINE);
    text[0] = "Welcome to Loki Pager";
    title = newCDKLabel(cdkscreen, CENTER, 0,
                        (CDK_CSTRING2) text, 1,
                        FALSE, FALSE);
    ua_text[0] = alloca(512);
    loki_label = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2) loki_logo, 15, FALSE, FALSE);

    if (http_client_init())
    {
        sprintf(ua_text[0], "HTTP Client User-Agent: %s\n", client_ua);
        ua_label = newCDKLabel(cdkscreen, CENTER, BOTTOM, (CDK_CSTRING2)ua_text, 1, FALSE, FALSE);
        http_init = 1;
    }
    else
    {
        printw("failed to start web client\n");
        refreshCDKScreen(cdkscreen);
    }
}

main(argc, argv)
char** argv;
{
    int status;
    CDK_PARAMS params;

    CDKparseParams(argc, argv, &params, "s:" CDK_CLI_PARAMS);

    /* Start curses. */
    cdkscreen = initCDKScreen(NULL);
    initCDKColor();
    curs_set(0);

    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    /* Display the first window, and start the web client */
    splash();

    if (!http_init)
    {
        status = -1;
    }
    
    destroyCDKScreen(cdkscreen);
    endCDK();
    status = 0;

    return status;
}
