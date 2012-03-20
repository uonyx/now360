//
//  cx_debug.h
//  earthnews
//
//  Created by Ubaka  Onyechi on 04/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#ifndef CX_DEBUG_H
#define CX_DEBUG_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_defines.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// defines
#define CX_DEBUG                    (DEBUG)
#define CX_RELEASE                  (!DEBUG)
#define CX_DEBUG_LOG_ENABLE         (CX_DEBUG && 1)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// asserts
#if CX_DEBUG
#define CX_ASSERT(X)                do { if (!(X)) { cx_assert (__FILE__, __LINE__, #X); } } while (0)
#define CX_FATAL_ASSERT(X)          CX_ASSERT(X)
#define CX_FATAL_ERROR(X)           cx_fatal_error (__FILE__, __LINE__, #X) 
#define CX_DEBUG_BREAK              cx_debug_break ()
#define CX_DEBUG_BREAK_ABLE         int brk = 0; brk += 0;
#else
#define CX_ASSERT(X) 
#define CX_FATAL_ASSERT(X)
#define CX_FATAL_ERROR(X) 
#define CX_DEBUG_BREAK
#define CX_DEBUG_BREAK_ABLE
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// logs

#if CX_DEBUG_LOG_ENABLE
#define CX_OUTPUTLOG_CONSOLE(C, ...)  do { if (C) { cx_outputLog(__FILE__, __LINE__, __VA_ARGS__); } } while (0)
#else
#define CX_OUTPUTLOG_CONSOLE(C, ...)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// miscellany
#define CX_REFERENCE_UNUSED_VARIABLE(X) (void)(X)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_outputLog (const char *file, int line, const char *format, ...);
void cx_debug_break (void);

void cx_assert (const char *filename, int lineNumber, const char *assertString);
void cx_fatal_error (const char *filename, int lineNumber, const char *fatalString);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
