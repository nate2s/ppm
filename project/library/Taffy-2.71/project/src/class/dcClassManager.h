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

#ifndef __DC_CLASS_MANAGER_H__
#define __DC_CLASS_MANAGER_H__

#include "dcDefines.h"

// create the singleton //
void dcClassManager_create(void);

// free the singleton //
void dcClassManager_free(void);

// clear the templates //
void dcClassManager_clearClassTemplates(void);

// getting //
struct dcNode_t *dcClassManager_getClassFromPackage
    (const struct dcPackage_t *_classNamePackage,
     const struct dcPackage_t *_package,
     struct dcList_t *_packageContentsList,
     const struct dcClassTemplate_t *_requestorTemplate);

struct dcNode_t *dcClassManager_getClass
    (const char *_fullyQualifiedClassName,
     const struct dcPackage_t *_package,
     struct dcList_t *_packageContentsList,
     const struct dcClassTemplate_t *_requestorTemplate);

// convenience function //
struct dcClassTemplate_t *dcClassManager_getClassTemplate
    (const char *_fullyQualifiedClassName,
     const struct dcPackage_t *_package,
     struct dcList_t *_packageContentsList,
     const struct dcClassTemplate_t *_requestorTemplate);

struct dcNode_t *dcClassManager_getPackageContents
    (const struct dcPackage_t *_package);

// registering //
dcResult dcClassManager_registerClassTemplate
    (struct dcClassTemplate_t *_template,
     struct dcList_t *_packageContents,
     bool _initializePlease,
     struct dcNode_t **_metaResult);

void dcClassManager_addClassTemplateTemplate
    (struct dcClassTemplate_t *_template);

// marking //
void dcClassManager_mark(void);

// marshalling //
bool dcClassManager_unmarshall(struct dcString_t *_stream);
struct dcString_t *dcClassManager_marshall(struct dcString_t *_stream);

// displaying //
char *dcClassManager_displayClasses(void);

struct dcClassTemplate_t *dcClassManager_getClassTemplateFromId(uint16_t _id);

void dcClassManager_registerSingleton(struct dcNode_t *_node,
                                      const char *_name);
// returns dcIdentifier-node
struct dcNode_t *dcClassManager_getSingletonFromId(uint16_t _id);

#endif
