//
//  cx_http.h
//
//  Created by Ubaka Onyechi on 03/05/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_HTTP_H
#define CX_HTTP_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_HTTP_MAX_HEADER_FIELD_NAME_BUFFER   32
#define CX_HTTP_MAX_HEADER_FIELD_VALUE_BUFFER  96

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum cx_http_conn
{
  CX_HTTP_CONNECTION_OK,
  CX_HTTP_CONNECTION_ERROR,
} cx_http_conn;

#define CX_HTTP_REQUEST_ID_INVALID -1
typedef cxi32 cx_http_request_id;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_http_request_field
{
  const char *name;
  const char *value;
} cx_http_request_field;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_http_response
{
  cx_http_conn error;
  cxi32 statusCode;
  const void *data;
  cxi32 dataSize;
} cx_http_response;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*cx_http_response_callback) (cx_http_request_id tId, const cx_http_response *response, void *userdata);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_http_init (void);
bool _cx_http_deinit (void);

cx_http_request_id cx_http_get (const char *url, cx_http_request_field *headers, cxi32 headerCount, cxi32 timeout, 
                                cx_http_response_callback callback, void *userdata);

cx_http_request_id cx_http_post (const char *url, const void *postdata, cxi32 postdataSize, 
                                 cx_http_request_field *headers, cxi32 headerCount, cxi32 timeout, 
                                 cx_http_response_callback callback, void *userdata);

void cx_http_cancel (cx_http_request_id requestId);

cxu32 cx_http_percent_encode (char *dst, cxu32 dstSize, const char *src);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
