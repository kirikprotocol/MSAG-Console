// $Id$
/*
  этот файл содержит код для проверки условий.
*/

#if !defined ( __Cpp_Header__smsc_util_debug_h__ )
#define __Cpp_Header__smsc_util_debug_h__

#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>

#include "logger/Logger.h"

#ifndef _WIN32
#include <pthread.h>
#endif
#include <stdexcept>

using std::runtime_error;

namespace smsc{
namespace logger{
  extern smsc::logger::Logger* _trace_cat;
}
}

#ifdef _WIN32

#include <stdarg.h>
#include <windows.h>
#include <time.h>

#define __require__(expr)
#define require(expr)
#define __trace__(txt) trace2("%s",txt)
#define __trace2__ trace2

static inline void trace2(const char* fmt,...)
{
#ifndef DISABLE_TRACING
#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif
  va_list lst;
  va_start(lst,fmt);
  if(fileno(stderr)==2)
  {
    char fn[256];
    sprintf(fn,"smpp.%u-%08x.log",time(NULL),GetCurrentProcessId());
    *stderr=*fopen(fn,"wt");
  }
  char buf[4096];
  int n=sprintf(buf,"*trace*[%-5d %d]:",GetCurrentThreadId(),time(NULL));
  n+=vsprintf(buf+n,fmt,lst);
  sprintf(buf+n,"\n");
  fprintf(stderr,"%s",buf);
  fflush(stderr);
  va_end(lst);
#endif
}

#define __warning__(txt) warning2("%s",txt)
#define __warning2__ warning2
static inline void warning2(const char* fmt,...)
{
#ifndef DISABLE_ANY_CHECKS
  va_list lst;
  va_start(lst,fmt);
  char buf[4096];
  if(fileno(stderr)==2)
  {
    char fn[256];
    sprintf(fn,"smpp.%u-%08x.log",time(NULL),GetCurrentProcessId());
    *stderr=*fopen(fn,"wt");
  }
  int n=sprintf(buf,"*WARNING*[%-5d %d]:",GetCurrentThreadId(),time(NULL));
  n+=vsprintf(buf+n,fmt,lst);
  sprintf(buf+n,"\n");
  fprintf(stderr,"%s",buf);
  fflush(stderr);
  va_end(lst);
#endif
}
#define __goto_if_fail__(expr,label)
#define __ret_if_fail__(expr)
#define __ret0_if_fail__(expr)
#define __ret1_if_fail__(expr)
#define __retneg_if_fail__(expr)
#define __retval_if_fail__(expr,val)
#define __throw_if_fail__(expr,except)
#define __unreachable__(text)
#define __watch__(var)
#define throw_if(expr)
#define __trace2_if_fail__

#else

#if defined ccassert
  #error "opps, ccassert alreadry defined"
#endif

#if defined DISABLE_ANY_CHECKS
//  #warning "any chacks will disabled, it is very dungrouse mode"
  #if !defined DISABLE_HARD_CHECKS
    #define DISABLE_HARD_CHECKS
  #endif
  #if !defined DISABLE_SOFT_CHECKS
    #define DISABLE_SOFT_CHECKS
  #endif
  #if !defined DISABLE_WATCHDOG
    #define DISABLE_WATCHDOG
  #endif
  #if !defined DISABLE_TRACING
    #define DISABLE_TRACING
  #endif
#endif

#define require(expr) ccassert(expr)
#define __require__(expr) ccassert(expr)

#define __PRETTY_FUNCTION__ __func__

// very hard checks and can't be disabled


#define __warning__(text) \
  smsc::util::warningImpl(text,__FILE__,__PRETTY_FUNCTION__,__LINE__)

#define warning_if(expr) __warning2__(#expr)
//  {if (expr) smsc::util::warningImpl("Warning !!! "#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__);}

#define __warning2__(text,...) if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN)) smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,text,__VA_ARGS__)

#define __log2__(category,priority,text,...) if(category->isLogLevelEnabled(priority)) category->log_(priority,text,__VA_ARGS__)

#if !defined DISABLE_WATCHDOG
  #define __watchdog__(expr) __watchdog2__(expr,"GAW-GAW")
  #define __watchdog2__(expr,info) \
    smsc::util::watchdogImpl(expr,info,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
#else
  #define __watchdog__(expr) (expr)
  #define __watchdog2__(expr,info) (expr)
#endif

#define throw_if_fail(expr) \
    if ( !__watchdog2__(expr,"failed: "#expr"\n\tthrow runtime_error") ) throw runtime_error("failed: "#expr); else;
#define throw_if(expr) throw_if_fail(!(expr))

#if !defined DISABLE_SOFT_CHECKS
  #define __goto_if_fail__(expr,label) \
    if ( !__watchdog2__(expr,"goto :" #label) ) goto label; else;
  #define __throw_if_fail__(expr,excep) \
    if ( !__watchdog2__(expr,"throw " #excep) ) throw excep(); else;
  #define __ret_if_fail__(expr) \
    if ( !__watchdog2__(expr,"return")) return; else;
  #define __retval_if_fail__(expr,val) \
    if ( !__watchdog2__(expr,"return " #val) ) return val; else;
  #define __ret0_if_fail__(expr) __retval_if_fail__(expr,0) /*aka false*/
  #define __ret1_if_fail__(expr) __retval_if_fail__(expr,1) /*aka true*/
  #define __retneg_if_fail__(expr) __retval_if_fail__(expr,-1)
#else
  #define __goto_if_fail__(expr,label)
  #define __ret_if_fail__(expr)
  #define __ret0_if_fail__(expr)
  #define __ret1_if_fail__(expr)
  #define __retneg_if_fail__(expr)
  #define __retval_if_fail__(expr,val)
  #define __throw_if_fail__(expr,except)
#endif

#define retval_if_fail(expr,val) \
  if ( !__watchdog2__(expr,"return " #val) ) return val; else;
#define ret0_if_fail(expr) retval_if_fail(expr,0) /*aka false*/
#define ret1_if_fail(expr) retval_if_fail(expr,1) /*aka true*/

#if !defined ( LOG_DOMAIN )
  #define LOG_DOMAIN "DEBUG"
  //#error "LOG_DOMAIN undefined"
#endif

#if !defined ( LOG_STREAM )
  #define LOG_STREAM stderr
  //#error "LOG_SRTREAM undefined"
#endif

#if !defined ( ASSERT_LOG_DOMAINM )
  #define ASSERT_LOG_DOMAIN "ASSERT"
#endif

#if !defined ( UNREACHABLE_LOG_DOMAINM )
  #define UNREACHABLE_LOG_DOMAIN "UNREACHABLE"
#endif

#if !defined ( ASSERT_LOG_STREAM )
  #define ASSERT_LOG_STREAM LOG_STREAM
#endif

#if defined ( DISABLE_HARD_CHECKS )
  #define ccassert(expr)
  #define __unreachable__(text)
#else
  #if defined ( ASSERT_ABORT_IF_FAIL )
    #define ccassert(expr) \
      smsc::util::abortIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
                #define __unreachable__(text) \
                  smsc::util::abortIfReached(text,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #elif defined ( ASSERT_THROW_IF_FAIL )
    #define ccassert(expr) \
      smsc::util::throwIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
                #define __unreachable__(text) \
                  smsc::util::throwIfReached(text,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #elif defined ( ASSERT_ONLY_WARNING )
    #define ccassert(expr) \
      smsc::util::warningIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
                #define __unreachable__(text) \
                  smsc::util::warningIfReached(text,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #else
    //#warning "default assertion type set abort_if_fail"
    //#error "type of assertion is unknown"
    #define ccassert(expr) \
      smsc::util::abortIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
                #define __unreachable__(text) \
                  smsc::util::abortIfReached(text,__FILE__,__PRETTY_FUNCTION__,__LINE__)

  #endif
#endif

#if !defined ( WATCH_LOG_STREAM )
  #define WATCH_LOG_STREAM LOG_STREAM
#endif

#if !defined ( TRACE_LOG_STREAM )
  #define TRACE_LOG_STREAM LOG_STREAM
#endif

#if !defined ( DISABLE_TRACING )
  /*#define watch(expr) \
    smsc::util::watchImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)*/
  #define watch(expr) \
    smsc::util::watchImpl(expr,#expr,"","",__LINE__)
  #define watchx(expr) \
    smsc::util::watchxImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #define watcht(expr) \
    smsc::util::watchtImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #define watchtext(expr,len) \
    smsc::util::watchtextImpl(expr,len,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #if defined ENABLE_FILE_NAME
    #define trace2(format,...) if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG)) smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_DEBUG,format,__VA_ARGS__)
  #else
    #define trace2(format,...) if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG)) smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_DEBUG,format,__VA_ARGS__)
  #endif
  #define debug1(category,text) if(category->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG)) category->log_(smsc::logger::Logger::LEVEL_DEBUG,text)
  #define debug2(category,format,...) if(category->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG)) category->log_(smsc::logger::Logger::LEVEL_DEBUG,format,__VA_ARGS__)
  #define warn1(category,text) if(category->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN)) category->log_(smsc::logger::Logger::LEVEL_WARN,text)
  #define warn2(category,format,...) if(category->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN)) category->log_(smsc::logger::Logger::LEVEL_WARN,format,__VA_ARGS__)
  #define info1(category,text) if(category->isLogLevelEnabled(smsc::logger::Logger::LEVEL_INFO)) category->log_(smsc::logger::Logger::LEVEL_INFO,text)
  #define info2(category,format,...) if(category->isLogLevelEnabled(smsc::logger::Logger::LEVEL_INFO)) category->log_(smsc::logger::Logger::LEVEL_INFO,format,__VA_ARGS__)
#else
  #define watch(expr)
  #define watchx(expr)
  #define watcht(expr)
  #define watchtext(expr)
  #define trace2(format,...)
  #define debug1(category,text)
  #define debug2(category,format,...)
  #define warn1(category,text)
  #define warn2(category,format,...)
  #define info1(category,text)
  #define info2(category,format,...)
#endif
#define __watch__(expr)     watch(expr)
#define __watchx__(expr)    watchx(expr)
#define __watcht__(expr)    watcht(expr)
#define __watchtext__(expr) watchtext(expr)
#define __trace__(text)     trace2("%s", text)
#define __debug__(category,text)     debug1(category,text)
#define __debug2__(category,format,...)     debug2(category,format,__VA_ARGS__)
#define __warn__(category,text)     warn1(category,text)
#define __warn2__(category,format,...)     warn2(category,format,__VA_ARGS__)
#define __info__(category,text)     info1(category,text)
#define __info2__(category,format,...)     info2(category,format,__VA_ARGS__)
#define __trace2__(format,...)     trace2(format,__VA_ARGS__)
#define __trace2_if_fail__(expr,format,...) \
  {if (!expr) trace2(format,__VA_ARGS__);}
namespace smsc{
namespace util{

        class AssertException{};

  inline void abortIfFail(bool expr,const char* expr_text,
                          const char* file, const char* func, int line) throw()
  {
    if (!expr)
    {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_FATAL))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_FATAL,
         "\n*%s*<%s(%s):%d>\n\tassertin %s failed\n",
              ASSERT_LOG_DOMAIN,
              file,
              func,
              line,
              expr_text);
      abort(); /* dump core */
    }
  }
  inline void throwIfFail(bool expr,const char* expr_text,
                          const char* file, const char* func, int line)
  {
    if (!expr)
    {
      char throw_message[512];
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,"\n*%.100s*[%d]<%.100s(%.100s):%d>\n\tassertin %.100s failed\n\n",
              ASSERT_LOG_DOMAIN,
              pthread_self(),
              file,
              func,
              line,
              expr_text);
      fprintf(ASSERT_LOG_STREAM,"%s", throw_message);
      //throw throw_message;
                        throw AssertException();
    }
  }
  inline void warningIfFail(bool expr,const char* expr_text,
                            const char* file, const char* func, int line) throw()
  {
    if ( !expr )
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*%s*<%s(%s):%d>\n\tassertin %s failed\n",
              ASSERT_LOG_DOMAIN,
              file,
              func,
              line,
              expr_text);
  }

  inline void abortIfReached(const char* expr_text,
                          const char* file, const char* func, int line) throw()
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_FATAL))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_FATAL,
         "\n*%s*<%s(%s):%d>\n\t%s\n",
              UNREACHABLE_LOG_DOMAIN,
              file,
              func,
              line,
              expr_text);
      abort(); /* dump core */
  }
  inline void throwIfReached(const char* expr_text,
                          const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "\n*%.100s*[%d]<%.100s(%.100s):%d>\n\t%.100s\n\n",
              UNREACHABLE_LOG_DOMAIN,
              pthread_self(),
              file,
              func,
              line,
              expr_text);
      //throw throw_message;
      throw AssertException();
  }

  inline void warningIfReached(const char* expr_text,
                            const char* file, const char* func, int line) throw()
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*%s*<%s(%s):%d>\n%s\n",
              UNREACHABLE_LOG_DOMAIN,
              file,
              func,
              line,
              expr_text);
  }

        inline void watchImpl(bool e, const char* expr,
                        const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
       "*watch*: %s = %s     %s(%s):%d\n",
            expr,e?"true":"false",
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
            func,line);
  }

  inline void watchImpl(int e, const char* expr,
                        const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*watch*: %s = %d     %s(%s):%d\n",
            expr,e,
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
            func,line);
  }

  inline void watchImpl(unsigned int e, const char* expr,
                        const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*watch*: %s = %d     %s(%s):%d\n",
            expr,e,
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
            func,line);
  }

  inline void watchxImpl(int e, const char* expr,
                         const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*watch*: %s = %x     %s(%s):%d\n",
            expr,e,
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
            func,line);
  }

  inline void watchxImpl(unsigned int e, const char* expr,
                         const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*watch*: %s = %x     %s(%s):%d\n",
            expr,e,
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
            func,line);
  }

  inline void watchImpl(char e, const char* expr,
                        const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*watch*: %s = '%c'     %s(%s):%d\n",
            expr,e,
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
            func,line);
  }

  inline void watchImpl(void* e, const char* expr,
                        const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*watch*: %s = %p     %s(%s):%d\n",
            expr,e,
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
            func,line);
  }

  inline void watchtImpl(const char* e, const char* expr,
                         const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*watch*: %s = %s     %s(%s):%d\n",
            expr,e,
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
            func,line);
  }

  inline void watchtextImpl(const char* e, int len, const char* expr,
                            const char* file, const char* func, int line)
  {
    smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN, "*watch*: %s = '",expr);
    fwrite(e,len,1,WATCH_LOG_STREAM);
    fprintf(WATCH_LOG_STREAM,"'\t%s(%s):%d\n",
#if defined ENABLE_FILE_NAME
      file,
#else
      "",
#endif
      func,line);
  }

  inline void warningImpl(const char* e, const char* file, const char* func, int line)
  {
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,"*WARNING*: %s",e);
  }

  inline bool watchdogImpl(bool expr, const char* info, const char* e,
                           const char* file, const char* func, int line)
  {
    if ( !expr )
      if(smsc::logger::_trace_cat->isLogLevelEnabled(smsc::logger::Logger::LEVEL_WARN))
         smsc::logger::_trace_cat->log_(smsc::logger::Logger::LEVEL_WARN,
         "*%s*[%d]: %s     %s(%s):%d\n",info,pthread_self(),e,
            #if defined ENABLE_FILE_NAME
              file,
            #else
              "",
            #endif
              func,line);
    return expr;
  }
}
}

#endif

#endif /* __Cpp_Header__smsc_util_debug_h__ */
