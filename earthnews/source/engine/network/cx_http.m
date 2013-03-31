//
//  cx_http.c
//  earthnews
//
//  Created by Ubaka Onyechi on 03/05/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#import "cx_http.h"
#import "../system/cx_string.h"
#import "../system/cx_math.h"
#import <Foundation/Foundation.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_HTTP_DEBUG_LOG_ENABLED   1
#define CX_HTTP_MAX_NUM_NSCONN      16
#define CX_HTTP_CACHE_CUSTOM        1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_http_request
{
  const char *url;
  cx_http_request_field *headers;
  cxu32 headerCount;
  cxu32 timeout;
  void *nsconn;
} cx_http_request;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface CXNSURLConnection : NSObject <NSURLConnectionDelegate>
{
  NSURLConnection *conn;
  NSMutableData *respdata;
  cx_http_request_id rId;
  cx_http_response resp;
  cx_http_response_callback callback;
  void *callbackUserdata;
}

@property cx_http_request_id rId;
@property cx_http_response_callback callback;
@property void *callbackUserdata;
@property (nonatomic, retain) NSURLConnection *conn;

- (id)init;
- (id)initWith:(cx_http_request_id)transactionId :(cx_http_response_callback)responseCallback :(void *)userdata;
- (void)dealloc;

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool s_initialised = false;
static unsigned int s_requesrIdFactory = 0;
static NSMutableArray *s_nsconnFreeList = nil;
static NSMutableArray *s_nsconnBusyList = nil;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_http_init (cxu32 cacheMemSizeMb, cxu32 cacheDiskSizeMb, bool clearCache)
{
  CX_ASSERT (!s_initialised);
  
  s_requesrIdFactory = 0;
  
  s_nsconnFreeList = [[NSMutableArray alloc] initWithCapacity:CX_HTTP_MAX_NUM_NSCONN];
  s_nsconnBusyList = [[NSMutableArray alloc] initWithCapacity:CX_HTTP_MAX_NUM_NSCONN];
  
  for (cxu32 i = 0; i < CX_HTTP_MAX_NUM_NSCONN; ++i)
  {
    CXNSURLConnection *nsconn = [[CXNSURLConnection alloc] init];
    [s_nsconnFreeList addObject:nsconn];
  }
  
#if CX_HTTP_CACHE_CUSTOM
  if ((cacheMemSizeMb > 0) && (cacheDiskSizeMb > 0))
  {
    cxu32 cacheMemory  = 1024 * 1024 * cacheMemSizeMb;
    cxu32 cacheStorage = 1024 * 1024 * cacheDiskSizeMb;
    
    [NSURLCache setSharedURLCache:[[NSURLCache alloc] initWithMemoryCapacity:cacheMemory
                                                                diskCapacity:cacheStorage
                                                                    diskPath:@"cx_http"]];
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (CX_HTTP_DEBUG_LOG_ENABLED, "cx_http: Warning: using default system cache");
  }
#else
  cxu32 sharedCacheMemory = [[NSURLCache sharedURLCache] memoryCapacity];
  cxu32 sharedCacheDisk = [[NSURLCache sharedURLCache] diskCapacity];
  
  CX_DEBUGLOG_CONSOLE (CX_HTTP_DEBUG_LOG_ENABLED && 1, "shared memory cache size: %u", sharedCacheMemory);
  CX_DEBUGLOG_CONSOLE (CX_HTTP_DEBUG_LOG_ENABLED && 1, "shared disk cache size: %u", sharedCacheDisk);
  
  cxu32 newCacheMemory = cx_min (sharedCacheMemory, (1024 * 1024 * cacheMemSizeMb));
  cxu32 newCacheDisk = cx_max (sharedCacheDisk, (1024 * 1024 * cacheDiskSizeMb));
  
  [[NSURLCache sharedURLCache] setMemoryCapacity:newCacheMemory];
  [[NSURLCache sharedURLCache] setDiskCapacity:newCacheDisk];
#endif
  
  if (clearCache)
  {
    [[NSURLCache sharedURLCache] removeAllCachedResponses];
  }
  
  s_initialised = true;
  
  return s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_http_deinit (void)
{
  CX_ASSERT (s_initialised);
  
#if CX_HTTP_CACHE_CUSTOM
  NSURLCache *sharedCache = [NSURLCache sharedURLCache];
  [sharedCache release];
#endif
  
  for (cxu32 i = 0; i < [s_nsconnFreeList count]; ++i)
  {
    CXNSURLConnection *nsconn = [s_nsconnFreeList objectAtIndex:i];
    
    [s_nsconnFreeList removeObject:nsconn];
    
    [nsconn release];
  }
  
  for (cxu32 i = 0; i < [s_nsconnBusyList count]; ++i)
  {
    CXNSURLConnection *nsconn = [s_nsconnBusyList objectAtIndex:i];
    
    [s_nsconnBusyList removeObject:nsconn];
    
    [nsconn release];
  }
  
  [s_nsconnFreeList release];
  [s_nsconnBusyList release];
  
  s_initialised = false;
  
  return !s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_http_request_id cx_http_get (const char *url, cx_http_request_field *headers, int headerCount, int timeout, 
                                cx_http_response_callback callback, void *userdata)
{
  CX_ASSERT (s_nsconnFreeList);
  CX_ASSERT (s_nsconnBusyList);
  CX_ASSERT (url);
  
  cx_http_request_id rId = s_requesrIdFactory++;
  
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
  
  CXNSURLConnection *nsconn = [s_nsconnFreeList objectAtIndex:0];
  CX_ASSERT (nsconn);
  
  [nsconn setRId:rId];
  [nsconn setCallback:callback];
  [nsconn setCallbackUserdata:userdata];
  
  [s_nsconnFreeList removeObject:nsconn];
  [s_nsconnBusyList addObject:nsconn];
  
  NSURLConnection *conn = [NSURLConnection connectionWithRequest:nsrequest delegate:nsconn];
  
  [nsconn setConn:conn];
  
  return rId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_http_request_id cx_http_post (const char *url, const void *postdata, cxi32 postdataSize, cx_http_request_field *headers, 
                               cxi32 headerCount, cxi32 timeout, cx_http_response_callback callback, void *userdata)
{
  CX_ASSERT (s_nsconnFreeList);
  CX_ASSERT (s_nsconnBusyList);
  
  cx_http_request_id rId = s_requesrIdFactory++;
  
  NSURL *nsurl = [NSURL URLWithString:[NSString stringWithCString:url encoding:NSASCIIStringEncoding]];
  NSMutableURLRequest *nsrequest = [NSMutableURLRequest requestWithURL:nsurl 
                                                           cachePolicy:NSURLRequestUseProtocolCachePolicy 
                                                       timeoutInterval:timeout];  
  NSData *body = [NSData dataWithBytes:postdata length:postdataSize];
  
  [nsrequest setHTTPMethod:@"POST"];
  [nsrequest setHTTPBody:body];  
  
  cx_http_request_field reqfield [] = { {"user-agent", "earthnews"}, {"referer", "jack"}};
  cxu32 reqfieldCount = sizeof (reqfield) / sizeof (cx_http_request_field);
  
  for (cxu32 i = 0; i < reqfieldCount; ++i)
  {
    NSString *name = [NSString stringWithCString:reqfield [i].name encoding:NSASCIIStringEncoding];
    NSString *value = [NSString stringWithCString:reqfield [i].value encoding:NSASCIIStringEncoding];
    
    [nsrequest setValue:value forHTTPHeaderField:name];
  }
  
  if (headers)
  {
    for (cxi32 i = 0; i < headerCount; ++i)
    {
      NSString *name = [NSString stringWithCString:headers [i].name encoding:NSASCIIStringEncoding];
      NSString *value = [NSString stringWithCString:headers [i].value encoding:NSASCIIStringEncoding];
      
      [nsrequest setValue:value forHTTPHeaderField:name];
    }
  }
  
  CXNSURLConnection *nsconn = [s_nsconnFreeList objectAtIndex:0];
  CX_ASSERT (nsconn);
  
  [nsconn setRId:rId];
  [nsconn setCallback:callback];
  [nsconn setCallbackUserdata:userdata];
  
  [s_nsconnFreeList removeObject:nsconn];
  [s_nsconnBusyList addObject:nsconn];
  
  NSURLConnection *conn = [NSURLConnection connectionWithRequest:nsrequest delegate:nsconn];
  
  [nsconn setConn:conn];
  
  return rId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_http_cancel (cx_http_request_id requestId)
{
  CX_ASSERT (requestId > CX_HTTP_REQUEST_ID_INVALID);
  
  for (cxu32 i = 0, c = [s_nsconnBusyList count]; i < c; ++i)
  {
    CXNSURLConnection *nsconn = [s_nsconnBusyList objectAtIndex:i];
    
    if (nsconn.rId == requestId)
    {
      [nsconn.conn cancel];
      
      [s_nsconnBusyList removeObject:nsconn];
      [s_nsconnFreeList addObject:nsconn];
      
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_http_clear_cache (void)
{
  [[NSURLCache sharedURLCache] removeAllCachedResponses];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation CXNSURLConnection

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@synthesize conn;
@synthesize rId;
@synthesize callback;
@synthesize callbackUserdata;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (id)initWith: (cx_http_request_id)transactionId :(cx_http_response_callback)responseCallback :(void *)userdata;
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
  //[self->conn cancel];
  [self->conn release];
  [self->respdata release];
  [super dealloc];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
  CX_DEBUGLOG_CONSOLE (CX_HTTP_DEBUG_LOG_ENABLED, "cx_http: didFailWithError: Connection error: Internet offline maybe");
  
  self->resp.error = CX_HTTP_CONNECTION_ERROR;
  self->resp.statusCode = -1;
  self->resp.data = NULL;
  self->resp.dataSize = 0;
  
  cx_http_response_callback responseCallback = self->callback;
  
  if (responseCallback)
  {
    responseCallback (self->rId, &self->resp, self->callbackUserdata);
  }
  
  [self setRId:CX_HTTP_REQUEST_ID_INVALID];
  [self setCallback:NULL];
  [self setCallbackUserdata:NULL];
  
  [s_nsconnBusyList removeObject:self];
  [s_nsconnFreeList addObject:self];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)connectionDidFinishLoading:(NSURLConnection *) connection
{
  NSMutableData *data = self->respdata;
  
  self->resp.data = [data bytes];
  self->resp.dataSize = [data length];
  
  cx_http_response_callback responseCallback = self->callback;
  
  if (responseCallback)
  {
    responseCallback (self->rId, &self->resp, self->callbackUserdata);
  }
  
  [data resetBytesInRange:NSMakeRange(0, [data length])];
  [data setLength:0];
  
  [self setRId:CX_HTTP_REQUEST_ID_INVALID];
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
  
  self->resp.error = CX_HTTP_CONNECTION_OK;
  self->resp.statusCode = statusCode;
  
  CX_DEBUGLOG_CONSOLE (CX_HTTP_DEBUG_LOG_ENABLED, "cx_http: didReceiveResponse: HTTP request: Server Response Status Code [%d]", statusCode);
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

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
