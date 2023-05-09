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

#include "dcArray.h"
#include "dcCFunctionArgument.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcExceptionClass.h"
#include "dcExceptions.h"
#include "dcNode.h"
#include "dcGraphData.h"
#include "dcLexer.h"
#include "dcList.h"
#include "dcMemory.h"
#include "dcParseErrorExceptionClass.h"
#include "dcScope.h"
#include "dcScopeData.h"
#include "dcString.h"
#include "dcStringClass.h"
#include "dcSystem.h"
#include "dcTaffyCMethodWrapper.h"

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcParseErrorExceptionClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (PARSE_ERROR_EXCEPTION_PACKAGE_NAME, // package name
          PARSE_ERROR_EXCEPTION_CLASS_NAME,   // class name
          MAKE_FULLY_QUALIFIED(EXCEPTION),    // super type
          NO_FLAGS,                           // class flags
          NO_FLAGS,                           // scope data flags
          NULL,                               // meta methods
          NULL,                               // methods
          NULL,                               // class initialization function
          NULL,                               // class deinitialization function
          NULL,                               // allocate
          NULL,                               // deallocate
          NULL,                               // meta mark
          NULL,                               // mark
          NULL,                               // copy
          NULL,                               // free
          NULL,                               // register
          NULL,                               // marshall
          NULL,                               // unmarshall
          NULL));                             // set template
}

dcNode *dcParseErrorExceptionClass_createNode(const char *_text,
                                              bool _object)
{
    return (dcClass_createNode
            (sTemplate,
             dcExceptionClass_createNode
             (dcStringClass_createObject(_text, true), _object),
             NULL, // scope
             _object,
             NULL)); // aux
}

void dcParseErrorExceptionClass_throwObject(const char *_errorText)
{
    dcExceptions_throwObject(dcParseErrorExceptionClass_createNode
                             (_errorText, true));
}
