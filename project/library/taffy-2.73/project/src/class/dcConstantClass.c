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
#include "dcConstantClass.h"
#include "dcIdentifier.h"
#include "dcNode.h"
#include "dcNumberClass.h"
#include "dcObjectClass.h"
#include "dcScopeData.h"
#include "dcTaffyCMethodWrapper.h"

static const dcTaffyCMethodWrapper sMethodWrappers[] =
{
    {
        "#operator(+):",
        SCOPE_DATA_PUBLIC,
        &dcConstantClass_add,
        gCFunctionArgument_wild
    },
    {
        "#operator(-):",
        SCOPE_DATA_PUBLIC,
        &dcConstantClass_subtract,
        gCFunctionArgument_wild
    },
    {
        "#operator(*):",
        SCOPE_DATA_PUBLIC,
        &dcConstantClass_multiply,
        gCFunctionArgument_wild
    },
    {
        "#operator(/):",
        SCOPE_DATA_PUBLIC,
        &dcConstantClass_divide,
        gCFunctionArgument_wild
    },
    {
        "#operator(^):",
        SCOPE_DATA_PUBLIC,
        &dcConstantClass_raise,
        gCFunctionArgument_wild
    },
    {
        "#operator(%):",
        SCOPE_DATA_PUBLIC,
        &dcConstantClass_modulus,
        gCFunctionArgument_wild
    },
    {
        0
    }
};

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcConstantClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (CONSTANT_PACKAGE_NAME,               // package name
          CONSTANT_CLASS_NAME,                 // class name
          MAKE_FULLY_QUALIFIED(OBJECT),        // super name
          NO_FLAGS,                            // class flags
          NO_FLAGS,                            // scope data flags
          NULL,                                // meta methods
          sMethodWrappers,                     // methods
          NULL,                                // initialization function
          NULL,                                // deinitialization function
          NULL,                                // allocate
          NULL,                                // deallocate
          NULL,                                // mark
          NULL,                                // copy
          NULL,                                // free
          NULL,                                // register
          NULL,                                // print
          NULL,                                // marshall
          NULL,                                // unmarshall
          NULL));                              // set template
};

dcNode *dcConstantClass_createNode(bool _object)
{
    return dcClass_createBasicNode(sTemplate, _object);
}

dcNode *dcConstantClass_derive(dcNode *_receiver, dcArray *_arguments)
{
    return dcNumberClass_getZeroNumberObject();
}

dcNode *dcConstantClass_add(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

dcNode *dcConstantClass_subtract(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

dcNode *dcConstantClass_multiply(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

dcNode *dcConstantClass_divide(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

dcNode *dcConstantClass_raise(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}

dcNode *dcConstantClass_modulus(dcNode *_receiver, dcArray *_arguments)
{
    return _receiver;
}
