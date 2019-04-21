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

//
// Main is the dummy object that sits at the top of the heap
//

#include "CompiledMain.h"
#include "dcClass.h"
#include "dcClassManager.h"
#include "dcClassTemplate.h"
#include "dcError.h"
#include "dcMainClass.h"
#include "dcNode.h"
#include "dcObjectClass.h"
#include "dcScope.h"
#include "dcStringEvaluator.h"
#include "dcSystem.h"

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcMainClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (MAIN_PACKAGE_NAME,            // package name
          MAIN_CLASS_NAME,              // class name
          MAKE_FULLY_QUALIFIED(OBJECT), // super name
          (CLASS_SINGLETON
           | CLASS_ABSTRACT
           | CLASS_FINAL),              // class flags
          NO_FLAGS,                     // scope data flags
          NULL,                         // methods
          NULL,                         // meta methods
          &dcMainClass_initialize,      // initialization function
          NULL,                         // deinitialization function
          NULL,                         // allocate
          NULL,                         // deallocate
          NULL,                         // meta mark
          NULL,                         // mark
          NULL,                         // copy
          NULL,                         // free
          NULL,                         // register
          NULL,                         // marshall
          NULL,                         // unmarshall
          NULL));                       // set template
};

static dcNode *sTheMain = NULL;

dcNode *dcMainClass_getInstance(void)
{
    return sTheMain;
}

void dcMainClass_resetInstance(void)
{
    sTheMain = dcScope_getObject(CAST_SCOPE(dcSystem_getGlobalScope()), "main");
}

void dcMainClass_initialize(void)
{
    dcError_assert(sTheMain == NULL);
    sTheMain = dcMainClass_createObject();
    dcSystem_addToGlobalScope(sTheMain, "main");
    dcClassManager_registerSingleton(sTheMain, "main");
}

dcNode *dcMainClass_createNode(bool _object)
{
    return dcClass_createBasicNode(sTemplate, _object);
}
