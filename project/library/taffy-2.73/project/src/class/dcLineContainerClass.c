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

#include "CompiledLineContainer.h"
#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcContainerClass.h"
#include "dcLineContainerClass.h"
#include "dcNode.h"
#include "dcStringEvaluator.h"

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcLineContainerClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (LINE_CONTAINER_PACKAGE_NAME,      // package name
          LINE_CONTAINER_CLASS_NAME,        // class name
          "Container",                      // super name
          CLASS_ABSTRACT,                   // class flags
          NO_FLAGS,                         // scope data flags
          NULL,                             // meta methods
          NULL,                             // methods
          &dcLineContainerClass_initialize, // initialization function
          NULL,                             // deinitialization function
          NULL,                             // allocate
          NULL,                             // deallocate
          NULL,                             // meta mark
          NULL,                             // mark
          NULL,                             // copy
          NULL,                             // free
          NULL,                             // register
          NULL,                             // marshall
          NULL,                             // unmarshall
          NULL));                           // set template
};

void dcLineContainerClass_initialize(void)
{
    assert(dcStringEvaluator_evalString(__compiledLineContainer,
                                        "src/class/LineContainer.ty",
                                        NO_STRING_EVALUATOR_FLAGS)
           != NULL);
}

dcNode *dcLineContainerClass_createNode(bool _object)
{
    return dcClass_createNode(sTemplate,
                              NULL, // super
                              NULL, // scope
                              _object,
                              NULL);
}
