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

#include <string.h>

#include "dcGraphData.h"
#include "dcImport.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcNode.h"
#include "dcPackage.h"
#include "dcString.h"
#include "dcTestUtilities.h"

dcList *dcImportAndPackageTest_createPath(const char *_path)
{
    return dcLexer_splitString(_path, '.');
}

void dcImportAndPackageTest_checkContents(dcPackage *_package,
                                          const char *_path)
{
    dcList *path = dcLexer_splitString(_path, '.');
    dcListElement *thisPackagePath = _package->path->head;
    dcListElement *thisPath = path->head;

    const char *last = dcString_getString(dcList_getTail(path));
    dcTestUtilities_assert(_package->isWild
                           == (last[strlen(last) - 1] == '*'));

    for (thisPackagePath = _package->path->head, thisPath = path->head;
         thisPackagePath != NULL && _path != NULL;
         thisPackagePath = thisPackagePath->next, thisPath = thisPath->next)
    {
        dcTestUtilities_assert
            (strcmp(dcString_getString(thisPackagePath->object),
                    dcString_getString(thisPath->object))
             == 0);
    }

    dcList_free(&path, DC_DEEP);
}
