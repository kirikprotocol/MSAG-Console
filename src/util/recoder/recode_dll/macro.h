#if !defined HEADER_MACRO
#define HEADER_MACRO

#ifdef _WIN32
#define NORETURN  __declspec(noreturn)
extern void* __cdecl operator new(size_t size);
extern void __cdecl operator delete(void* mem);
extern void NORETURN FatalError(const char* file,int lineno);
extern void NORETURN __cdecl FatalError(const char* file,int lineno,const char* format,...);
extern char* GetImagePath();
extern char* GetFullPath(const char*);
#define ensure(expr) {if (!(expr)) FatalError(__FILE__,__LINE__,"ASSERTION '%s' failed",#expr);}
#define require(expr) {if (!(expr)) FatalError(__FILE__,__LINE__,"ASSERTION '%s' failed",#expr);}
inline bool ISBADHANDLE(HANDLE h) { return ( h==0 || h==INVALID_HANDLE_VALUE ); }
inline bool ISOKHANDLE(HANDLE h) { return (!ISBADHANDLE(h)); }


extern void Log_Write(const char* text);
extern void __cdecl Log_Message(const char* msg,...);
extern void TerminateByASSERTIN(const char* expr,const char* file, int line);
extern void Log_WinErrorMessage(const char* expr,const char* file, int line);
#define XMessage(x) Log_Message x
#else
#include "util/debug.h"
#define ensure __require__
//#define XMessage(x) __trace2__ x
#define XMessage(x)
extern void FatalError(const char* file,int lineno);
extern void FatalError(const char* file,int lineno,const char* format,...);
#endif

#include "a_ptr.h"

#endif
