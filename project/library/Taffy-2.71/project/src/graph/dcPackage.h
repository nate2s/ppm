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

#ifndef __DC_PACKAGE_H__
#define __DC_PACKAGE_H__

#include "dcDefines.h"

struct dcPackage_t
{
    struct dcList_t *path;

    // is it wild?
    bool isWild;
};

typedef struct dcPackage_t dcPackage;

/**
 * @brief Creates a dcPackage node
 */
struct dcNode_t *dcPackage_createNode(struct dcList_t *_path);

/**
 * @brief Creates a dcPackage
 */
dcPackage *dcPackage_create(struct dcList_t *_path);

/**
 * @brief Creates a dcPackage node from a textual path
 */
dcPackage *dcPackage_createFromString(const char *_path);

/**
 * @brief Returns a copy of a dcPackage
 */
dcPackage *dcPackage_copy(const dcPackage *_package, dcDepth _unused);

/**
 * @brief Returns a string representation of a dcPackage
 */
const char *dcPackage_display(const dcPackage *_package);

/**
 * @brief Appends a string representation of a dcPackage to a stream
 */
dcResult dcPackage_print(const dcPackage *_package, struct dcString_t *_stream);

/**
 * @brief Extracts a directory name
 */
char *dcPackage_extractDirectoryName(const dcPackage *_package);

/**
 * @brief Frees a package
 */
void dcPackage_free(dcPackage **_package);

char *dcPackage_getPathString(const dcPackage *_package);
bool dcPackage_isWild(const struct dcNode_t *_package);

void dcPackage_pop(dcPackage *_package);
const char *dcPackage_getTail(const dcPackage *_package);

/**
 * @brief Compares a package with another
 */
dcResult dcPackage_compare(dcPackage *_left,
                           dcPackage *__right,
                           dcTaffyOperator *_compareResult);

bool dcPackage_equals(const dcPackage *_left, const dcPackage *_right);

void dcPackage_append(dcPackage *_package, const char *_path);

// standard functions //
COMPARE_FUNCTION(dcPackage_compareNode);
COPY_FUNCTION(dcPackage_copyNode);
FREE_FUNCTION(dcPackage_freeNode);
PRINT_FUNCTION(dcPackage_printNode);

#endif
