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
#endif

/* On Linux only ncurses exists, duh */
#if defined(__linux__)
#include <ncurses/curses.h>
#else
/* We can use AT&T or BSD curses too */
#include <curses.h>
#endif

#include "http.h"
#define __LABEL__ "P A G E R   v e r s i o n   v 0 . 1"

char loki_logo[] = {
    "        .o0l.\n"
    "       ;kNMNo.\n"
    "     ;kNMMXd'\n"
    "   ;kNMMXd'                 .ld:             ,ldxkkkdl,.     'dd;     ,odl.  ;dd\n"
    " ;kNMMXo.  'ol.             ,KMx.          :ONXkollokXN0c.   cNMo   .dNNx'   dMW\n"
    "dNMMM0,   ;KMMXo.           ,KMx.        .oNNx'      .dNWx.  :NMo .cKWk;     dMW\n"
    "'dXMMNk;  .;ONMMXo'         ,KMx.        :NMx.         oWWl  cNWd;ON0:.      oMW\n"
    "  'dXMMNk;.  ;kNMMXd'       ,KMx.        lWWl          :NMd  cNMNNMWd.       dMW\n"
    "    'dXMMNk;.  ;kNMMXd'     ,KMx.        :NMx.         oWWl  cNMKolKWO,      dMW\n"
    "      .oXMMK;   ,0MMMNd.    ,KMx.        .dNNx'      .dNWx.  cNMo  .dNNd.    dMW\n"
    "        .lo'  'dXMMNk;.     ,KMXxdddddl.   :ONNkollokXN0c.   cNMo    ;OWKl.  dMW\n"
    "            'dXMMNk;        .lddddddddo.     ,ldxkkkdl,.     'od,     .cdo;  ;dd\n"
    "          'dXMMNk;\n"
    "         .oNMNk;             " __LABEL__ "\n"
    "          .l0l."
};

main(argc, argv)
char** argv;
{
    int x,y,i;

    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    for(i = 0; i < sizeof(loki_logo); i++)
    {
        if (loki_logo[i] == 0)
            break;
        getyx(stdscr, y, x);
        if (x > 25)
            addch(loki_logo[i]|COLOR_PAIR(2));
        else
            addch(loki_logo[i]|COLOR_PAIR(1));
    }
    printw("\n\n");
    refresh();
    if (http_client_init())
    {
        printw("HTTP Client User-Agent: %s\n", client_ua);
        refresh();
        http_client_cleanup();
    }
    else
    {
        printw("failed to start web client\n");
        refresh();
    }
    printw("\nPress any key to exit\n");
    refresh();
    getch();
    endwin();
    return 0;
}
