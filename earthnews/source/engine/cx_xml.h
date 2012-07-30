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

void cx_xml_doc_create (cx_xml_doc *doc, const char *data, cxu32 dataSize);
void cx_xml_doc_destroy (cx_xml_doc *doc);

cx_xml_node cx_xml_doc_get_root_node (cx_xml_doc doc);

cx_xml_node cx_xml_node_get_child (cx_xml_node node, const char *name);
cx_xml_node cx_xml_node_get_first_child (cx_xml_node node);
cx_xml_node cx_xml_node_get_last_child (cx_xml_node node);
cx_xml_node cx_xml_node_get_next_sibling (cx_xml_node node);
cx_xml_node cx_xml_node_get_parent (cx_xml_node node);
const char *cx_xml_node_get_name (cx_xml_node node);
const char *cx_xml_node_get_content (cx_xml_node node);
cx_xml_attr cx_xml_node_get_attr (cx_xml_node node);

cx_xml_attr cx_xml_attr_get_next_sibling (cx_xml_attr attr);
const char *cx_xml_attr_get_name (cx_xml_attr attr);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif