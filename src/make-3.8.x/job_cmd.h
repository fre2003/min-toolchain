#ifndef JOB_CMD_H
#define JOB_CMD_H

/* Default shell to use.  */
#ifdef WINDOWS32
#include <windows.h>
//#include <shlwapi.h>
#include "tchar.h"
//#pragma comment(lib, "shlwapi.lib")
#endif 


typedef struct
{
	char* name;
	int    flags;
	int    (*func)(char*, char*);
} COMMAND_STRU;

BOOL ExecuteCmd(CHAR* pszCmdName, int cmd_len, CHAR* pszCmdParam, int* pErrNo);
COMMAND_STRU* is_builtin_cmd(char* argv, int* perror, char cmd[MAX_PATH], OUT char** cmd_param);

// change '/' to '\\' and remove all trailing  slash and blanks.
TCHAR* PathFormatEx(CONST TCHAR *szPath, TCHAR* out);

#endif // _H 