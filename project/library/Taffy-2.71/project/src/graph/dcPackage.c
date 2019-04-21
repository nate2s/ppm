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

#include <assert.h>
#include <string.h>

#include "dcFileManagement.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcPackage.h"
#include "dcString.h"
#include "dcStringManager.h"

//
// Path ordering is ensured by the Parser
// A wildcard (*) can only appear at the end of a path
//

dcNode *dcPackage_createNode(dcList *_path)
{
    return dcGraphData_createNodeWithGuts
        (NODE_PACKAGE, dcPackage_create(_path));
}

static dcPackage *create(dcList *_path, bool _isWild)
{
    dcPackage *result =
        (dcPackage *)dcMemory_allocateAndInitialize(sizeof(dcPackage));
    result->path = (_path == NULL
                    ? dcList_create()
                    : _path);
    result->isWild = _isWild;
    return result;
}

dcPackage *dcPackage_create(dcList *_path)
{
    dcPackage *result = create(_path, false);
    dcNode *tail = dcList_getTail(result->path);

    if (tail != NULL)
    {
        if (dcString_equalsCharArray(CAST_STRING(tail), "*"))
        {
            result->isWild = true;
            dcList_pop(result->path, DC_DEEP);
        }
    }

    return result;
}

void dcPackage_append(dcPackage *_package, const char *_path)
{
    dcList_push(_package->path, dcString_createNodeWithString(_path, true));
}

dcPackage *dcPackage_createFromString(const char *_path)
{
    return (strcmp(_path, "") == 0
            ? dcPackage_create(dcList_create())
            : dcPackage_create(dcLexer_splitString(_path, '.')));
}

void dcPackage_free(dcPackage **_package)
{
    if (_package != NULL && *_package != NULL)
    {
        dcPackage *package = *_package;
        dcList_free(&package->path, DC_DEEP);
        dcMemory_free(package);
    }
}

void dcPackage_freeNode(dcNode *_node, dcDepth _ignored)
{
    dcPackage_free(&CAST_PACKAGE(_node));
}

dcPackage *dcPackage_copy(const dcPackage *_from, dcDepth _ignored)
{
    return create(dcList_copy(_from->path, DC_DEEP), _from->isWild);
}

dcResult dcPackage_compare(dcPackage *_left,
                           dcPackage *_right,
                           dcTaffyOperator *_compareResult)
{
    if ((dcList_compare(_left->path, _right->path, _compareResult)
         == TAFFY_SUCCESS)
        && *_compareResult == TAFFY_EQUALS
        && _left->isWild == _right->isWild)
    {
        *_compareResult = TAFFY_EQUALS;
    }
    else
    {
        // lazy
        *_compareResult = TAFFY_LESS_THAN;
    }

    return TAFFY_SUCCESS;
}

bool dcPackage_equals(const dcPackage *_left, const dcPackage *_right)
{
    dcTaffyOperator operatorResult = TAFFY_LAST_OPERATOR;
    // derr
    return ((dcPackage_compare((dcPackage *)_left,
                               (dcPackage *)_right,
                               &operatorResult)
             == TAFFY_SUCCESS)
            && operatorResult == TAFFY_EQUALS);
}

dcResult dcPackage_compareNode(dcNode *_left,
                               dcNode *_right,
                               dcTaffyOperator *_compareResult)
{
    return dcPackage_compare(CAST_PACKAGE(_left),
                             CAST_PACKAGE(_right),
                             _compareResult);
}

dcTaffy_createCopyNodeFunctionMacro(dcPackage, CAST_PACKAGE);
dcTaffy_createDisplayFunctionMacro(dcPackage);
dcTaffy_createPrintNodeFunctionMacro(dcPackage, CAST_PACKAGE);

dcResult dcPackage_print(const dcPackage *_package, dcString *_stream)
{
    const dcListElement *that = NULL;
    dcString_appendString(_stream, "package \"");

    for (that = _package->path->head; that != NULL; that = that->next)
    {
        dcString_append(_stream,
                        "%s%s",
                        dcString_getString(that->object),
                        (that->next == NULL
                         ? ""
                         : "."));
    }

    if (_package->isWild)
    {
        dcString_append(_stream, ".*");
    }

    dcString_appendString(_stream, "\"");
    return TAFFY_SUCCESS;
}

static char *getPath(const dcPackage *_package,
                     char _separator,
                     bool _handleWild)
{
    char separator[2] = {_separator, 0};
    dcString *pathString = dcString_create();

    if (_package != NULL)
    {
        const dcListElement *that;

        for (that = _package->path->head; that != NULL; that = that->next)
        {
            dcString_append(pathString,
                            "%s%s",
                            dcString_getString(that->object),
                            (that->next == NULL
                             ? (_handleWild && _package->isWild
                                ? separator
                                : "")
                             : separator));
        }

        if (_handleWild && _package->isWild)
        {
            dcString_appendCharacter(pathString, '*');
        }
    }

    char *result = dcString_freeAndReturn(&pathString);
    return result;
}

char *dcPackage_getPathString(const dcPackage *_package)
{
    return getPath(_package, '.', true);
}

char *dcPackage_extractDirectoryName(const dcPackage *_package)
{
    return getPath(_package,
                   dcFileManagement_getDirectorySeparator(),
                   false);
}

bool dcPackage_isWild(const dcNode *_package)
{
    return CAST_PACKAGE(_package)->isWild;
}

void dcPackage_pop(dcPackage *_package)
{
    dcList_pop(_package->path, DC_DEEP);
}

const char *dcPackage_getTail(const dcPackage *_package)
{
    return (_package->path->size > 0
            ? dcString_getString(_package->path->tail->object)
            : NULL);
}
