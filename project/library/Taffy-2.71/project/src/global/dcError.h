//
// This file is part of Taffy, a mathematical programming language.
// Copyright (C) 2016-2017 Arithmagic, LLC
//
// Taffy is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Taffy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __DC_ERROR_H__
#define __DC_ERROR_H__

#include <stdio.h>
#include <stdlib.h>

#define dcError_assert(what)                                  \
    do                                                        \
    {                                                         \
        if (!(what))                                          \
        {                                                     \
            fprintf(stderr, "Assert failure: %s\n", #what);   \
            abort();                                          \
        }                                                     \
    } while(0)

#define dcError_check(what, ...)                                        \
    do                                                                  \
    {                                                                   \
        if (!(what))                                                    \
        {                                                               \
            fprintf(stderr, "Fatal internal error check fail: %s\n",    \
                    #what);                                             \
            fprintf(stderr, __VA_ARGS__);                               \
            abort();                                                    \
        }                                                               \
    } while (0)

#define dcError_internal(...)            \
    fprintf(stderr, "***\n");     \
    fprintf(stderr, "Fatal internal error\n"); \
    fprintf(stderr, __VA_ARGS__);        \
    fprintf(stderr, "\n***\n");     \
    abort();

#define dcError(...)              \
    fprintf(stderr, "***\n");     \
    fprintf(stderr, "Error\n");   \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n***\n");     \
    abort();

#define dcWarning(...)            \
    fprintf(stderr, "***\n");     \
    fprintf(stderr, "Warning\n"); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n***\n");

#define dcAbort(...)                            \
    fprintf(stderr, "***\nError (abort): %s\n***\n", __VA_ARGS__)

#endif
