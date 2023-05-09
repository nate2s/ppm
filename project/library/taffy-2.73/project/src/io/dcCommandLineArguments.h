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

#ifndef __DC_COMMAND_LINE_ARGUMENTS_H__
#define __DC_COMMAND_LINE_ARGUMENTS_H__

#include "dcDefines.h"

struct dcCommandLineArguments_t
{
    // both hashes have elements that may point
    // to the same dcCommandLineArgument
    struct dcHash_t *tinyArguments;
    struct dcHash_t *bigArguments;

    // the default argument
    struct dcCommandLineArgument_t *defaultArgument;
};

typedef struct dcCommandLineArguments_t dcCommandLineArguments;

dcCommandLineArguments *dcCommandLineArguments_create(void);
void dcCommandLineArguments_free(dcCommandLineArguments **_arguments);

/* @brief The default argument is that which values with no left-hand-side
 * are stored in
 *
 * Example:
 *
 * taffy foo.ty -i directory1
 *
 * the default argument contains "foo.ty" in its values
 */
void dcCommandLineArguments_setDefaultArgument
    (dcCommandLineArguments *_arguments,
     const char *_argument);

void dcCommandLineArguments_registerArguments
    (dcCommandLineArguments *_arguments,
     const char *_tinyArgument,
     const char *_largeArgument);

/* @brief Parse a command line and populate the given _arguments state
 */
bool dcCommandLineArguments_parse
    (dcCommandLineArguments *_arguments,
     int _argc,
     char **_argv,
     bool _failOnUnknownArgument);

// getting //

/* @brief Was an argument given at all, even if no values are present?
 */
bool dcCommandLineArguments_getHit(const dcCommandLineArguments *_arguments,
                                     const char *_argumentName);

/* @brief Was an argument (out of two possibilities) given at all,
 * even if no values are present?
 */
bool dcCommandLineArguments_getHits(const dcCommandLineArguments *_arguments,
                                      const char *_argumentName1,
                                      const char *_argumentName2);

/* @brief Get the dcCommandLineArgument* for an argument
 */
struct dcCommandLineArgument_t *dcCommandLineArguments_get
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName);

/* @brief Get the right hand side of an argument
 */
struct dcList_t *dcCommandLineArguments_getValues
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName);

/* @brief Get the integer value for a given argument
 */
int dcCommandLineArguments_getInt
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName,
     int _default);

/* @brief Get the string for a given argument
 */
const struct dcString_t *dcCommandLineArguments_getString
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName);

/* @brief Get a string list for a given argument
 * The returned dcList* is created on the heap, and must be deleted
 * by the caller
 */
struct dcList_t *dcCommandLineArguments_getStringList
    (const dcCommandLineArguments *_arguments,
     const char *_argumentName);

/* @brief Get an integer list for a given argument
 * The returned dcList* is created on the heap, and must be deleted
 * by the caller
 */
struct dcList_t *dcCommandLineArguments_getIntList
    (const dcCommandLineArguments *_args,
     const char *_argumentName);

#endif
