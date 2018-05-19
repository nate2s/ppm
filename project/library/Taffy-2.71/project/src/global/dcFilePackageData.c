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

#include "dcFilePackageData.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcMutex.h"
#include "dcNode.h"
#include "dcPackage.h"
#include "dcString.h"

dcNode *dcFilePackageData_createNode(void)
{
    return dcNode_createWithGuts(NODE_FILE_PACKAGE_DATA,
                                 dcFilePackageData_create());
}

dcFilePackageData *dcFilePackageData_create(void)
{
    dcFilePackageData *data =
        (dcFilePackageData *)(dcMemory_allocateAndInitialize
                              (sizeof(dcFilePackageData)));
    data->packageContents = dcList_create();
    data->deferredImports = dcList_create();
    data->mutex = dcMutex_create(false);
    return data;
}

void dcFilePackageData_free(dcFilePackageData **_data)
{
    dcFilePackageData *data = *_data;
    dcList_free(&data->packageContents, DC_SHALLOW);
    dcList_free(&data->deferredImports, DC_DEEP);
    dcPackage_free(&data->package);
    dcMutex_free(&data->mutex);
    dcMemory_free(data);
}

void dcFilePackageData_freeNode(dcNode *_packageDataNode, dcDepth _depth)
{
    dcFilePackageData_free(&(CAST_FILE_PACKAGE_DATA(_packageDataNode)));
}

dcResult dcFilePackageData_printNode(const dcNode *_packageData,
                                     dcString *_stream)
{
    dcFilePackageData *data = CAST_FILE_PACKAGE_DATA(_packageData);
    dcString_append(_stream, "[FilePackageData:\nPackage: ");
    dcPackage_print(data->package, _stream);
    dcString_append(_stream, "\nPackage Contents: ");
    dcList_print(data->packageContents, _stream);
    dcString_append(_stream, "\nDeferred Imports:");
    dcList_print(data->deferredImports, _stream);
    dcString_append(_stream, "\n]");
    return TAFFY_SUCCESS;
}
