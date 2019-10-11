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
  * File:   mnemonic.h
  * Author: despair
  * mnemonic stuff
  *
  * Created on October 2, 2019, 7:58 PM
  */

#ifndef MNEMONIC_H
#define MNEMONIC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "arraylist.h"

    typedef ARRAYLIST(char*) stringList;
    
	typedef enum
	{
		LANGUAGE_DEFAULT,
		/* Actually simplfied. Pls fix and remove ZH-CN mnemonics! */
		LANGUAGE_ZH_TW,
		LANGUAGE_NL,
		LANGUAGE_EN_ELECTRUM,
		LANGUAGE_EN,
		LANGUAGE_EO,
		LANGUAGE_FR,
		LANGUAGE_DE,
		LANGUAGE_IT,
		LANGUAGE_JP,
		LANGUAGE_LOJBAN,
		LANGUAGE_PT,
		LANGUAGE_SU,
		LANGUAGE_ES
	} language_code;

	typedef struct
	{
		language_code lc;
		int prefix_length;
        /* truncated_words will be empty for Electrum */
        stringList words, truncated_words;
	} wordlist;

	wordlist* initialise_wordlist(language_code);
	void destroy_wordlist(wordlist*);
	char *mnemonic_encode(char*, wordlist*);
    
#ifdef __cplusplus
}
#endif

#endif /* MNEMONIC_H */

