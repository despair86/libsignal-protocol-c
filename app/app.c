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
#include "app.h"
#include "mnemonic.h"

 /* For now, each screen in the user flow is coded as a separate closed
  * function. If anyone has any better ideas, please send in a patch!
  * -rick
  */
#define __LABEL__ "P A G E R   v e r s i o n   v 0 . 1"
static char* loki_logo[] = {
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

/* Changes the title bar by destroying and regenerating the
 * title bar itself (yikes)
 */
void set_window_title(char* name)
{
	char* window_text[1];

	window_text[0] = name;
	destroyCDKObject(title);
	title = newCDKLabel(cdkscreen, CENTER, 0,
		(CDK_CSTRING2)window_text, 1,
		FALSE, FALSE);
}

void splash()
{
	CDKLABEL* loki_label, * ua_label, * message_label, * copy_label;
	char* ua_text[1], * message[1], * copy[1];

	message[0] = "</B/02>Press any key to continue<!02>";
	copy[0] = "Copyright (c)2018-2019. All rights reserved.";
	ua_text[0] = client_ua;

	/* loki_logo */
	loki_label = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2)loki_logo, 15, FALSE, FALSE);

	ua_label = newCDKLabel(cdkscreen, CENTER, BOTTOM, (CDK_CSTRING2)ua_text, 1, FALSE, FALSE);
	moveCDKLabel(ua_label, 0, -1, TRUE, FALSE);

	message_label = newCDKLabel(cdkscreen, CENTER, BOTTOM, (CDK_CSTRING2)message, 1, TRUE, FALSE);
	moveCDKLabel(message_label, 0, -2, TRUE, FALSE);

	copy_label = newCDKLabel(cdkscreen, CENTER, TOP, (CDK_CSTRING2)copy, 1, FALSE, FALSE);
	moveCDKLabel(copy_label, 0, 2, TRUE, FALSE);

	refreshCDKScreen(cdkscreen);
	waitCDKLabel(message_label, 0);

	destroyCDKLabel(ua_label);
	destroyCDKLabel(message_label);
	destroyCDKLabel(copy_label);
	destroyCDKLabel(loki_label);
}

#ifndef _EXPORT_BUILD
void export_warning()
{
	CDKLABEL* warn_header, * msg, * message_label;

	char* warning[] = { "</B/03>E X P O R T  W A R N I N G<!03>" };
	char* msg2[] = { "</B/02>Press any key to continue<!02>" };
	char* warn_msg[] = {
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
	warn_header = newCDKLabel(cdkscreen, CENTER, TOP, (CDK_CSTRING2)warning, 1, TRUE, FALSE);
	moveCDKLabel(warn_header, 0, 1, TRUE, FALSE);
	msg = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2)warn_msg, 16, FALSE, FALSE);
	moveCDKLabel(title, CENTER, 0, FALSE, FALSE);
	message_label = newCDKLabel(cdkscreen, CENTER, BOTTOM, (CDK_CSTRING2)msg2, 1, TRUE, FALSE);
	moveCDKLabel(message_label, 0, -1, TRUE, FALSE);
	refreshCDKScreen(cdkscreen);
	waitCDKLabel(msg, 0);
	destroyCDKLabel(warn_header);
	destroyCDKLabel(msg);
	destroyCDKLabel(message_label);
}
#endif

int create_or_restore_seed()
{
	/* set window title */
	CDKRADIO* choices;
	enum RESULT r;

	char radio_title[] = "Create your Loki Messenger Account";
	char* radio_list[] = {
		"Restore from seed or file",
		"Register a new account"
	};

	set_window_title("<C>Loki Pager Setup");
	refreshCDKScreen(cdkscreen);
	choices = newCDKRadio(cdkscreen, CENTER, CENTER, NONE, 5, 20, radio_title, (CDK_CSTRING2)radio_list, 2, '*' | A_REVERSE, 1, A_REVERSE, TRUE, FALSE);
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
	/* NOTREACHED */
	return -1;
}

void restore_seed()
{
	CDKLABEL* error_msg;
	char* msg[] = { "Not implemented yet, press any key to exit." };

	error_msg = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2)msg, 1, TRUE, FALSE);
	refreshCDKScreen(cdkscreen);
	set_window_title("<C>Restore keys from seed or file");
	refreshCDKScreen(cdkscreen);
	waitCDKLabel(error_msg, 0);
	destroyCDKLabel(error_msg);
}

/* Windows NT only. Copies a string to the clipboard. */
#ifdef _WIN32
static void copy_to_clipboard(str)
const char* str;
{
	HGLOBAL hdst;
	LPSTR dst;
	size_t len;

	len = strlen(str);
	hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, len + 1);
	dst = GlobalLock(hdst);
	memcpy(dst, str, len + 1);
	dst[len] = 0;
	GlobalUnlock(hdst);
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hdst);
	CloseClipboard();
}
#endif

void new_user()
{
	CDKLABEL* label, *clip_label;
	char* msg[9], * clip_msg[2], * seed, * key, * word, * loc;
	int i, j;
	stringList seed_list;

	memset(&seed_list, 0, sizeof(stringList));
	msg[0] = "<C>Please save the seed below in a safe location.";
	msg[1] = "<C>It can be used to restore your account if you lose access";
	msg[2] = "<C>or migrate to a new device.";
	msg[3] = "";

#ifdef _WIN32
	clip_msg[0] = "<C>This seed has been copied to the clipboard.";
	clip_msg[1] = "<C>The clipboard will be cleared when Pager exits.";
#endif

	/* get our key */
	key = signal_buffer_data(new_user_ctx->secret_key);
	/* encode key into seed */
	seed = mnemonic_encode(key, w, signal_buffer_len(new_user_ctx->secret_key));
#ifdef _WIN32
	copy_to_clipboard(seed);
#endif
	/* regenerate the list we just junked */
	for (word = strtok_r(seed, " ", &loc); word != NULL; word = strtok_r(NULL, " ", &loc))
		ARRAYLIST_PUSH(seed_list, strdup(word));
	/* original string is modified, trash it */
	mbedtls_platform_zeroize(seed, 8192);
	free(seed);
	/* load words five at a time */
	msg[4] = alloca(1024);
	msg[5] = alloca(1024);
	msg[6] = alloca(1024);
	msg[7] = alloca(1024);
	msg[8] = alloca(1024);

	/* Is it possible to optimise this? */
	for (i = 4; i < 9; i++)
		memset(msg[i], 0, 1024);
	strlcat(msg[4], "<C>", 1024);
	strlcat(msg[5], "<C>", 1024);
	strlcat(msg[6], "<C>", 1024);
	strlcat(msg[7], "<C>", 1024);
	strlcat(msg[8], "<C>", 1024);
	for (i = 0; i < 5; i++)
	{
		word = ARRAYLIST_GET(seed_list, i);
		strlcat(msg[4], word, 1024);
		strlcat(msg[4], " ", 1024);
	}
	for (i = 5; i < 10; i++)
	{
		word = ARRAYLIST_GET(seed_list, i);
		strlcat(msg[5], word, 1024);
		strlcat(msg[5], " ", 1024);
	}
	for (i = 10; i < 15; i++)
	{
		word = ARRAYLIST_GET(seed_list, i);
		strlcat(msg[6], word, 1024);
		strlcat(msg[6], " ", 1024);
	}
	for (i = 15; i < 20; i++)
	{
		word = ARRAYLIST_GET(seed_list, i);
		strlcat(msg[7], word, 1024);
		strlcat(msg[7], " ", 1024);
	}
	for (i = 20; i < 25; i++)
	{
		word = ARRAYLIST_GET(seed_list, i);;
		strlcat(msg[8], word, 1024);
		strlcat(msg[8], " ", 1024);
	}
	/* generate UI widgets */
	label = newCDKLabel(cdkscreen, CENTER, CENTER, (CDK_CSTRING2)msg, 9, TRUE, FALSE);
#ifdef _WIN32
	clip_label = newCDKLabel(cdkscreen, CENTER, LINES - 3, clip_msg, 2, FALSE, FALSE);
#endif
	set_window_title("<C>New User Registration");
	refreshCDKScreen(cdkscreen);
	waitCDKLabel(label, 0);

	/* scrub everything */
	for (i = 4; i < 9; i++)
		mbedtls_platform_zeroize(msg[i], 1024);
	while (seed_list.count)
	{
		word = ARRAYLIST_POP(seed_list);
		mbedtls_platform_zeroize(word, strlen(word) + 1);
		free(word);
	}
	ARRAYLIST_FREE(seed_list);
}

#ifndef _MSC_VER
DECLSPEC_NORETURN void fatal_error(msg)
#else
void fatal_error(msg) DECLSPEC_NORETURN
#endif
const char* msg;
{
#ifdef _WIN32
	MessageBox(NULL, msg, "Fatal Error", MB_ICONHAND);
	abort();
#else
	fprintf(stderr, "%s\n", msg);
	abort();
#endif
}
