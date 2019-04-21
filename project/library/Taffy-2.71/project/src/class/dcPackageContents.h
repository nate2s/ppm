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

#ifndef __DC_PACKAGE_CONTENTS_H__
#define __DC_PACKAGE_CONTENTS_H__

#include "dcDefines.h"

struct dcPackageContents_t
{
    // where the classes reside
    struct dcHash_t *classes;

    //
    // contents: dcString => dcPackageContents
    // the next level of the package map
    //
    struct dcHash_t *subMap;

    // the package itself
    struct dcPackage_t *package;
};

typedef struct dcPackageContents_t dcPackageContents;

struct dcNode_t *dcPackageContents_createNode(struct dcPackage_t *_package);
void dcPackageContents_free(dcPackageContents **_contents, dcDepth _unused);
const char *dcPackageContents_display(const dcPackageContents *_contents);
struct dcNode_t *dcPackageContents_createShell(dcPackageContents *_contents);

// debug //
dcPackageContents *dcPackageContents_castMe(struct dcNode_t *_node);

FREE_FUNCTION(dcPackageContents_freeNode);
PRINT_FUNCTION(dcPackageContents_printNode);

#endif
