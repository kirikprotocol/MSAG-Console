
#include "stdafx.h"

#if defined _WIN32

static HANDLE logfile = 0;
static HANDLE hInstance = 0;

void Log_Write(const char* text)
{
  if ( !logfile )
  {
    a_ptr<char> modulename = new char[1024];
    if ((hInstance = GetModuleHandle(0))==0) goto broken;
    if (!GetModuleFileName((HINSTANCE)hInstance,modulename,1024)) goto broken;
    wsprintf(modulename+lstrlen(modulename),".txt");
    logfile = CreateFile(modulename,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS/*OPEN_ALWAYS*/,FILE_ATTRIBUTE_NORMAL,0);
    if ( !logfile ) goto broken;
    SetFilePointer(logfile,0,0,FILE_END);
  }
  DWORD l;
  WriteFile(logfile,text,lstrlen(text),&l,0);
  if ( l != (unsigned)lstrlen(text) ) goto broken;
exit:
  return;
broken:
  MessageBox(0,"can't write log!","log error",MB_OK);
  goto exit;
}

void __cdecl Log_Message(const char* msg,...){
  a_ptr<char> text = new char[1024*4];
  va_list arg;
  va_start(arg,msg);
  int len = wvsprintf(text,msg,arg);
  wsprintf(text+len,"\r\n");
  va_end(msg);
  Log_Write(text);
}

void Log_WinErrorMessage(const char* expr,const char* file, int line)
{
  a_ptr<char> buffer = new char[1024*4];
  //a_ptr<char> lpMsgBuf = new char[1024];
  char* lpMsgBuf = 0;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (char*)&lpMsgBuf,
    0,
    NULL
  );
  wsprintf(buffer,"windows error %s\n  when %s at %s(%d)\n",(char*)lpMsgBuf,expr,file,line);
  LocalFree(lpMsgBuf);
  Log_Write(buffer);
}

#endif
