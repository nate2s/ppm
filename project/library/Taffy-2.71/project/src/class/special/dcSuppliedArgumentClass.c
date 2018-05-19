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

#include "dcClass.h"
#include "dcClassTemplate.h"
#include "dcObjectClass.h"
#include "dcTaffyCMethodWrapper.h"
#include "dcSuppliedArgumentClass.h"

static dcClassTemplate *sTemplate = NULL;

dcClassTemplate *dcSuppliedArgumentClass_getTemplate(void)
{
    CLASS_TEMPLATE_SINGLETON_HELPER
        (sTemplate,
         dcClassTemplate_create
         (SUPPLIED_ARGUMENT_PACKAGE_NAME,      // package name
          SUPPLIED_ARGUMENT_CLASS_NAME,        // class name
          MAKE_FULLY_QUALIFIED(OBJECT),        // super name
          (CLASS_ABSTRACT | CLASS_FINAL),      // class flags
          NO_FLAGS,                            // scope data flags
          NULL,                                // meta methods
          NULL,                                // methods
          NULL,                                // initialization function
          NULL,                                // deinitialization function
          NULL,                                // allocate
          NULL,                                // deallocate
          NULL,                                // meta mark
          NULL,                                // mark
          NULL,                                // copy
          NULL,                                // free
          NULL,                                // register
          NULL,                                // marshall
          NULL,                                // unmarshall
          NULL));                              // set template
}
