//
//  cx_json.h
//
//  Created by Ubaka Onyechi on 02/03/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

#ifndef CX_JSON_H
#define CX_JSON_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  CX_JSON_TYPE_INVALID,
  CX_JSON_TYPE_BOOL,
  CX_JSON_TYPE_INT,
  CX_JSON_TYPE_FLOAT,
  CX_JSON_TYPE_STRING,
  CX_JSON_TYPE_OBJECT,
  CX_JSON_TYPE_ARRAY
} cx_json_type;

typedef void * cx_json_tree;
typedef void * cx_json_node;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_json_tree cx_json_tree_create (const char *data, cxu32 size);
void         cx_json_tree_destroy (cx_json_tree tree);
cx_json_node cx_json_tree_root_node (cx_json_tree tree);

cx_json_type cx_json_node_type (cx_json_node node);
cx_json_node cx_json_object_child (cx_json_node node, const char * CX_RESTRICT name);
cx_json_node cx_json_array_member (cx_json_node node, cxu32 index);
cxu32        cx_json_array_size (cx_json_node node);

const char * cx_json_value_string (cx_json_node node);
cxi64        cx_json_value_int (cx_json_node node);
cxf32        cx_json_value_float (cx_json_node node);
bool         cx_json_value_bool (cx_json_node node);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////