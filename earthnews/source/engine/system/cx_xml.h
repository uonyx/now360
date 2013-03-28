//
//  cx_xml.h
//
//  Created by Ubaka Onyechi on 28/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_XML_H
#define CX_XML_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void * cx_xml_doc;
typedef void * cx_xml_node;
typedef void * cx_xml_attr;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_xml_doc   cx_xml_doc_create (const char *data, cxu32 dataSize);
void         cx_xml_doc_destroy (cx_xml_doc doc);
cx_xml_node  cx_xml_doc_root_node (cx_xml_doc doc);

cx_xml_node  cx_xml_node_child (cx_xml_node node, const char *name, const char *nsprefix);
cx_xml_node  cx_xml_node_first_child (cx_xml_node node);
cx_xml_node  cx_xml_node_last_child (cx_xml_node node);
cx_xml_node  cx_xml_node_next_sibling (cx_xml_node node);
cx_xml_node  cx_xml_node_parent (cx_xml_node node);
const char * cx_xml_node_name (cx_xml_node node);
char *       cx_xml_node_content (cx_xml_node node);
char *       cx_xml_node_attr (cx_xml_node node, const char *name);

cx_xml_attr  cx_xml_attr_get_next_sibling (cx_xml_attr attr);
const char * cx_xml_attr_get_name (cx_xml_attr attr);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
