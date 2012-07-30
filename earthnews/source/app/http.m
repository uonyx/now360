//
//  http.c
//  earthnews
//
//  Created by Ubaka Onyechi on 03/05/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#import "http.h"
#import "../engine/cx_engine.h"
#import <Foundation/Foundation.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface HTTPNSConn : NSObject <NSURLConnectionDelegate>
{
  NSURLConnection *conn;
  NSMutableData *respdata;
  http_transaction_id tId;
  http_response resp;
  http_response_callback callback;
  void *callbackUserdata;
}

@property http_transaction_id tId;
@property http_response_callback callback;
@property void *callbackUserdata;
@property (nonatomic, retain) NSURLConnection *conn;

- (id)init;
- (id)initWith: (http_transaction_id)transactionId: (http_response_callback)responseCallback: (void *)userdata;
- (void)dealloc;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define HTTP_MAX_NUM_NSCONN 16

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned int s_transactionIdGen = 0;
static NSMutableArray *s_nsconnFreeList = nil;
static NSMutableArray *s_nsconnBusyList = nil;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void http_init (void)
{
  s_nsconnFreeList = [[NSMutableArray alloc] initWithCapacity:HTTP_MAX_NUM_NSCONN];
  s_nsconnBusyList = [[NSMutableArray alloc] initWithCapacity:HTTP_MAX_NUM_NSCONN];
  
  for (unsigned int i = 0; i < HTTP_MAX_NUM_NSCONN; ++i)
  {
    HTTPNSConn *nsconn = [[HTTPNSConn alloc] init];
    [s_nsconnFreeList addObject:nsconn];
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void http_deinit (void)
{
  for (unsigned int i = 0; i < [s_nsconnFreeList count]; ++i)
  {
    HTTPNSConn *nsconn = [s_nsconnFreeList objectAtIndex:i];
    
    [s_nsconnFreeList removeObject:nsconn];
    
    [nsconn release];
  }

  for (unsigned int i = 0; i < [s_nsconnBusyList count]; ++i)
  {
    HTTPNSConn *nsconn = [s_nsconnBusyList objectAtIndex:i];
    
    [s_nsconnBusyList removeObject:nsconn];
    
    [nsconn release];
  }
  
  [s_nsconnFreeList release];
  [s_nsconnBusyList release];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

http_transaction_id http_get (const char *url, http_request_field *headers, int headerCount, int timeout, 
                              http_response_callback callback, void *userdata)
{
  CX_ASSERT (url);
 
  CX_ASSERT (s_nsconnFreeList);
  CX_ASSERT (s_nsconnBusyList);
  
  http_transaction_id tId = s_transactionIdGen++;
  
  NSURL *nsurl = [NSURL URLWithString:[NSString stringWithCString:url encoding:NSASCIIStringEncoding]];
  NSMutableURLRequest *nsrequest = [NSMutableURLRequest requestWithURL:nsurl 
                                                           cachePolicy:NSURLRequestUseProtocolCachePolicy 
                                                       timeoutInterval:timeout];
  [nsrequest setHTTPMethod:@"GET"];
  
  if (headers)
  {
    for (int i = 0; i < headerCount; ++i)
    {
      NSString *name = [NSString stringWithCString:headers [i].name encoding:NSASCIIStringEncoding];
      NSString *value = [NSString stringWithCString:headers [i].value encoding:NSASCIIStringEncoding];
      
      [nsrequest setValue:value forHTTPHeaderField:name];
    }
  }
  
  HTTPNSConn *nsconn = [s_nsconnFreeList objectAtIndex:0];
  CX_ASSERT (nsconn);
  
  [nsconn setTId:tId];
  [nsconn setCallback:callback];
  [nsconn setCallbackUserdata:userdata];
  
  [s_nsconnFreeList removeObject:nsconn];
  [s_nsconnBusyList addObject:nsconn];
  
  NSURLConnection *conn = [NSURLConnection connectionWithRequest:nsrequest delegate:nsconn];
  
  [nsconn setConn:conn];
  
  return tId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

http_transaction_id http_post (const char *url, const void *postdata, int postdataSize, http_request_field *headers, 
                               int headerCount, int timeout, http_response_callback callback, void *userdata)
{
  CX_ASSERT (s_nsconnFreeList);
  CX_ASSERT (s_nsconnBusyList);
  
  http_transaction_id tId = s_transactionIdGen++;
  
  NSURL *nsurl = [NSURL URLWithString:[NSString stringWithCString:url encoding:NSASCIIStringEncoding]];
  NSMutableURLRequest *nsrequest = [NSMutableURLRequest requestWithURL:nsurl 
                                                           cachePolicy:NSURLRequestUseProtocolCachePolicy 
                                                       timeoutInterval:timeout];  
  NSData *body = [NSData dataWithBytes:postdata length:postdataSize];

  [nsrequest setHTTPMethod:@"POST"];
  [nsrequest setHTTPBody:body];  
  
  http_request_field reqfield [] = { {"user-agent", "earthnews"}, {"referer", "jack"}};
  unsigned int reqfieldCount = sizeof (reqfield) / sizeof (http_request_field);
  
  for (unsigned int i = 0; i < reqfieldCount; ++i)
  {
    NSString *name = [NSString stringWithCString:reqfield [i].name encoding:NSASCIIStringEncoding];
    NSString *value = [NSString stringWithCString:reqfield [i].value encoding:NSASCIIStringEncoding];
    
    [nsrequest setValue:value forHTTPHeaderField:name];
  }
  
  if (headers)
  {
    for (int i = 0; i < headerCount; ++i)
    {
      NSString *name = [NSString stringWithCString:headers [i].name encoding:NSASCIIStringEncoding];
      NSString *value = [NSString stringWithCString:headers [i].value encoding:NSASCIIStringEncoding];
      
      [nsrequest setValue:value forHTTPHeaderField:name];
    }
  }
  
  HTTPNSConn *nsconn = [s_nsconnFreeList objectAtIndex:0];
  CX_ASSERT (nsconn);
  
  [nsconn setTId:tId];
  [nsconn setCallback:callback];
  [nsconn setCallbackUserdata:userdata];
  
  [s_nsconnFreeList removeObject:nsconn];
  [s_nsconnBusyList addObject:nsconn];
  
  NSURLConnection *conn = [NSURLConnection connectionWithRequest:nsrequest delegate:nsconn];
  
  [nsconn setConn:conn];
  
  return tId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int http_percent_encode (char *dst, unsigned int dstSize, const char *src)
{
  CX_ASSERT (src);
  CX_ASSERT (dst);
  CX_ASSERT (dstSize);
  
  unsigned int len = 0;
  char c = 0;
  char enc [16];
  const char *s = src;
  
  while ((c = *s))
  {
    // unreserved characters
    if (((c >= '0') && (c <= '9')) ||
        ((c >= 'a') && (c <= 'z')) ||
        ((c >= 'A') && (c <= 'Z')) ||
        ((c == '-') || (c == '.') || (c == '_') || (c == '~')))
    {
      CX_ASSERT (len < dstSize);
      dst [len++] = c;
    }
    else // ascii control characters, reserved characters, unsafe characters
    {
      int enc_size = cx_sprintf (enc, 16, "%%%02x", c);
#if 1
      for (int i = 0; i < enc_size; ++i)
      {
        CX_ASSERT (len < dstSize);
        dst [len++] = enc [i];
      }
#else
      CX_ASSERT ((len + enc_size) < dstSize); CX_REFERENCE_UNUSED_VARIABLE (enc_size);
      dst [len++] = enc [0];
      dst [len++] = enc [1];
      dst [len++] = enc [2];
#endif
    }
    
    s++;
  }
 
  CX_ASSERT (len < dstSize);
  dst [len] = 0;
  
  return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation HTTPNSConn

@synthesize conn;
@synthesize tId;
@synthesize callback;
@synthesize callbackUserdata;

- (id)init
{
  self = [super init];
  
  if (self) 
  {
    self->conn = [[NSURLConnection alloc] init];
    self->respdata = [[NSMutableData alloc] initWithCapacity:512];
  }
  
  return self;
}

- (id)initWith: (http_transaction_id)transactionId: (http_response_callback)responseCallback: (void *)userdata;
{
  self = [super init];
  
  if (self)
  {
    self->conn = [[NSURLConnection alloc] init];
    self->respdata = [[NSMutableData alloc] initWithCapacity:512];
    self->callback = responseCallback;
    self->callbackUserdata = userdata;
  }
  
  return self;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)dealloc
{
  [self->conn release];
  [self->respdata release];
  [super dealloc];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)connectionDidFinishLoading:(NSURLConnection *) connection
{
  NSMutableData *data = self->respdata;
  
  self->resp.data = [data bytes];
  self->resp.dataSize = [data length];
  
  http_response_callback responseCallback = self->callback;
  
  if (responseCallback)
  {
    responseCallback (self->tId, &self->resp, self->callbackUserdata);
  }
  
  [data resetBytesInRange:NSMakeRange(0, [data length])];
  [data setLength:0];
  
  [self setTId:HTTP_TRANSACTION_ID_INVALID];
  [self setCallback:NULL];
  [self setCallbackUserdata:NULL];
  
  [s_nsconnBusyList removeObject:self];
  [s_nsconnFreeList addObject:self];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
  NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) response;  
  NSInteger statusCode = [httpResponse statusCode];
  
  self->resp.error = HTTP_CONNECTION_OK;
  self->resp.statusCode = statusCode;
  
  CX_DEBUGLOG_CONSOLE (1, "didReceiveResponse: HTTP request: Server Response Status Code [%d]", statusCode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
  [self->respdata appendData:data];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
  CX_DEBUGLOG_CONSOLE (1, "didFailWithError: Connection error: Internet offline maybe");
  
  self->resp.error = HTTP_CONNECTION_ERROR;
  self->resp.statusCode = -1;
  self->resp.data = NULL;
  self->resp.dataSize = 0;
  
  http_response_callback responseCallback = self->callback;
  
  if (responseCallback)
  {
    responseCallback (self->tId, &self->resp, self->callbackUserdata);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


@end