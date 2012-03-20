//
//  cx_debug.c
//  earthnews
//
//  Created by Ubaka  Onyechi on 04/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#include "cx_debug.h"

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sysctl.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static BOOL in_debugger (void)
{
  // Reference: http://developer.apple.com/library/ios/#qa/qa1361/_index.html
  
  // Returns true if the current process is being debugged (either 
  // running under the debugger or has a debugger attached post facto).
  
  int                 junk;
  int                 mib[4];
  struct kinfo_proc   info;
  size_t              size;
  
  // Initialize the flags so that, if sysctl fails for some bizarre 
  // reason, we get a predictable result.
  
  info.kp_proc.p_flag = 0;
  
  // Initialize mib, which tells sysctl the info we want, in this case
  // we're looking for information about a specific process ID.
  
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PID;
  mib[3] = getpid();
  
  // Call sysctl.
  
  size = sizeof(info);
  junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
  CX_ASSERT (junk == 0);
  
  // We're being debugged if the P_TRACED flag is set.
  
  return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_outputLog (const char *file, int line, const char *format, ...)
{
  va_list arg_list;
  
  fprintf (stderr, "%s %d:", file, line);
  va_start (arg_list, format);
  vfprintf (stderr, format, arg_list);
  va_end (arg_list);
  fprintf (stderr, "\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_debug_break (void)
{
  if (in_debugger ())
  {
    kill (getpid(), SIGINT);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_assert (const char *filename, int lineNumber, const char *assertString)
{
  cx_outputLog (filename, lineNumber, "%s", assertString);
  cx_debug_break ();
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_fatal_error (const char *filename, int lineNumber, const char *fatalString)
{
  cx_outputLog (filename, lineNumber, "%s", fatalString);
  cx_debug_break ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
