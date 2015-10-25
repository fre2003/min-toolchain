#include "make.h"
#include <assert.h>
#include "job_cmd.h"
#include "debug.h"
#include <string.h>

/* Default shell to use.  */
#ifdef WINDOWS32
#include <windows.h>
#include "tchar.h"
#endif 

static DWORD DeleteFilesEx(LPCTSTR FileName, DWORD dwFlags, int* pret);
extern char * getcwd_fs(char *buf, int len);
BOOL IsExistingDirectory (LPCTSTR pszPath);
BOOL IsExistingFile (LPCTSTR pszPath);
int MakeFullPath(TCHAR* DirPath);

BOOL add_entry(int* ac, char*** arg, const char* entry);

INT GetRootPath(TCHAR* InPath, TCHAR * OutPath, INT size)
{
  if (InPath[0] && InPath[1] == _T(':'))
  {
    INT t = 0;
    
    if ((InPath[0] >= _T('0')) && (InPath[0] <= _T('9')))
    {
      t = (InPath[0] - _T('0')) + 28;
    }
    
    if ((InPath[0] >= _T('a')) && (InPath[0] <= _T('z')))
    {
      t         = (InPath[0] - _T('a')) + 1;
      InPath[0] = t + _T('A') - 1;
    }
    
    if ((InPath[0] >= _T('A')) && (InPath[0] <= _T('Z')))
    {
      t = (InPath[0] - _T('A')) + 1;
    }
    
    return _tgetdcwd(t, OutPath, size) == NULL;
  }
  
  /* Get current directory */
  return! GetCurrentDirectory(size, OutPath);
}

__inline BOOL IsFileExist(LPCTSTR pszFile)
{
  if (GetFileAttributes((LPTSTR) pszFile) == 0xffffffff)  // Win32
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

VOID StripQuotes(TCHAR* in)
{
  TCHAR* out = in;
  for (; *in; in++)
  {
    if (*in != _T('"'))
      *out++ = *in;
  }
  *out = _T('\0');
}

/*
* freep -- frees memory used for a call to split
*
*/
VOID freep(LPTSTR* p)
{
  LPTSTR* q;
  
  if (!p)
    return;
  
  q = p;
  while (*q)
    free(*q++);
  
  free(p);
}

static BOOL expand(int* ac, char*** arg, const char* pattern)
{
  HANDLE hFind;
  WIN32_FIND_DATA FindData;
  BOOL ok;
  LPCTSTR pathend;
  LPTSTR dirpart, fullname;
  
  pathend = _tcsrchr(pattern, _T('\\'));
  if (NULL != pathend)
  {
    dirpart = malloc((pathend - pattern + 2) * sizeof(char));
    if (NULL == dirpart)
    {
      return FALSE;
    }
    memcpy(dirpart, pattern, pathend - pattern + 1);
    dirpart[pathend - pattern + 1] = _T('\0');
  }
  else
  {
    dirpart = NULL;
  }
  hFind = FindFirstFile(pattern, &FindData);
  if (INVALID_HANDLE_VALUE != hFind)
  {
    do
    {
      if (NULL != dirpart)
      {
        fullname = malloc((_tcslen(dirpart) + _tcslen(FindData.cFileName) + 1) * sizeof(TCHAR));
        if (NULL == fullname)
        {
          ok = FALSE;
        }
        else
        {
          _tcscat(_tcscpy(fullname, dirpart), FindData.cFileName);
          ok = add_entry(ac, arg, fullname);
          free(fullname);
        }
      }
      else
      {
        ok = add_entry(ac, arg, FindData.cFileName);
      }
    } while (FindNextFile(hFind, &FindData) && ok);
    FindClose(hFind);
  }
  else
  {
    ok = add_entry(ac, arg, pattern);
  }
  
  if (NULL != dirpart)
  {
    free(dirpart);
  }
  
  return ok;
}


/* add new entry for new argument */
BOOL add_entry(int* ac, char*** arg, const char* entry)
{
  LPTSTR q;
  
  LPTSTR* oldarg;
  
  q = malloc((_tcslen(entry) + 1) * sizeof(TCHAR));
  if (NULL == q)
  {
    return FALSE;
  }
  
  _tcscpy(q, entry);
  oldarg = *arg;
  *arg   = realloc(oldarg, (*ac + 2) * sizeof(LPTSTR));
  if (NULL == *arg)
  {
    *arg = oldarg;
    return FALSE;
  }
  
  /* save new entry */
  (*arg)[*ac] = q;
  (*arg)[++(*ac)] = NULL;
  
  return TRUE;
}


/*
 * split - splits a line up into separate arguments, deliminators
 *         are spaces and slashes ('/').
*/
LPTSTR* split(LPTSTR s, int* args, BOOL expand_wildcards)
{
  LPTSTR* arg;
  LPTSTR start;
  LPTSTR q;
  int ac;
  int len;
  
  arg = malloc(sizeof(LPTSTR));
  if (!arg)
    return NULL;

  *arg = NULL;
  
  ac = 0;
  while (*s)
  {
    BOOL bQuoted = FALSE;
    
    /* skip leading spaces */
    while (*s && (_istspace(*s) || _istcntrl(*s)))
      ++s;
    
    start = s;
    
    if (*s == _T('-'))
      ++s;
    
    /* skip to next word delimiter or start of next option */
    while (_istprint(*s) && (bQuoted || (!_istspace(*s) && *s != _T('-'))))
    {
      /* if quote (") then set bQuoted */
      bQuoted ^= (*s == _T('\"'));
      ++s;
    }

    /* a word was found */
    if (s != start)
    {
      q = malloc(((len = s - start) + 1) * sizeof(TCHAR));
      if (!q)
      {
        return NULL;
      }

      memcpy(q, start, len * sizeof(TCHAR));
      q[len] = _T('\0');

      StripQuotes(q);
      
      if (expand_wildcards && _T('/') != *start && (NULL != _tcschr(q, _T('*')) || NULL != _tcschr(q, _T('?'))))
      {
        if (!expand(&ac, &arg, q))
        {
          free(q);
          freep(arg);
          return NULL;
        }
      }
      else
      {
        // Unix TO WIN32 Path 
        if(q && (q[0] != '-'))
        {
          char* path = q;
          
          for (path = q; path && *path; path++) // WIN32
            if (*path == '/')
              *path = '\\';
        }
        
        if (!add_entry(&ac, &arg, q))
        {
          free(q);
          freep(arg);
          return NULL;
        }
      }
      free(q);
    }
  }
  
  *args = ac;
  
  return arg;
}


static DWORD ProcessDirectory(LPTSTR FileName, DWORD dwFlags, int* pret)
{
  TCHAR szFullPath[MAX_PATH] = {0, };
  HANDLE hFile;
  WIN32_FIND_DATA f;
  DWORD dwFiles = 0;
  LPSTR pFilePart = 0;

  GetFullPathName(FileName, MAX_PATH, szFullPath, &pFilePart);
  
  dwFiles = DeleteFilesEx(szFullPath, dwFlags, pret);
  if (dwFiles & 0x80000000)
  {
    return dwFiles;
  }
  
  if (dwFlags & DEL_SUBDIR)
  {
    TCHAR szFindPath[MAX_PATH] = {0, };
    
    GetFullPathName(FileName, MAX_PATH, szFullPath, NULL);

    sprintf(szFindPath, "%s\\*", szFullPath);
    hFile = FindFirstFile(szFindPath, &f);
    if (hFile != INVALID_HANDLE_VALUE)
    {
      do
      {
        if (!(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !_tcscmp(f.cFileName, _T(".")) || !_tcscmp(f.cFileName, _T("..")))
        {
          continue;
        }
        
        sprintf(szFindPath, "%s\\%s", szFullPath, f.cFileName);
        
        dwFiles += ProcessDirectory(szFindPath, dwFlags, pret);
        if(pret && *pret)
        {
          break;
        }
        if (dwFiles & 0x80000000)
        {
          break;
        }
      } while (FindNextFile(hFile, &f)) ;
      FindClose(hFile);
    }
  }
  return dwFiles;
}

#define SPLIT_PARAM(pszParam, argv, args) do {\
    char* __p  = strtok(pszParam, " ");\
    if(__p == 0) argv[args++] = __p;\
    while (__p != NULL)\
    {\
      if(args<20) argv[args++] = __p;\
      __p = strtok(NULL, " ");\
    }\
} while (0)

int cmd_del(CHAR* pszCmdName, CHAR* pszParam)
{
  int ret = 0;

  char* argv[20] = { 0, };
  INT args       = 0;

  INT i         = 0;
  DWORD dwFlags = 0;
  DWORD dwFiles = 0;
  LONG ch       = 0;
  TCHAR szOrginalArg[MAX_PATH] = { 0, };
  
  SPLIT_PARAM(pszParam, argv, args);
  
	if (args == 0)
	{
    fprintf(stderr,"delx: invalid input.\n");
		return 1;
	}
  
  for (i = 0; i < args; i++)
  {
    if (argv[i][0] == _T('/') || argv[i][0] == _T('-'))
    {
      /*found a command, but check to make sure it has something after it*/
      if (_tcslen (argv[i]) >= 2)
      {
        ch = _totupper (argv[i][1]);
        if (ch == _T('Q'))
        {
          dwFlags |= DEL_QUIET;
        }
        else if (ch == _T('S'))
        {
          dwFlags |= DEL_SUBDIR;
        }
        else if (ch == _T('F'))
        {
          dwFlags |= DEL_FORCE;
        }
      }
    }
  }
  
  /* check for filenames anywhere in command line */
  for (i = 0; i < args && !(dwFiles & 0x80000000); i++)
  {
    /* thischecks to see if it isnt a flag, if it isnt, we assume it is a file name */
    if ((argv[i][0] == _T('/')) || (argv[i][0] == _T('-')))
      continue;
    
    /* We want to make a copies of the argument */
    if (_tcslen(argv[i]) == 2 && argv[i][1] == _T(':'))
    {
      /* Check for C: D: ... */
      GetRootPath(argv[i], szOrginalArg, MAX_PATH);
    }
    else
    {
      _tcscpy(szOrginalArg, argv[i]);
    }
    
    dwFiles += ProcessDirectory(szOrginalArg, dwFlags, &ret);
    if(ret)
    {
      if(dwFlags & DEL_FORCE)
      {
        break;
      }
    }
  }
  
  return ret;
}

int cmd_md(CHAR* pszCmdName, CHAR* pszParam)
{
  int ret = 0;
  char szFullPath[MAX_PATH] = {0, };
  char szFullPathIn[MAX_PATH] = {0, };
  
  if (pszParam == NULL)
  {
    fprintf(stderr,"***error: the path name is empty\n");
    return ERROR_BAD_PATHNAME;
  }
  
  PathFormatEx(pszParam, szFullPathIn);
  if(strlen(szFullPathIn) == 0)
  {
    fprintf(stderr,"***warning: empty path to create. ignore it\n");
    return 0;
  }

  if(!GetFullPathName(szFullPathIn, MAX_PATH, szFullPath, NULL))
  {
    ret = (int)GetLastError();
    fprintf(stderr,"***error(%d): fail to get valid path name %s\n", ret, pszParam);
    return ret;
  }
  
  ret = 0;
  
  if (!IsExistingDirectory(szFullPath))
  {
    MakeFullPath(szFullPath);
    if (!IsExistingDirectory(szFullPath))
    {
      ret = ERROR_ACCESS_DENIED;
    }
  }
  
  if (ret)
  {
    fprintf(stderr,"***error(%d): fail to create path %s\n", ret, szFullPath);
  }

  return ret;
}

BOOL DeleteFolder(LPTSTR FileName, DWORD* pdwErr)
{
  TCHAR Base[MAX_PATH];
  TCHAR TempFileName[MAX_PATH];
  HANDLE hFile;
  WIN32_FIND_DATA f;
  
  _tcscpy(Base, FileName);
  _tcscat(Base, _T("\\*"));
  hFile = FindFirstFile(Base, &f);
  
  Base[_tcslen(Base) - 1] = _T('\0');
  if (hFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (!_tcscmp(f.cFileName, _T(".")) || !_tcscmp(f.cFileName, _T("..")))
      {
        continue;
      }

      _tcscpy(TempFileName, Base);
      _tcscat(TempFileName, f.cFileName);
      
      if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        DeleteFolder(TempFileName, pdwErr);
      }
      else
      {
        SetFileAttributes(TempFileName, FILE_ATTRIBUTE_NORMAL);
        if (!DeleteFile(TempFileName))
        {
          DWORD err = GetLastError();
          if(pdwErr)
          {
            *pdwErr = err;
          }
          _tprintf(_T("*** warnning : fail to delete file %s(err=%d)\n"), TempFileName, err);
        }
      }      
    } while (FindNextFile(hFile, &f));
    
    FindClose(hFile);
  }
  
  return RemoveDirectory(FileName);
}

// change '/' to '\\' and remove all trailing  slash and blanks.
TCHAR* PathFormatEx(CONST TCHAR *szPath, TCHAR* out)
{
	CONST TCHAR *str;
  int i = 0;

  str = szPath;
  
  while(*str == (' ')) str++; // Skip blank.

  for (; *str; str++)
  {
		if(*str == ('/'))
    {
      *out = ('\\');
    }
    else 
      *out = *str;
    out ++;
    i ++;
  }
  *out = *str;

  if(i)
  {
    i = 0;
    while(out[--i] == (' ') || out[i] == ('\\')) 
      out[i] = ('\0'); // SKip '\\';
  }
  
  return out;
}

int cmd_rd(CHAR* pszCmdName, CHAR* pszParam)
{
  int ret = 0;
  LPTSTR* arg;
  INT args;
  INT dirCount;  
  BOOL bSubDir   = FALSE;
  BOOL bQuiet = FALSE;
  int i = 0;
  TCHAR ch;
  TCHAR szFullPath[MAX_PATH];
  INT res;

  dirCount = 0;
  arg = split(pszParam, &args, FALSE);
  
  /* check for options anywhere in command line */
  for (i = 0; i < args; i++)
  {
    if (*arg[i] == _T('-'))
    {
      /* founda command, but check to make sure it has something after it */
      if (_tcslen(arg[i]) == 2)
      {
        ch = _totupper(arg[i][1]);
        
        if (ch == _T('S'))
        {
          bSubDir = TRUE;
        }
        else if (ch == _T('Q'))
        {
          bQuiet = TRUE;
        }
      }
    }
    else
    {
      dirCount++;
    }
  }
  
  if (dirCount == 0)
  {
    freep(arg);
    
    if (!bQuiet)
    {
      fprintf(stderr,"***error(%d): fail to find any input folder\n");
      return 1;
    }
    
    return 0;
  }
  
  for (i = 0; i < args; i++)
  {
    LPCTSTR pszFolder = NULL;
    LPCSTR lpFileName = arg[i];
    
    if (*arg[i] == _T('-'))
      continue;    
    
    if(lpFileName[0] == 0) 
      continue;
    
    if (bSubDir)
    {
      /* get the folder name */
      GetFullPathName(lpFileName, MAX_PATH, szFullPath, NULL);
      
      /* remove trailing \ if any, but ONLY if dir is not the root dir */
      if (_tcslen(szFullPath) >= 2 && szFullPath[_tcslen(szFullPath) - 1] == _T('\\'))
        szFullPath[_tcslen(szFullPath) - 1] = _T('\0');
      
      res = DeleteFolder(szFullPath, &ret);
      pszFolder = szFullPath;
    }
    else
    {
      res = RemoveDirectory(lpFileName);
      pszFolder = lpFileName;
      if(!res)
      {
        ret = GetLastError();
      }
    }    
    
    if (ret && !bQuiet)
    {
      freep(arg);
      fprintf(stderr,"***error(%d): fail to delete folder %s\n", ret, pszFolder);
      return ret;
    }
  }
  
  freep(arg);
  Sleep(10);

  return 0;
}

static char* g_apszExcludeName[128] = {0, };
BOOL CheckExcludeFile(char* pszName, BOOL bMatch)
{
  int i =0;
  
  for (i = 0; i < sizeof(g_apszExcludeName)/sizeof(g_apszExcludeName[0]); i++)
  {
    if (g_apszExcludeName[i])
    {
      if(_tcsicmp(pszName, g_apszExcludeName[i]) == 0)
      {
        return TRUE;
      }
      
      if(bMatch)
      {
        /* The following lines of copy were written by someone else (most likely Eric Khoul) and it was taken from ren.c */
        PCHAR p, q, r;
        char DoneFile[MAX_PATH] = {0};        
        /* build destination file name */
        
        p = pszName;
        q = g_apszExcludeName[i];
        r = DoneFile;
        while (*q != 0)
        {
          if (*q == '*')
          {
            q++;
            while (*p != 0 && *p != *q)
            {
              *r = *p;
              p++;
              r++;
            }
          }
          else if (*q == '?')
          {
            q++;
            if (*p != 0)
            {
              *r = *p;
              p++;
              r++;
            }
          }
          else
          {
            *r = *q;
            if (*p != 0)
              p++;
            q++;
            r++;
          }
        }        
        *r = 0;

        if (_tcsicmp(pszName, DoneFile) == 0)
        {
          return TRUE;
        }
        else
        {
          break;
        }
      }
    }
    else
    {
      break;
    }
  }
  
  return FALSE;
}

DWORD CopyFolder(TCHAR* apszCpDir[2], BOOL bQuiet, BOOL bMatch)
{
  DWORD ret = 0;
  TCHAR Base[MAX_PATH];
  TCHAR srcFileName[MAX_PATH];
  TCHAR dstFileName[MAX_PATH];
  HANDLE hFile;
  WIN32_FIND_DATA f;
  
  if((strlen(apszCpDir[0]) >= MAX_PATH) || (strlen(apszCpDir[1]) >= MAX_PATH))
  {
    fprintf(stderr,"***error: too long input\n");
    return ERROR_LABEL_TOO_LONG;
  }
  
  _tcscpy(Base, apszCpDir[0]);
  _tcscat(Base, _T("\\*"));
  hFile = FindFirstFile(Base, &f);  
  Base[_tcslen(Base) - 1] = _T('\0');
  
  if (hFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (!_tcscmp(f.cFileName, _T(".")) || !_tcscmp(f.cFileName, _T("..")))
      {
        continue;
      }

      _tcscpy(srcFileName, Base);
      _tcscat(srcFileName, f.cFileName);
      
      _tcscpy(dstFileName, apszCpDir[1]);
      _tcscat(dstFileName, _T("\\"));
      _tcscat(dstFileName, f.cFileName);
      
      if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        TCHAR* apszCpDirTmp[2] = {srcFileName, dstFileName};
        ret = CopyFolder(apszCpDirTmp, bQuiet, bMatch);
      }
      else
      {
        if(CheckExcludeFile(f.cFileName, bMatch)) 
        {
          continue;
        }
        
        if(!IsExistingDirectory(apszCpDir[1]))
        {
          MakeFullPath(apszCpDir[1]);
        }
        
        SetFileAttributes(dstFileName, FILE_ATTRIBUTE_NORMAL);
        if (!CopyFile(srcFileName, dstFileName, FALSE))
        {
          DWORD err = GetLastError();
          if (!bQuiet)
          {
            ret = err;
          }
          fprintf(stderr,"***error: fail to copy %s to %s\n", srcFileName, dstFileName);
        }
      }
    } while (FindNextFile(hFile, &f));
    
    FindClose(hFile);
  }
  
  return ret;
}

int cmd_cpyx(CHAR* pszCmdName, CHAR* pszParam)
{
  int ret = 0;
  LPTSTR* arg;
  INT args;
  INT dirCount;  
  BOOL bSubDir   = FALSE;
  BOOL bQuiet = FALSE;
  BOOL bMatch = FALSE;
  int i = 0;
  TCHAR ch;
  TCHAR szSrcFullPath[MAX_PATH];
  TCHAR szDstFullPath[MAX_PATH];
  int ecnt = 0;
  TCHAR* apszCpDir[2] = {0,0};
  
  for (i = 0; i < sizeof(g_apszExcludeName) / sizeof(g_apszExcludeName[0]); i++)
  {
    g_apszExcludeName[i] = 0;
  }
  
  dirCount = 0;
  arg      = split(pszParam, &args, FALSE);
  
  /* check for options anywhere in command line */
  for (i = 0; i < args; i++)
  {
    if (*arg[i] == _T('-'))
    {
      /* founda command, but check to make sure it has something after it */
      if (_tcslen(arg[i]) == 2)
      {
        ch = _totupper(arg[i][1]);
        
        if (ch == _T('S'))
        {
          bSubDir = TRUE;
        }
        else if (ch == _T('Q'))
        {
          bQuiet = TRUE;
        }
        else if (ch == _T('X'))
        {
          bMatch = TRUE;
          if(ecnt < sizeof(g_apszExcludeName)/sizeof(g_apszExcludeName[0]))
          {
            g_apszExcludeName[ecnt++] = arg[i+1];
            i++;
          }
          else
          {
            fprintf(stderr,"***error: too many parameters with -e, please check -e option.\n");
            return 1;
          }
        }
        else if (ch == _T('E'))
        {
          if(ecnt < sizeof(g_apszExcludeName)/sizeof(g_apszExcludeName[0]))
          {
            g_apszExcludeName[ecnt++] = arg[i+1];
            i++;
          }
          else 
          {
            fprintf(stderr,"***error: too many parameters with -e, please check -e option.\n");
            return 1;
          }
        }
      }
    }
    else
    {
      if(dirCount < 2)
      {
        apszCpDir[dirCount++] = arg[i];
      }
    }
  }
  
  if (dirCount < 2)
  {
    freep(arg);
    
    if (!bQuiet)
    {
      fprintf(stderr,"*** error: fail to find any input folder or file.\n");
      return 1;
    }
    
    return 0;
  }
  
  /* get the folder name */
  GetFullPathName(apszCpDir[0], MAX_PATH, szSrcFullPath, NULL);
  
  /* remove trailing \ if any, but ONLY if dir is not the root dir */
  if (_tcslen(szSrcFullPath) >= 2 && szSrcFullPath[_tcslen(szSrcFullPath) - 1] == _T('\\'))
    szSrcFullPath[_tcslen(szSrcFullPath) - 1] = _T('\0');
  
  GetFullPathName(apszCpDir[1], MAX_PATH, szDstFullPath, NULL);
  
  /* remove trailing \ if any, but ONLY if dir is not the root dir */
  if (_tcslen(szDstFullPath) >= 2 && szDstFullPath[_tcslen(szDstFullPath) - 1] == _T('\\'))
    szDstFullPath[_tcslen(szDstFullPath) - 1] = _T('\0');
  
  apszCpDir[0] = szSrcFullPath;
  apszCpDir[1] = szDstFullPath;  
  
  if (bSubDir)
  {
    ret = CopyFolder(apszCpDir, bQuiet, bMatch);
  }
  else
  {
    if(!IsExistingFile(apszCpDir[0]))
    {
      if (!bQuiet) 
      {
        fprintf(stderr,"***error: fail to find file %s to copy\n", apszCpDir[0]);
        return ERROR_NOT_FOUND;
      }
      return 0;
    }
    
    SetFileAttributes(apszCpDir[1], FILE_ATTRIBUTE_NORMAL);
    if (!CopyFile(apszCpDir[0], apszCpDir[1], FALSE))
    {
      ret = GetLastError();
      fprintf(stderr,"***error: fail to copy %s to %s\n", apszCpDir[0], apszCpDir[1]);
    }
    else 
    {
      ret = 0;
      if (!bQuiet)
      {
        fprintf(stdout, "copy %s to %s ok.\n", apszCpDir[0], apszCpDir[1]);
      }
    }
  }
  
  freep(arg);
  
  return ret;
}

int PrintFile(char* pszFile, char* pszText, int len, BOOL append_to_file)
{
  FILE* f = fopen(pszFile, (append_to_file == 0) ? "wt" : "at");
  if (f)
  {
    pszText[len] = 0;
    fputs(pszText, f);
    fputs("\n", f);
    fclose(f);
    return 0;
  }
  else
  {
    fprintf(stderr,"***error: fail to open file %s(err=%d). \n", pszFile, GetLastError());
    return 2;
  }
}

static int echo_string(char* p)
{
  BOOL bOutputFile = FALSE;
  int len          = 0;
  
  while (*p == ' ')
    p++;
  
  len = strlen(p);
  if (len)
  {
    char* p2 = p + len;
    
    while (p2 > p)
    {
      if ((*p2 == '>' && *(p2 - 1) == '>') || (*p2 == '>'))
      {
        BOOL bAppendFile = (*p2 == '>' && *(p2 - 1) == '>') ? TRUE : FALSE;
        char* file       = p2;
        
        while ((*file == ' ') || (*file == '>'))
        {
          if (*file == ' ' || *file == '>')
          {
            file++;
            continue;
          }
          else
          {
            break;
          }
        }
        
        if (*file)
        {
          bOutputFile = TRUE;
          return PrintFile(file, p, bAppendFile ? (p2 - p - 1) : (p2 - p), bAppendFile);
        }
      }
      p2--;
    }
    
    if (!bOutputFile)
    {
      fputs(p, stdout);
      fputs("\n", stdout);
      fflush(stdout);
      fflush(stderr);
    }
  }
  
  return 0;
}

int cmd_echo(CHAR* pszCmdName, CHAR* pszParam)
{
  return echo_string(pszParam);
}

int cmd_echodt(CHAR* pszCmdName, CHAR* pszParam)
{
  char buf[80] = {0, 0, };
  
  SYSTEMTIME st;
  GetLocalTime(&st);
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d ", st.wYear, st.wMonth, st.wDay,    st.wHour, st.wMinute, st.wSecond);
  fputs(buf, stdout);
  
  cmd_print(pszCmdName, pszParam);
  
  return 0;
}

int cmd_print(CHAR* pszCmdName, CHAR* pszParam)
{
  char* p = pszParam;

  if(p == NULL)
  {
    return 0;
  }
  
  while (*p == ' ')
    p++;
  
  fputs(p, stdout);
  fputs("\n", stdout);
  
  fflush(stdout);
  fflush(stderr);

  return 0;
}

int cmd_printerr(CHAR* pszCmdName, CHAR* pszParam)
{
  char* p = pszParam;
  
  if(p == NULL)
  {
    return 0;
  }
  
  while (*p == ' ')
    p++;
  
  fprintf(stderr,p);
  fprintf(stderr,"\n");
  fflush(stderr);
  
  return 1;
}

static __inline int DoFileProcess(int append_to_file, char* p, int linewrap)
{
  char file[512] = { 0 };
  char* q;

  while (*p == ' ')  p++;
  
  for (q = file; *p; p++, q++)
  {
    if (*p == ' ')
    {
      break;
    }
    else
    {
      *q = *p;
    }
  }
  
  {
    char* path = file;
    
    for (path = file; path && *path; path++)
      if (*path == '/')
        *path = '\\';
  }
  
  SetFileAttributes(file, FILE_ATTRIBUTE_NORMAL);
  
  if (file[0])
  {
    FILE* f = fopen(file, (append_to_file == 0) ? "wt" : "at");
    if (f)
    {
      char* str = NULL;

      while (*p == ' ') p++;

      str = p;
      if (linewrap)
      {
        for (; *p; p++)
        {
          if (*p == ' ')
          {
            *p = '\n';
            p++;
            while (*p == ' ') p++;
          }
        }
      }
      
      fputs(str, f);
      fputs("\n", f);
      
      fclose(f);
    }
    else
    {
      return FALSE;
    }
  }
  else 
  {
    return FALSE;
  }
  
  return TRUE;
}

int cmd_fileat(CHAR* pszCmdName, CHAR* pszParam)
{
  BOOL ret = DoFileProcess(1, pszParam, 0);
  
  if (!ret)
  {
    fprintf(stderr,"***error(%d) : fail to open file!\n", GetLastError());
    fflush(stderr);
    return ERROR_FILE_NOT_FOUND;
  }
  
  return 0;
}

int cmd_filewt(CHAR* pszCmdName, CHAR* pszParam)
{
  BOOL ret = DoFileProcess(0, pszParam, 0);
  
  if (!ret)
  {
    fprintf(stderr,"***error(%d) : fail to open file!\n", GetLastError());
    fflush(stderr);
    return ERROR_FILE_NOT_FOUND;
  }
  
  return 0;
}

int cmd_filewtl(CHAR* pszCmdName, CHAR * pszParam)
{
  BOOL ret = DoFileProcess(0, pszParam, 1);
  
  if (!ret)
  {
    fprintf(stderr,"***error(%d) : fail to open file!\n", GetLastError());
    fflush(stderr);
    return ERROR_FILE_NOT_FOUND;
  }
  
  return 0;
}

int cmd_fileatl(CHAR* pszCmdName, CHAR * pszParam)
{
  BOOL ret = DoFileProcess(1, pszParam, 1);
  
  if (!ret)
  {
    fprintf(stderr,"***error(%d) : fail to open file!\n", GetLastError());
    fflush(stderr);
    return ERROR_FILE_NOT_FOUND;
  }
  
  return 0;
}

BOOL IsExistingFile (LPCTSTR pszPath)
{
	DWORD attr = GetFileAttributes (pszPath);
	return (attr != 0xFFFFFFFF && (! (attr & FILE_ATTRIBUTE_DIRECTORY)) );
}

BOOL IsExistingDirectory (LPCTSTR pszPath)
{
	DWORD attr = GetFileAttributes (pszPath);
	return (attr != 0xFFFFFFFF && (attr & FILE_ATTRIBUTE_DIRECTORY) );
}

static DWORD DeleteFilesEx(LPCTSTR FileName, DWORD dwFlags, int* pret)
{
  int ret = 0;
  TCHAR szFullPath[MAX_PATH];
  TCHAR szFileName[MAX_PATH];
  LPTSTR pFilePart;
  HANDLE hFile;
  WIN32_FIND_DATA f;
  DWORD dwFiles = 0;
  
  _tcscpy(szFileName, FileName);
  
  if (_tcschr(szFileName, _T('*')) == NULL && IsExistingDirectory(szFileName))
  {
    /* If it doesnt have a \ at the end already then on needs to be added */
    if (szFileName[_tcslen(szFileName) - 1] != _T('\\'))
      _tcscat(szFileName, _T("\\"));
    
    /* Add a wildcard after the \ */
    _tcscat(szFileName, _T("*"));
  }
  
  // change to win32 format and get the full path.
  GetFullPathName(szFileName, MAX_PATH, szFullPath, &pFilePart);
  
  hFile = FindFirstFile(szFullPath, &f);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      /* ignore directories */
      if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        continue;
      
      _tcscpy(pFilePart, f.cFileName);
      
      if (f.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
      {
        SetFileAttributes(szFullPath, FILE_ATTRIBUTE_NORMAL);
      }
      
      if (DeleteFile(szFullPath))
      {
        dwFiles++;
      }
      else
      {
        int err = GetLastError();          
        if(dwFlags & DEL_FORCE)
        {
          if (pret)
          {
            *pret = err;
          }
          fprintf(stderr, "***error(%d): fail to delete file %s\n", err, szFullPath);
          break;
        }
        else
        {
          fprintf(stderr, "***warning(%d): fail to delete file %s\n", err, szFullPath);
        }
      }
    } while (FindNextFile(hFile, &f));
    
    FindClose(hFile);
  }
  
  return dwFiles;
}

DWORD DeleteAllFiles(LPCTSTR pszFilePath, DWORD dwFlags, int* pret)
{
  return DeleteFilesEx(pszFilePath, dwFlags, pret);
}

static int DoDeleteFileE(char* p)
{
  int ret        = 0;
  char* q        = 0;
  char* perrtext = 0;
  
  while (*p == ' ')
    p++;
  
  {
    char* pp = p;
    
    for (; *pp; pp++)
    {
      if (*pp == ' ')
      {
        *pp      = '\0';
        perrtext = pp + 1;
        break;
      }
    }
  }
  
  if (perrtext)
  {
    while (*perrtext == ' ')
      perrtext++;
  }
  
  if (!IsFileExist(p))
  {
    return 0;
  }
  
  SetFileAttributes(p, FILE_ATTRIBUTE_NORMAL);
  
  if (!DeleteFile(p))
  {
    ret = GetLastError();
    if (perrtext)
    {
      fprintf(stderr, "%s\n", perrtext);
    }
    else
    {
      fprintf(stderr, "***error : fail to delete file %s.\n", p);
    }
  }
  else
  {
    ret = 0;
  }
  
  fflush(stdout);
  fflush(stderr);
  
  return ret;
}

static char* check_cmd_swt( char* input, char chswt, int *quiet)
{
  char* left_param = input;
  char* swt        = 0;
  
  swt = input;
  
  for (; *swt; swt += 2)
  {
    if (swt[0] == '-' && swt[1] == chswt)
    {
      *quiet     = 1;
      left_param = swt + 2;
      break;
    }
  }
  
  return left_param;
}

// program.exe any charaters --->
// name = program.exe
// return = any charaters
static char* get_cmd_name_and_left_param(char* input, char** name)
{
  char* leftparam = 0;
  char* pp        = input;
  
  *name = input;
  
  for (; *pp; pp++)
  {
    if (*pp == ' ')
    {
      *name      = input;
      *pp       = '\0';
      leftparam = pp + 1;
      skip_blank(leftparam);
      if(strlen(leftparam)==0) leftparam = 0;
      break;
    }
  }
  
  return leftparam;
}

static int chkexe(char* p)
{
  int ret        = 0;
  int quiet      = 0;
  char* name     = 0;
  char* perrtext = 0;
  char* left_param = 0;
  
  //
  // -q program.exe
  // -q program.exe ***error: don't find program.exe file.
  //
  skip_blank(p);
  left_param = check_cmd_swt(p, 'q', &quiet); // check  '-q'   
  skip_blank(left_param);
  perrtext = get_cmd_name_and_left_param(left_param, &name); // get name(program.exe) and error string(***error: don't find program.exe file.).
  skip_blank(name);
  skip_blank(perrtext);
  
  {
    TCHAR lpExecFileBuffer[MAX_PATH];
    LPSTR lpFilePart    = 0;
    lpExecFileBuffer[0] = 0;
    
    if (SearchPath(NULL, (LPCSTR)name, NULL, MAX_PATH, lpExecFileBuffer, &lpFilePart))
    {
      if(!quiet)
      {
        fprintf(stdout, "%s \n", lpExecFileBuffer);
      }
    }
    else
    {
      if (SearchPath(NULL, (LPCSTR) name, ".exe", MAX_PATH, lpExecFileBuffer, &lpFilePart))
      {
        if (!quiet)
        {
          fprintf(stdout, "%s \n", lpExecFileBuffer);
        }
      }
      else
      {
        if (SearchPath(NULL, (LPCSTR) name, ".bat", MAX_PATH, lpExecFileBuffer, &lpFilePart))
        {
          if (!quiet)
          {
            fprintf(stdout, "%s \n", lpExecFileBuffer);
          }
        }
        else
        {
          if (perrtext)
          {
            fprintf(stderr, "%s\n", perrtext);
          }
          else
          {
            fprintf(stderr, "*** error : fail to find file %s.\n", name);
          }
          
          ret = GetLastError();
          if (ret == 0)
          {
            ret = ERROR_NOT_FOUND;
          }
        }
      }
    }
  }
  
  fflush(stdout);
  fflush(stderr);
  
  return ret;
}


#if HAVE_MY_STRCHR
TCHAR* strchr(const TCHAR * string, INT ch)
{
  while (*string && *string != (TCHAR)ch)
    string++;
  
  if (*string == (TCHAR)ch)
    return((TCHAR *)string);
  
  return(NULL);
}
#endif

// return error code
int MakeFullPath(TCHAR* DirPath)
{
  TCHAR path[MAX_PATH+2]       = {0, };
  TCHAR szFullPath[MAX_PATH+2] = {0, };  
  TCHAR* p = szFullPath;
  INT n    = 0;
  int len  = 0;
  
  //
  // Get the full path to the file 
  //  
  GetFullPathName(DirPath, MAX_PATH, szFullPath, NULL);
  
  if (CreateDirectory(szFullPath, NULL))
  {
    return 0;
  }
  else
  {
    if (IsExistingDirectory(path))
    {
      return 0;
    }
  }
  
  //
  // got ERROR_PATH_NOT_FOUND, so try building it up one component at a time
  //
  if (p[0] && p[1] == _T(':'))
  {
    p += 2;
  }
  
  while (*p == _T('\\'))
  {
    p++; // skip drive root  C:\\a\\b\\c
  }
  
  len = _tcslen(szFullPath);
  do
  {
    p = _tcschr(p, _T('\\'));
    n = p ? p++ - szFullPath : len;
    _tcsncpy(path, szFullPath, n);
    path[n] = _T('\0');

    if (!IsExistingDirectory(path))
    if (!CreateDirectory(path, NULL))
    {
      DWORD err = GetLastError();
      if (!IsExistingDirectory(path))
      {
        fprintf(stderr, "***error(%d): fail to create direcotry: %s\n", err, path);
        return err;
      }
    }
  } while (p != NULL);
  
  return 0;
}


// Create the Dir, support nesting path.
int MakeDir(LPCTSTR pszDir)
{
  TCHAR* endptr = NULL, *ptr = NULL, *slash = NULL, *pStrDir = NULL;
  TCHAR szDir[MAX_PATH] = { 0 };
  
  int len = strlen(pszDir);
  
  if (len >= MAX_PATH)
  {
    strncpy(szDir, pszDir, MAX_PATH);
    PathFormatEx(szDir, szDir);
  }
  else
  {
    PathFormatEx(pszDir, szDir);  // PathRemoveBackslash 
  }
  
  pStrDir = szDir;
  
  return MakeFullPath(pStrDir);
}

static int DoMkDir(char* p)
{
  int ret     = 0;
  BOOL silent = FALSE;
  char* q     = 0;
  
  while (*p == ' ')
    p++;
  q = p;
  
  for (; *p; p += 2)
  {
    if (p[0] == '-' && p[1] == 'q')
    {
      silent = TRUE;
      p += 2;
      q = p;
      break;
    }
  }
  
  while (*q == ' ')
    q++;
  p = q;
  
  if (*p == 0)
  {
    if (!silent)
    {
      fprintf(stderr,"*** error : no any dir to create.\n");
      ret = ERROR_FILE_NOT_FOUND;
    }
    fflush(stdout);
    fflush(stderr);
    return ret;
  }
  
  if (IsExistingDirectory(p))
  {
    return 0;
  }

  ret = CreateDirectory((LPTSTR) p, (LPSECURITY_ATTRIBUTES) NULL);
  if (ret)
  {
    if (!silent)
      fprintf(stdout, "create dir %s ok.\n", p);
  }
  else
  {
    MakeDir(p);
    if (IsExistingDirectory(p))
    {
      if (!silent)
        fprintf(stdout, "create dir %s ok.\n", p);
      fflush(stdout);
      fflush(stderr);
      return 0;
    }
    else
    {
      if (!silent)
        fprintf(stdout, "*** error : fail to create dir %s.\n", p);
      fflush(stdout);
      fflush(stderr);
      if (!silent)
        return 1;
      else
        return 0;
    }
  }
  
  fflush(stdout);
  fflush(stderr);
  
  return 0;
}

static int DoChDir(char* p)
{
  while (*p == ' ')
    p++;
  
  if (*p == 0)
  {
    fprintf(stderr,"***error: no any input to change current directory\n");
    return 1;
  }
  
  if (chdir(p) < 0)
  {
    fprintf(stderr,"***error: change current directory to %s fail. \n", p);
    return 1;
  }
  else
  {
    char current_directory[MAX_PATH + 64] = { 0 };
    
    getcwd_fs(current_directory, sizeof(current_directory));
    printf("%s\n", current_directory);
  }
  
  return 0;
}

static int findx(char* p, char bFindErr)
{
  int ret = 0;
  
  char* perrtext = NULL;
  
  while (*p == ' ')
    p++;
  
  {
    char* path;
    
    for (path = p; path && *path; path++)
      if (*path == '/')
        *path = '\\';
  }
  
  {
    char* pp = p;
    
    for (; *pp; pp++)
    {
      if (*pp == ' ')
      {
        *pp      = '\0';
        perrtext = pp + 1;
        break;
      }
    }
  }
  
  if (perrtext)
  {
    while (*perrtext == ' ')
      perrtext++;
  }

  if(bFindErr)
  {
    if (IsFileExist(p))
    {
      ret = ERROR_ALREADY_EXISTS;
      if (perrtext && perrtext[0])
      {
        fprintf(stderr,"%s\n", perrtext);
      }
      else
      {
        fprintf(stderr,"***error: find file '%s'.\n", p);
      }
    }
    else
    {
      ret = 0;
    }
  }
  else 
  {
    if (IsFileExist(p))
    {
      ret = 0;
    }
    else
    {
      if (perrtext && perrtext[0])
      {
        fprintf(stderr,"%s\n", perrtext);
      }
      else
      {
        fprintf(stderr,"***error: fail to find file '%s'.\n", p);
      }
      ret = ERROR_NOT_FOUND;
    }
  }
  
#if 0
  if (access(p, 0) == 0)
  {
    ret = 0;
  }
  else
  {
    fprintf(stderr,"***error: fail to find file '%s'.\n", p);
    ret = GetLastError();
  }
#endif
  
  return ret;
  
#if 0
  // access(us, 0) == 0 
  BOOL ret = FALSE;
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind = 0;
  
  while (*p == ' ')
    p++;
  
  hFind = FindFirstFile(p, &FindFileData);
  if (hFind == INVALID_HANDLE_VALUE)
  {
    fprintf(stderr,"***error: fail to find file '%s'.\n", p);
    *perror = GetLastError();
  }
  else
  {
    FindClose(hFind);
  }
#endif
  
#if 0
  OFSTRUCT file_info = { 0 };
  
  if ((HANDLE)OpenFile(p, &file_info, OF_READ) != (HANDLE)HFILE_ERROR)
  {
    *ret = 0;
  }
  else
  {
    fprintf(stderr, "***error: fail to find file '%s'.\n", p);
    ret = GetLastError();
  }
#endif
  
  return ret;
}

//
// batch find file : btfind "1.txt 2.txt 3.txt"
//
static int btfind(char* p)
{
  int ret = 0;
    
  while (*p == ' ')
    p++;
  
  {
    char* path;
    
    for (path = p; path && *path; path++)
      if (*path == '/')
        *path = '\\';
  }
  
  {
    char buffer[1024];
    char* pFile = buffer;
    char* pp = p;
    
    for (; *pp; pp++)
    {
      if (*pp == ' ')
      {
        *pFile = '\0';
         pFile = buffer;
        if (*pFile && !IsFileExist(pFile))
        {
          ret = ERROR_NOT_FOUND;
          fprintf(stderr,"***error: fail to find file '%s'.\n", pFile);
        }
      }
      else
      {
        if(pFile >= &buffer[sizeof(buffer)-1])
        {
          fprintf(stderr,"***error: intput file path is too long.\n");
          ret = RPC_S_STRING_TOO_LONG;
          break;
        }
        *pFile++ = *pp;
      }
    }
  }
  
  return ret;
}

static int chkempty(char* p)
{
  int ret = 0;
  
  while (*p == ' ')
    p++;
  
  ret = ERROR_EMPTY;
  
  {
    FILE* f = fopen(p, "r");
    if (f)
    {
      char szline[128] = { 0 };
      
      while (fgets(szline, 128, f))
      {
        char* pline = szline;
        
        while (*pline == ' ')
          pline++;
        while (*pline == '\r')
          pline++;
        while (*pline == '\n')
          pline++;
        while (*pline == '\t')
          pline++;
        
        if (*pline)
        {
          ret = 0;
          break;
        }
        memset(szline, 0, sizeof(szline));
      }
      fclose(f);
    }
    else
    {
      ret = 0;
    }
    
    if (ret)
    {
      fprintf(stderr,"***error: file '%s' is empty.\n", p);
    }
  }
  
  return ret;
}

int cmd_rmfe(CHAR* pszCmdName, CHAR* pszParam)
{
  return DoDeleteFileE(pszParam);
} 

int cmd_chkexe(CHAR* pszCmdName, CHAR* pszParam)
{
  return chkexe(pszParam);
}

int cmd_echodummy(CHAR* pszCmdName, CHAR* pszParam)
{
  return 0;
}

int cmd_mkdirx(CHAR* pszCmdName, CHAR* pszParam)
{
  return DoMkDir(pszParam);
}

int cmd_chdirx(CHAR* pszCmdName, CHAR* pszParam)
{
  return DoChDir(pszParam);
}

int cmd_findx(CHAR* pszCmdName, CHAR* pszParam)
{
  return findx(pszParam, 0);
}

int cmd_findxe(CHAR* pszCmdName, CHAR* pszParam)
{
  return findx(pszParam, 1);
}

int cmd_btfind(CHAR* pszCmdName, CHAR* pszParam)
{
  return btfind(pszParam);
}

int cmd_chkempty(CHAR* pszCmdName, CHAR* pszParam)
{
  return chkempty(pszParam);
}

void splitparam(char** p1, char** p2, char* input)
{
  char* p = input;
  
  while (*p == ' ')
    p++;
  
  {
    char* pp = p;
    
    for (; *pp; pp++)
    {
      if (*pp == ' ')
      {
        *pp = '\0';
        *p1 = p;
        pp++;
        while (*pp == ' ')
          pp++;
        *p2 = pp;
        break;
      }
    }
  }
}

int cmd_cmdexec(CHAR* pszCmdName, CHAR* pszParam)
{
  int ret        = 0;
  char* cmd_line = 0;
  char* p1       = 0, *p2 = 0;
  int append     = 0;
  int len        = 0;

  if(strcmp(pszCmdName, "cmdexeca") == 0)
  {
    append = 1;
  }
  else if(strcmp(pszCmdName, "cmdexec") == 0)
  {
    append = 0;
  }
  else
  {
    fprintf(stderr,"***error: not find valid command.\n");
    return 1;
  }
  
  if (pszParam == 0)
    return 0;
  
  len = strlen(pszParam) + 64;
  cmd_line = malloc(len);
  if (cmd_line == 0)
  {
    ret = GetLastError();
    fprintf(stderr,"***error: no more memory to use.\n");
    return ret;
  }
  
  memset(cmd_line, 0, len);  
  strcpy(cmd_line, pszParam);
  splitparam(&p1, &p2, cmd_line);
  
  if(p1 && p2)
  {
    ret = process_exec(p2, p1, append);
  }

  free(cmd_line);
  
  return ret;
}

int cmd_filecat(CHAR* pszCmdName, CHAR * pszParam)
{
  int ret = 0;
	LPTSTR *arg = NULL;
	DWORD dwFlags = 0;
	DWORD dwFiles = 0;
	TCHAR* szOutFile = 0;
  FILE* fo = 0;
	INT args = 0;
	INT i = 0;
  
  arg = split (pszParam, &args, FALSE);
	if (args == 0)
	{
    fprintf(stderr,"***error: filecat: invalid input.\n");
		if(arg) freep (arg);
		return 1;
	}
  
  szOutFile = arg[args-1];
  SetFileAttributes(szOutFile, FILE_ATTRIBUTE_NORMAL);
  fo = fopen(szOutFile, "wb");
  if(fo == 0)
  {
    fprintf(stderr,"***error: filecat: fail to open output file %s.\n", szOutFile);
    freep (arg);
    return ERROR_FILE_NOT_FOUND;
  }
  
  for (i = 0; i < args-1; i++)
  {
    FILE* f = fopen(arg[i], "rb");
    if (f)
    {
      int fsize = 0;
      char* fpbuffer = 0;
      fseek(f, 0, SEEK_END);
      fsize = ftell(f);
      fseek(f, 0 , SEEK_SET);
      
      fpbuffer = (char*)malloc(fsize);
      if(fread(fpbuffer, 1, fsize, f))
      {
        fwrite(fpbuffer, 1, fsize, fo);
      }
      free(fpbuffer); fpbuffer = 0;
      fclose(f);
    }
  }
  
  fclose(fo); fo = 0;
  if(arg) freep (arg); arg = 0;
  
  return 0;
}

void DoFindFiles( char* pszPath, FILE* fo, char* pszExt) 
{
  DWORD dwFiles = 0;
  HANDLE hFile;
  WIN32_FIND_DATA f;
  TCHAR szFullPath[MAX_PATH] = {0, };

  sprintf(szFullPath, "%s\\%s", pszPath, pszExt);
  hFile = FindFirstFile(szFullPath, &f);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      TCHAR szPath[MAX_PATH] = {0, };
      if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        sprintf(szPath, "%s\\%s%s", pszPath, f.cFileName, pszExt);
        DoFindFiles(szPath, fo, pszExt);
      }
      else
      {
        sprintf(szPath, "%s\\%s", pszPath, f.cFileName);

        if (fo)
        {
          fprintf(fo, "%s\n", szPath);
        }
        else
        {
          fprintf(stdout, "%s\n", szPath);
        }
      }
      dwFiles++;
    } while (FindNextFile(hFile, &f));
    
    FindClose(hFile);
  }
}

void DoFindPath( char* pszPath, FILE* fo, char* pszExt )
{
  DWORD dwFiles = 0;
  HANDLE hFile;
  WIN32_FIND_DATA f;
  TCHAR szFullPath[MAX_PATH] = {0, };
 
  DoFindFiles(pszPath, fo, pszExt);

  // change to win32 format and get the full path.
  sprintf(szFullPath, "%s\\*", pszPath);
  hFile = FindFirstFile(szFullPath, &f);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        TCHAR szPath[MAX_PATH] = {0, };
        if(f.cFileName[0] == '.') continue;

        sprintf(szPath, "%s\\%s", pszPath, f.cFileName);
        DoFindPath(szPath, fo, pszExt);
      }
      dwFiles++;
    } while (FindNextFile(hFile, &f));
    
    FindClose(hFile);
  }
}

int cmd_listfile(CHAR* pszCmdName, CHAR * pszParam)
{
  int ret = 0;
  char* p1  = 0, *p2 = 0;
  char cmd_line[1024];
  FILE* fo = 0;
  TCHAR szFullPath[MAX_PATH] = {0, };
  LPTSTR pFilePart;
  TCHAR szExt[64] = {0, };

  memset(cmd_line, 0, 1024);
  strncpy(cmd_line, pszParam, 1024);
  splitparam(&p1, &p2, cmd_line);
  
  if(p1 == 0 ) p1 = cmd_line;
  
  if (p2)
  {
    SetFileAttributes(p2, FILE_ATTRIBUTE_NORMAL);
    fo = fopen(p2, "w");
    if(fo) fclose(fo);
    fo = fopen(p2, "at");
  }
  
  GetFullPathName(p1, MAX_PATH, szFullPath, &pFilePart);
  strncpy(szExt, pFilePart, sizeof(szExt));
  if(pFilePart)
  {
    pFilePart --;
    *pFilePart = 0;
  }

  DoFindPath(szFullPath, fo, szExt);
  
  if(fo) fclose(fo);
  
  return 0;
}

//////////////////////////////////////////////////////////////////////////
const COMMAND_STRU g_stCmdTbl[] =
{
  {_T("print"),    0, cmd_print},
  {_T("echo"),     0, cmd_echo},
  {_T("echodt"),   0, cmd_echodt},
  {_T("echoerr"),  0, cmd_printerr},
  
  {_T("fileat"),   0, cmd_fileat},
  {_T("filewt"),   0, cmd_filewt},
  {_T("filewtl"),  0, cmd_filewtl},
  {_T("fileatl"),  0, cmd_fileatl},
  {_T("filecat"),  0, cmd_filecat},
  
  {_T("filels"),  0, cmd_listfile},
  
  {_T("mkdirx"),   0, cmd_mkdirx},
  {_T("chdirx"),   0, cmd_chdirx},
  
  {_T("findx"),    0, cmd_findx},
  {_T("findxe"),   0, cmd_findxe},
  {_T("btfind"),   0, cmd_btfind},
  {_T("chkempty"), 0, cmd_chkempty},
  
	{_T("rmfe"),      0, cmd_rmfe}, // rm file, if fail and give error info.
  
	{_T("delx"),     0, cmd_del},
	{_T("mdx"),      1, cmd_md},
	{_T("rdx"),      1, cmd_rd},
	{_T("cpyx"),     1, cmd_cpyx},
	{_T("chkexe"),   1, cmd_chkexe},
	{_T("cmdexeca"),  1, cmd_cmdexec},
	{_T("cmdexec"),   1, cmd_cmdexec},
  
	{_T("echodummy"),0, cmd_echodummy},
  
	{0,  0,  0},
};


BOOL ExecuteCmd(CHAR* pszCmdName, int cmd_len, CHAR* pszCmdParam, int* pErrNo)
{
  const COMMAND_STRU* pstCmd;
  CHAR* pszParam = pszCmdParam;
  int ret        = 0;
  int len        = cmd_len;
  
  if (pErrNo)
  {
    *pErrNo = 0;
  }
  
  if (len == 0)
  {
    return TRUE;
  }
  
  for (pstCmd = g_stCmdTbl; pstCmd->name; pstCmd++)
  {
    if (!_tcsnicmp(pszCmdName, pstCmd->name, len) && (pstCmd->name[len] == _T('\0')))
    {
      int ret = 0;
      
      while (_istspace(*pszParam))
      {
        pszParam++;
      }
      
      if (pstCmd->func)
      {
        ret = pstCmd->func(pszCmdName, pszParam);
        if (pErrNo)
          *pErrNo = ret;
        
        return TRUE;
      }
      else 
      {
        return FALSE;
      }
    }
  }
  
  return FALSE;
}


static const COMMAND_STRU* find_buitin_cmd(CHAR* pszCmdName, int cmd_len)
{
  const COMMAND_STRU* pstCmd;
  int ret        = 0;
  int len        = cmd_len;
  
  if (len == 0)
  {
    return 0;
  }
  
  for (pstCmd = g_stCmdTbl; pstCmd->name; pstCmd++)
  {
    if (!_tcsnicmp(pszCmdName, pstCmd->name, len) && (pstCmd->name[len] == _T('\0')))
    {
      if (pstCmd->func)
      {
        return pstCmd;
      }
      else
      {
        return 0;
      }
    }
  }
  
  return 0;
}

COMMAND_STRU* is_builtin_cmd(char* argv, int * perror, char cmd[MAX_PATH], OUT char** cmd_param)
{
  char* p     = 0;
  char* q     = 0;
  int cmd_len = 0;
  
  *perror = 0;
  
  for (p = argv, q = cmd; *p; p++, q++)
  {
    if (q >= &cmd[MAX_PATH - 1])
    {
      fprintf(stderr,"***error: command name too long\n");
      *perror = ERROR_BUFFER_OVERFLOW;
      return 0;
    }
    
    if (*p == ' ')
    {
      break;
    }
    else
    {
      *q = *p;
    }
  }
  
  while (_istspace(*p)) p++;
  
  *cmd_param = p;
  
  cmd_len = strlen(cmd);
  if (cmd_len == 0)
  {
    *perror = ERROR_EMPTY;
    return 0;
  }
  
  return find_buitin_cmd(cmd, cmd_len);
}


//////////////////////////////////////////////////////////////////////////
#include "built_in_demo.h"

// --show-built-in-demo  
void show_builtin_demo()
{
  printf("\nbuilt-in demo\n");
  printf("%s\n", builtin_demo);
  printf("\n");
}
