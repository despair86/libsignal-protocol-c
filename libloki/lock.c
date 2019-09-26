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

#include <loki.h>

#ifndef _WIN32
extern pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
void loki_lock(void *user_data)
{
#ifndef _WIN32
    pthread_mutex_lock(&global_mutex);
#else
    EnterCriticalSection(&global_mutex);
#endif
}

void loki_unlock(void *user_data)
{
#ifndef _WIN32
    pthread_mutex_unlock(&global_mutex);
#else
    LeaveCriticalSection(&global_mutex);
#endif
}