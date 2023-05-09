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

#ifndef __DC_FILE_PACKAGE_DATA_H__
#define __DC_FILE_PACKAGE_DATA_H__

#include "dcDefines.h"

typedef enum
{
    FILE_PACKAGE_DATA_NONE,
    FILE_PACKAGE_DATA_IMPORTING,
    FILE_PACKAGE_DATA_IMPORTED
} FilePackageDataState;

struct dcFilePackageData_t
{
    struct dcPackage_t *package;
    struct dcList_t *packageContents;
    struct dcList_t *deferredImports;
    struct dcMutex_t *mutex;

    // don't import more than once
    FilePackageDataState state;

    // is this file package data deferred until later?
    // this variable exists as a convenience helper, to alleviate
    // the need to perform an expensive search on dcNodeEvaluator's
    // deferredImports list
    bool deferred;
};

typedef struct dcFilePackageData_t dcFilePackageData;

struct dcNode_t *dcFilePackageData_createNode(void);
dcFilePackageData *dcFilePackageData_create(void);

// deep free the deferred imports
// shallow free the package contents
void dcFilePackageData_free(dcFilePackageData **_contents);

// standard functions
FREE_FUNCTION(dcFilePackageData_freeNode);
PRINT_FUNCTION(dcFilePackageData_printNode);

#endif
