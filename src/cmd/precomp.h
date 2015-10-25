#ifndef __CMD_PRECOMP_H
#define __CMD_PRECOMP_H

#define _WIN32_WINNT  0x0500

#ifdef _MSC_VER
#pragma warning ( disable : 4103 ) /* use #pragma pack to change alignment */
#undef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif//_MSC_VER

#define  ULONG_PTR  ULONG 

typedef long NTSTATUS;

#include <stdlib.h>
#include <malloc.h>
//#define WIN32_NO_STATUS
#include <windows.h>
#include <winnt.h>
#include <shellapi.h>

#include <tchar.h>
#include <direct.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

#if 0 
#define NTOS_MODE_USER
#include <ndk/ntndk.h>
#endif // Lokie/2012-03-11 


#include "resource.h"

#include "cmd.h"
#include "config.h"
#include "batch.h"

#if 0 
#include <reactos/buildno.h>
#include <reactos/version.h>
#endif // Lokie/2012-03-11 

#if 0 
#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(cmd);
#ifdef UNICODE
#define debugstr_aw debugstr_w
#else
#define debugstr_aw debugstr_a
#endif
#endif // Lokie/2012-03-11 

#define DECLSPEC_NOINLINE 

#define  TRACE 
#define  debugstr_aw(x)   x 

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

#define BELOW_NORMAL_PRIORITY_CLASS       0x00004000
#define ABOVE_NORMAL_PRIORITY_CLASS       0x00008000

#define  KERNEL_RELEASE_STR "" 
#define  KERNEL_VERSION_BUILD_STR "" 
#define  COPYRIGHT_YEAR "" 
#define  WARN(x) 
 
#endif /* __CMD_PRECOMP_H */
