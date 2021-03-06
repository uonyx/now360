//
//  analytics.m
//
//  Copyright (c) 2013 Ubaka Onyechi. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "cx_analytics.h"
#import "../system/cx_math.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CX_ANALYTICS_FLURRY_ENABLED
#import "../3rdparty/flurry/Flurry.h"
#define FLURRY_MAX_PARAM_COUNT 10
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool g_session = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_analytics_session_begin (const cx_analytics_session_data *data)
{
  CX_ASSERT (!g_session);
  CX_ASSERT (data);
  
#if CX_ANALYTICS_FLURRY_ENABLED
  [Flurry setCrashReportingEnabled:TRUE];
  [Flurry startSession:[NSString stringWithCString:data->flurry.apiKey encoding:NSASCIIStringEncoding]];
  
#if CX_DEBUG
  [Flurry setDebugLogEnabled:TRUE];
  [Flurry setEventLoggingEnabled:TRUE];
#endif
  
#endif
  
  g_session = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_analytics_session_end (void)
{
  CX_ASSERT (g_session);
  
#if CX_ANALYTICS_FLURRY_ENABLED
  // nothing to do here
#endif
  
  g_session = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_analytics_set_identifier (const char *identifier)
{
  CX_ASSERT (g_session);
  CX_ASSERT (identifier);
  
#if CX_ANALYTICS_FLURRY_ENABLED
  [Flurry setUserID:[NSString stringWithCString:identifier encoding:NSASCIIStringEncoding]];
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_analytics_log_event (const char *event, const char **paramNames, const char **paramValues, int paramCount)
{
  CX_ASSERT (g_session);
  CX_ASSERT (event);
  
#if CX_ANALYTICS_FLURRY_ENABLED
  NSString *nsEvent = [NSString stringWithCString:event encoding:NSASCIIStringEncoding];
  NSDictionary *nsParams = nil;
  
  if (paramCount > 0)
  {
    CX_ASSERT (paramNames);
    CX_ASSERT (paramValues);
    
    int pcount = cx_min (paramCount, FLURRY_MAX_PARAM_COUNT);
    
    NSString *pn [pcount];
    NSString *pv [pcount];
    
    for (int i = 0; i < pcount; ++i)
    {
      const char *cpn = paramNames [i];
      const char *cpv = paramValues [i];
      
      pn [i] = [NSString stringWithCString:cpn encoding:NSASCIIStringEncoding];
      pv [i] = [NSString stringWithCString:cpv encoding:NSASCIIStringEncoding];
    }
    
    nsParams = [NSDictionary dictionaryWithObjects:(const id *) pv forKeys:(const id *) pn count:pcount];
  }
  
  [Flurry logEvent:nsEvent withParameters:nsParams];
#endif
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
