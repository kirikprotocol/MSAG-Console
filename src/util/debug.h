// $Id$
/*
  этот файл содержит код для проверки условий.
*/

#include <stdio.h>
#include <stdlib.h> 
  
#if !defined ( __Cpp_Header__smsc_util_debug_h__ )
#define __Cpp_Header__smsc_util_debug_h__

#if defined ccassert
  #error "opps, ccassert alreadry defined"
#endif

#if defined DISABLE_ANY_CHECKS
  #warning "any chacks will disabled, it is very dungrouse mode"
  #define DISABLE_HARD_CHECKS
  #define DISABLE_SOFT_CHECKS
  #define DISABLE_WATCHDOG
  #define DISABLE_TRACING
#endif

#define require(expr) ccassert(expr)  
#define __require__(expr) ccassert(expr)  

// very hard checks and can't be disabled
#define __abort_if_fail__(expr) \
  smsc::util::abortIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
#define __warning__(text) \
  smsc::util::warningImpl(text,__FILE__,__PRETTY_FUNCTION__,__LINE__)


#if !defined DISABLE_WATCHDOG
  #define __watchdog__(expr) __watchdog2__(expr,"GAW-GAW")
  #define __watchdog2__(expr,info) \
    smsc::util::watchdogImpl(expr,info,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
#else
  #define __watchdog__(expr) (expr)
  #define __watchdog2__(expr,info) (expr)
#endif

#if !defined DISABLE_SOFT_CHECKS
  #define __goto_if_fail__(expr,label) \
    if ( !__watchdog2__(expr,"goto :" #label) ) goto label; else;
  #define __throw_if_fail__(expr,excep) \
    if ( !__watchdog2__(expr,"throw " #excep) ) throw excep(); else;
  #define __ret_if_fail__(expr) \
    if ( !__watchdog2__(expr,"return")) return; else;
  #define __ret0_if_fail__(expr) \
    if ( !__watchdog2__(expr,"return 0")) return 0; else;
  #define __ret_val_if_fail__(expr,val) \
    if ( !__watchdog2(expr,"return " #val) return val; else;
#else
  #define __goto_if_fail__(expr,label)
  #define __ret_if_fail__(expr)
  #define __ret0_if_fail__(expr)
  #define __ret_val_if_fail__(expr,val)
#endif

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

#if !defined ( ASSERT_LOG_STREAM )
  #define ASSERT_LOG_STREAM LOG_STREAM 
#endif

#if defined ( DISABLE_HARD_CHECKS )
  #define ccassert(expr)
#else
  #if defined ( ASSERT_ABORT_IF_FAIL )
    #define ccassert(expr) \
      smsc::util::abortIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #elif defined ( ASSERT_THROW_IF_FAIL )
    #define ccassert(expr) \
      smsc::util::throwIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #elif defined ( ASSERT_ONLY_WARNING )
    #define ccassert(expr) \
      smsc::util::warningIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #else
    //#warning "default assertion type set abort_if_fail"
    //#error "type of assertion is unknown"
    #define ccassert(expr) \
      smsc::util::abortIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #endif
#endif

#if defined ( trace )
  #undef trace
#endif

#if !defined ( WATCH_LOG_STREAM )
  #define WATCH_LOG_STREAM LOG_STREAM
#endif

#if !defined ( TRACE_LOG_STREAM )
  #define TRACE_LOG_STREAM LOG_STREAM
#endif

#if !defined ( DISABLE_TRACING )
  #define watch(expr) \
    smsc::util::watchImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #define watchx(expr) \
    smsc::util::watchxImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #define watcht(expr) \
    smsc::util::watchtImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #define watchtext(expr,len) \
    smsc::util::watchtextImpl(expr,len,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
  #define trace(text) trace2("%s",text)
  #if defined ENABLE_FILE_NAME  
    #define trace2(format,args...) \
      fprintf(TRACE_LOG_STREAM,\
        "*trace*: " format " \n\t"__FILE__"(%s):%d\n",\
        ##args,__PRETTY_FUNCTION__,__LINE__);
  #else     
    #define trace2(format,args...) \
      fprintf(TRACE_LOG_STREAM,\
        "*trace*: " format "     (%s):%d\n",\
        ##args,__PRETTY_FUNCTION__,__LINE__);
  #endif      
#else
  #define watch(expr)
  #define watchx(expr)
  #define watcht(expr)
  #define watchtext(expr)
  #define trace(text)
  #define trace2(format,args...)
#endif
#define __watch__(expr)     watch(expr)
#define __watchx__(expr)    watchx(expr)
#define __watcht__(expr)    watcht(expr)
#define __watchtext__(expr) watchtext(expr)
#define __trace__(text)     trace(text)
#define __trace2__(format,args...)     trace2(format,##args)

namespace smsc
{
namespace util
{
  inline void abortIfFail(bool expr,const char* expr_text,
                          const char* file, const char* func, int line) throw() 
  {
    if (!expr)
    {
      fprintf(ASSERT_LOG_STREAM,"\n*%s*<%s(%s):%d>\n\tassertin %s failed\n",
              ASSERT_LOG_DOMAIN,
              file,
              func,
              line,
              expr_text);
      abort(); /* dump core */
    }
  }
  inline void throwIfFail(bool expr,const char* expr_text,
                          const char* file, const char* func, int line) throw(const char*)
  {
    if (!expr)
    {
      char throw_message[512];
      sprintf(throw_message,"\n*%.100s*<%.100s(%.100s):%d>\n\tassertin %.100s failed\n\n",
              ASSERT_LOG_DOMAIN,
              file,
              func,
              line,
              expr_text);
      fprintf(ASSERT_LOG_STREAM,throw_message);
      throw throw_message;
    }
  }
  inline void warningIfFail(bool expr,const char* expr_text,
                            const char* file, const char* func, int line) throw() 
  {
    if (!expr)
    {
      fprintf(ASSERT_LOG_STREAM,"*%s*<%s(%s):%d>\n\tassertin %s failed\n",
              ASSERT_LOG_DOMAIN,
              file,
              func,
              line,
              expr_text);
    }
  }
  
  inline void watchImpl(bool e, const char* expr, 
                        const char* file, const char* func, int line)
  {
    fprintf(WATCH_LOG_STREAM,"*watch*: %s = %s     %s(%s):%d\n",
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
    fprintf(WATCH_LOG_STREAM,"*watch*: %s = %d     %s(%s):%d\n",
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
    fprintf(WATCH_LOG_STREAM,"*watch*: %s = %x     %s(%s):%d\n",
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
    fprintf(WATCH_LOG_STREAM,"*watch*: %s = '%c'     %s(%s):%d\n",
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
    fprintf(WATCH_LOG_STREAM,"*watch*: %s = %p     %s(%s):%d\n",
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
    fprintf(WATCH_LOG_STREAM,"*watch*: %s = %s     %s(%s):%d\n",
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
    fprintf(WATCH_LOG_STREAM,"*watch*: %s = '",expr);
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
    fprintf(TRACE_LOG_STREAM,"*WARNING*: %s\n\t%s(%s):%d\n",e,
            #if defined ENABLE_FILE_NAME              
              file,
            #else
              "",
            #endif
            func,line);
  }
  
  inline bool watchdogImpl(bool expr, const char* info, const char* e, 
                           const char* file, const char* func, int line)
  {
    if ( !expr )
      fprintf(TRACE_LOG_STREAM,"*%s*: %s     %s(%s):%d\n",info,e,
            #if defined ENABLE_FILE_NAME              
              file,
            #else
              "",
            #endif
              func,line);
    return expr;
  }
};
};

#endif /* __Cpp_Header__smsc_util_debug_h__ */

