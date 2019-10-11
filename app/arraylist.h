/*
 * Copyright (C) 2019 Rick V. All rights reserved.
 * Copyright (C) 2019 R Odili and memeDownloader contribs. All rights reserved.
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
 * File:   arraylist.h
 * Author: despair, kayomn, odilitime
 * Generic, dynamic, (mostly) type-safe, contiguous list comparable to C++'s `std::vector`.
 * Now supports C89 compilation systems!
 * Created on October 9, 2019, 6:01 PM
 */

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * Creates an anonymous `struct` representing a sequentially-allocated, dynamic list data type
 * capable of holding anything that is equal in size to the specified `TYPE`.
 *
 * In order to pass structures between functions and as members of aggregate types it is required
 * that it be `typedef`'d like so:
 * ```c
 * typedef ARRAYLIST(int) intList;
 *
 * struct Foo {
 *   intList list;
 * }
 *
 * void bar(intList* list);
 * ```
 *
 * @param TYPE Contained value type.
 */
#define ARRAYLIST(TYPE) \
struct {                \
  size_t capacity;      \
  size_t count;         \
  size_t weighting;     \
  TYPE *buffer;         \
}

/**
 * Forcibly resizes the target `ARRAYLIST` to the specified `SIZE`.
 *
 * Automatic dereferencing of `LIST` is not currently supported, so all dereferencing needs to be
 * done in the macro arguments like so:
 * ```c
 * ARRAYLIST_RESIZE((*list), 10);
 * ```
 *
 * @param LIST Target `ARRAYLIST`.
 * @param SIZE New size, with size's stride being defined by the target `ARRAYLIST`'s type.
 */
#define ARRAYLIST_RESIZE(LIST, SIZE)                                          \
do {                                                                          \
  void* oldBuffer = LIST.buffer;                                              \
  LIST.buffer = malloc(SIZE * sizeof(*LIST.buffer));                          \
  if (!LIST.buffer)                                                           \
  {                                                                           \
    printf("failed to resize array buffer!\n");                               \
    abort();                                                                  \
  }                                                                           \
  LIST.capacity = SIZE;                                                       \
                                                                              \
  if (SIZE < LIST.count) {                                                    \
    if (oldBuffer) {                                                          \
      memcpy(LIST.buffer, oldBuffer, (SIZE * sizeof(*LIST.buffer)));          \
    }                                                                         \
                                                                              \
    LIST.count = SIZE;                                                        \
  } else if (oldBuffer) {                                                     \
    memcpy(LIST.buffer, oldBuffer, (LIST.count * sizeof(*LIST.buffer)));      \
  }                                                                           \
} while (0)

#if defined(_MSC_VER) || (__STDC_VERSION__ < 201112L)
# define _Static_assert(x, s) ((void) sizeof (struct { unsigned:-!(x); }))
#endif

#if (__STDC_VERSION__ < 201112L) && !defined(__cplusplus)
# define static_assert _Static_assert
#endif

#if (__STDC_VERSION__ < 201112L) && defined(__cplusplus)
# define static_assert _Static_assert
#endif

/**
 * Pushes the given `VALUE` to the back of the target `ARRAYLIST`.
 *
 * Automatic dereferencing of `LIST` is not currently supported, so all dereferencing needs to be
 * done in the macro arguments like so:
 * ```c
 * ARRAYLIST_PUSH((*list), 696969);
 * ```
 *
 * @param LIST Target `ARRAYLIST`.
 * @param VALUE Value equal in size to the `TYPE` defined in `ARRAYLIST(TYPE)`.
 */
#define ARRAYLIST_PUSH(LIST, VALUE)                                               \
do {                                                                              \
  _Static_assert((sizeof(VALUE) == sizeof(*LIST.buffer)),                         \
      "LIST and VALUE type sizes do not match");                                  \
                                                                                  \
  if ((LIST.count + 1) > LIST.capacity) {                                         \
    ARRAYLIST_RESIZE(LIST, (LIST.count + (LIST.weighting ? LIST.weighting : 2))); \
  }                                                                               \
                                                                                  \
  LIST.buffer[LIST.count] = VALUE;                                                \
  LIST.count++;                                                                   \
} while (0)

/**
 * Pops the `n`th element of the target `ARRAYLIST`.
 *
 * Automatic dereferencing of `LIST` is not currently supported, so all dereferencing needs to be
 * done in the macro arguments like so:
 * ```c
 * ARRAYLIST_POP((*list));
 * ```
 *
 * @param LIST Target `ARRAYLIST`.
 * @return The `n`th` element.
 */
#define ARRAYLIST_POP(LIST) LIST.buffer[LIST.count -= 1]

/**
 * Returns the `I`th element of the target `ARRAYLIST`.
 *
 * Automatic dereferencing of `LIST` is not currently supported, so all dereferencing needs to be
 * done in the macro arguments like so:
 * ```c
 * ARRAYLIST_GET((*list), 42);
 * ```
 *
 * @param LIST Target `ARRAYLIST`.
 * @param I Index into the array list.
 * @return The `I`th` element.
 */
#define ARRAYLIST_GET(LIST, I) LIST.buffer[I]

/**
 * Removes all elements from the target `ARRAYLIST` while maintaining the existing buffer. Use
 * `ARRAYLIST_FREE(LIST)` to cleanup allocated memory.
 *
 * Automatic dereferencing of `LIST` is not currently supported, so all dereferencing needs to be
 * done in the macro arguments like so:
 * ```c
 * ARRAYLIST_CLEAR((*list));
 * ```
 *
 * @param LIST Target `ARRAYLIST`.
 */
#define ARRAYLIST_CLEAR(LIST) LIST.count = 0

/**
 * Removes all elements and frees the buffer from the target `ARRAYLIST`. If no elements are
 * present this has no effect. Use `ARRAYLIST_CLEAR(LIST)` to simply remove the elements and
 * sustain the existing buffer.
 *
 * Automatic dereferencing of `LIST` is not currently supported, so all dereferencing needs to be
 * done in the macro arguments like so:
 * ```c
 * ARRAYLIST_FREE((*list));
 * ```
 *
 * @param LIST Target `ARRAYLIST`.
 */
#define ARRAYLIST_FREE(LIST) \
do {                         \
  if (LIST.buffer) {         \
    free(LIST.buffer);       \
                             \
    LIST.buffer = NULL;      \
    LIST.capacity = 0;       \
                             \
    ARRAYLIST_CLEAR(LIST);   \
  }                          \
} while (0)


#ifdef __cplusplus
}
#endif

#endif /* ARRAYLIST_H */

