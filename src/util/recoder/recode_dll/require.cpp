
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

#if defined _WIN32

void* __cdecl operator new(size_t size) { return LocalAlloc(LMEM_FIXED,size); }
void __cdecl operator delete(void* mem) { LocalFree(mem); }

void FatalError(const char* file,int line){
  char* buff = new char[strlen(file)+128];
  sprintf(buff,"Fatal error at %s:%d",file,line);
  Log_Write(buff);
  MessageBox(0,buff,"fatal error",MB_OK|MB_ICONERROR);
  delete buff;
  TerminateProcess(GetCurrentProcess(),0);
}

void __cdecl FatalError(const char* file,int line,const char* format,...){
  char* buff = new char[1024*16];
  int pos = sprintf(buff,"Fatal error at %s:%d\n\nReason: ",file,line);
  va_list arg;
  va_start(arg,format);
  vsprintf(buff+pos,format,arg);
  va_end(arg);
  Log_Write(buff);
  MessageBox(0,buff,"fatal error",MB_OK|MB_ICONERROR);
  delete buff;
 	TerminateProcess(GetCurrentProcess(),0);
}

char* GetImagePath()
{
  char* buff = new char[4*1024];
  memset(buff,0,4*1024);
  ensure(GetModuleFileName(GetModuleHandle(0),buff,4*1024-1));
  return buff;
}

char* GetFullPath(const char* file)
{
  char* imagepath = GetImagePath();
  int len = strlen(imagepath);
  for ( int i=len-1; i>=0; --i ){
    if ( imagepath[i] == '\\' )
    {
      strcpy(imagepath+i+1,file);
      return imagepath;
    }
  }
  FatalError(__FILE__,__LINE__,"Bad image path %s",imagepath);
}

#else
#include <stdarg.h>

void FatalError(const char* file,int line){
	__trace2__("fatal error at %s(%d)",file,line);
	abort();
}

void FatalError(const char* file,int line,const char* format,...){
	char* buff = new char[1024*16];
  int pos = sprintf(buff,"Fatal error at %s:%d\n\nReason: ",file,line);
  va_list arg;
  va_start(arg,format);
  vsprintf(buff+pos,format,arg);
  va_end(arg);
  __trace__(buff);
  delete buff;
  abort();
}

#endif
