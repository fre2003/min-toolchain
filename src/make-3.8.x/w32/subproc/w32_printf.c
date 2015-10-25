#if 0 

WORD GetColor(HANDLE hConsole)
{
  BOOL bSuccess;
  
  CONSOLE_SCREEN_BUFFER_INFO csbi = { 0, };
  
  // get the number of character cells in the current buffer
  bSuccess = GetConsoleScreenBufferInfo(hConsole, &csbi);
  if (bSuccess)
  {
    return csbi.wAttributes;
  }
  else
  {
    return 0;
  }
}

DWORD Color(HANDLE hConsole, WORD wColor /* FOREGROUND_GREEN| FOREGROUND_INTENSITY */ )
{
  BOOL bRet = FALSE;
  
  if (!hConsole)
  {
    return 1;
  } 
  
  // no color specified, reset to defaults (white font on black background)
  if (wColor != 0)
    bRet = SetConsoleTextAttribute(hConsole, wColor);
  else
    bRet = SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);  // white text on black bg
  
  return (bRet == TRUE) ? 0 : 1;
}

#include "stdio.h"

extern char* printf_color;

DWORD PrintStyledData(HANDLE hConsole, WORD wColor, CONST VOID* lpBuffer, DWORD nNumberOfCharsToWrite, DWORD * pNumberOfCharsWritten)
{
  DWORD dwRet     = 0;
  DWORD cCharsWritten = 0;
  WORD wPreColor      = 0;
  
  if (hConsole == NULL)
  {
    return 0;
  }
  
  if (hConsole == FALSE)
  {
    return 0;
  }
  
  if (lpBuffer == NULL || nNumberOfCharsToWrite == 0)
  {
    return 0;
  }
  
  if (printf_color == 0)
  {
    fwrite(lpBuffer, 1, nNumberOfCharsToWrite, stderr);
    dwRet = 0;
  }
  else
  {
    wPreColor = GetColor(hConsole);
    Color(hConsole, wColor);
    WriteConsole(hConsole, lpBuffer, nNumberOfCharsToWrite, &cCharsWritten, NULL);
    if (pNumberOfCharsWritten)
      *pNumberOfCharsWritten = cCharsWritten;
    
    if (cCharsWritten != nNumberOfCharsToWrite)
    {
      dwRet = 0;
    }
    else
    {
      dwRet = 0;
    }
    Color(hConsole, wPreColor);
  }
  
  return dwRet;
}

DWORD PrintfE (CONST TCHAR* pFormat, ...) 
{
  HANDLE hConsole     = GetStdHandle(STD_ERROR_HANDLE); // STD_ERROR_HANDLE
  DWORD cCharsWritten = 0;
  DWORD dwRet         = 0;
  TCHAR szOutput[1024];
  int cnt = 0;
  va_list argptr;
  
  if (hConsole == NULL)
  {
    return 0;
  }
  
  if (pFormat == NULL)
  {
    return 0;
  }
  
  ZeroMemory(szOutput, sizeof(szOutput));
  
  va_start(argptr, pFormat);
  cnt = wvsprintf(szOutput, pFormat, argptr);
  va_end(argptr);
  szOutput[1024 - 1] = '\0';
  
  if (cnt == 0)
  {
    return 0;
  }
  
  dwRet = PrintStyledData(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY, szOutput, cnt, &cCharsWritten);
  
  return dwRet;
}

#if 0 
DWORD vfprintfE(CONST TCHAR* pFormat, va_list ap)
{
  HANDLE hConsole     = GetStdHandle(STD_ERROR_HANDLE);
  DWORD cCharsWritten = 0;
  DWORD dwRet         = 0;
  TCHAR szOutput[1024];
  int cnt = 0;
  
  if (hConsole == NULL)
  {
    return 0;
  }
  
  if (pFormat == NULL)
  {
    return 0;
  }
  
  ZeroMemory(szOutput, sizeof(szOutput));
  
  cnt = wvsprintf(szOutput, pFormat, ap);
  szOutput[1024 - 1] = '\0';
  
  if (cnt == 0)
  {
    return 0;
  }
  
  dwRet = PrintStyledData(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY, szOutput, cnt, &cCharsWritten);
  
  return dwRet;
}
#endif // Lokie/2013-08-26 


#if 0 
DWORD PrintfStyle(WORD wColor, CONST TCHAR* pFormat, ...)
{
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD cCharsWritten = 0;
  DWORD dwRet         = 0;
  TCHAR szOutput[1024];
  int cnt = 0;
  va_list argptr;
  
  if (hConsole == NULL)
  {
    return 0;
  }
  
  if (pFormat == NULL)
  {
    return 0;
  }
  
  ZeroMemory(szOutput, sizeof(szOutput));
  
  va_start(argptr, pFormat);
  cnt = wvsprintf(szOutput, pFormat, argptr);
  va_end(argptr);
  szOutput[1024 - 1] = '\0';
  
  if (cnt == 0)
  {
    return 0;
  }
  
  dwRet = PrintStyledData(hConsole, wColor, szOutput, cnt, &cCharsWritten);
  
  return dwRet;
}

#endif // Lokie/2013-08-26 
#endif // Lokie/2013-08-26 
