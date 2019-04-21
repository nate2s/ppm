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

#ifndef __DC_PROCEDURE_CLASS_H__
#define __DC_PROCEDURE_CLASS_H__

#include "dcDefines.h"

/////////////////////////
// dcProcedureClassAux //
/////////////////////////

struct dcProcedureClassAux_t
{
    struct dcNode_t *body;
    struct dcMethodHeader_t *methodHeader;
};

typedef struct dcProcedureClassAux_t dcProcedureClassAux;

//////////////////////
// dcProcedureClass //
//////////////////////

// creating //
struct dcNode_t *dcProcedureClass_createNode(struct dcNode_t *_body,
                                             struct dcMethodHeader_t *_header,
                                             bool _object);

struct dcNode_t *dcProcedureClass_createObject
    (struct dcNode_t *_body, struct dcMethodHeader_t *_header);

// getting //
struct dcMethodHeader_t *dcProcedureClass_getMethodHeader
    (const struct dcNode_t *_procedure);

struct dcNode_t *dcProcedureClass_getBody(const struct dcNode_t *_procedure);

dcScopeDataFlags dcProcedureClass_getAttributes
    (const struct dcNode_t *_procedure);

bool dcProcedureClass_isMe(const struct dcNode_t *_node);

// setting //
void dcProcedureClass_setBody(struct dcNode_t *_procedureNode,
                              struct dcNode_t *_body);

// standard functions //
ALLOCATE_FUNCTION(dcProcedureClass_allocateNode);
COPY_FUNCTION(dcProcedureClass_copyNode);
DEALLOCATE_FUNCTION(dcProcedureClass_deallocateNode);
FREE_FUNCTION(dcProcedureClass_freeNode);
GET_TEMPLATE_FUNCTION(dcProcedureClass_getTemplate);
REGISTER_FUNCTION(dcProcedureClass_registerNode);
MARK_FUNCTION(dcProcedureClass_markNode);
MARSHALL_FUNCTION(dcProcedureClass_marshallNode);
UNMARSHALL_FUNCTION(dcProcedureClass_unmarshallNode);

// taffy c methods
TAFFY_C_METHOD(dcProcedureClass_asString);

#define PROCEDURE_PACKAGE_NAME CORE_PACKAGE_NAME
#define PROCEDURE_CLASS_NAME "Procedure"

#endif
