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

#include "dcHash.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcPackage.h"
#include "dcPackageContents.h"
#include "dcString.h"

dcNode *dcPackageContents_createNode(dcPackage *_package)
{
    dcPackageContents *result =
        (dcPackageContents *)(dcMemory_allocateAndInitialize
                              (sizeof(dcPackageContents)));
    result->classes = dcHash_create();
    result->subMap = dcHash_create();
    result->package = _package;
    return dcNode_createWithGuts(NODE_PACKAGE_CONTENTS, result);
}

void dcPackageContents_free(dcPackageContents **_contents, dcDepth _unused)
{
    dcPackageContents *contents = *_contents;
    dcPackage_free(&contents->package);
    dcHash_free(&contents->classes, DC_DEEP);
    dcHash_free(&contents->subMap, DC_DEEP);
    dcMemory_free(contents);
}

dcResult dcPackageContents_print(const dcPackageContents *_contents,
                                 dcString *_stream)
{
    dcString_appendString(_stream, "[PackageContentsList ");
    dcPackage_print(_contents->package, _stream);
    dcString_append(_stream,
                    " | classes: %u | subMaps: %u]",
                    _contents->classes->size,
                    _contents->subMap->size);
    return TAFFY_SUCCESS;
}

dcNode *dcPackageContents_createShell(dcPackageContents *_contents)
{
    return dcNode_createWithGuts(NODE_PACKAGE_CONTENTS, _contents);
}

dcTaffy_createDisplayFunctionMacro(dcPackageContents);
dcTaffy_createPrintNodeFunctionMacro(dcPackageContents, CAST_PACKAGE_CONTENTS);
dcTaffy_createFreeNodeFunctionMacro(dcPackageContents, CAST_PACKAGE_CONTENTS);

// debug
dcPackageContents *dcPackageContents_castMe(dcNode *_node)
{
    return CAST_PACKAGE_CONTENTS(_node);
}
