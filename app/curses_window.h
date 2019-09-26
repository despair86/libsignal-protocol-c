/*
 * Copyright (C) 2019 despair
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
 * File:   curses_window.h
 * Author: despair
 *
 * Created on September 25, 2019, 8:59 PM
 */

#ifndef CURSES_WINDOW_H
#define CURSES_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

    CDKLABEL *title;
    CDKSCREEN *cdkscreen;

    void set_window_title(const char*);

#ifdef __cplusplus
}
#endif

#endif /* CURSES_WINDOW_H */

