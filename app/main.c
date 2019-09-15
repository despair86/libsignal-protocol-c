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
#include "http.h"
#define __LABEL__ "P A G E R   v e r s i o n   v0.1"

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
"          .l0l.\n"
};

main(argc,argv)
char** argv;
{
    puts(loki_logo);
    if(http_client_init())
    {
        printf("HTTP Client User-Agent: %s\n", client_ua);
        http_client_cleanup();
    }
    else
    {
        printf("failed to start web client\n");
    }
    return 0;
}
