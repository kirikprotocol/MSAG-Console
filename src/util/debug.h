// $Id$
/*
	этот файл содержит код для проверки условий.
*/

#include <stdio.h>
#include <stdlib.h>	
	
#if !defined ( __Cpp_Header__smsc_util_debug_h__ )
#define __Cpp_Header__smsc_util_debug_h_

#if defined ccassert
	#error "opps, ccassert alreadry defined"
#endif

#define require(expr) ccassert(expr)	

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

#if defined ( DISABLE_ANY_CHECKS )
	#define ccassert(expr)
#else
	#if defined ( ASSERT_ABORT_IF_FAIL )
		#define ccassert(expr) smsc::util::abortIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
	#elif defined ( ASSERT_THROW_IF_FAIL )
		#define ccassert(expr) smsc::util::throwIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
	#elif defined ( ASSERT_ONLY_WARNING )
		#define ccassert(expr) smsc::util::warningIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
	#else
		//#warning "default assertion type set abort_if_fail"
		//#error "type of assertion is unknown"
		#define ccassert(expr) smsc::util::abortIfFail(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
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
	#define watch(expr) smsc::util::watchImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
	#define watchx(expr) smsc::util::watchxImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
	#define watcht(expr) smsc::util::watchtImpl(expr,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
	#define watchtext(expr,len) smsc::util::watchtextImpl(expr,len,#expr,__FILE__,__PRETTY_FUNCTION__,__LINE__)
	#define trace(text)	smsc::util::traceImpl(text,__FILE__,__PRETTY_FUNCTION__,__LINE__)
#else
	#define watch(expr)
	#define watchx(expr)
	#define watcht(expr)
	#define watchtext(expr)
	#define trace(text)
#endif

namespace smsc
{
namespace util
{
	inline void abortIfFail(bool expr,const char* expr_text,const char* file, const char* func, int line) throw() 
	{
		if (!expr)
		{
			fprintf(ASSERT_LOG_STREAM,"\n*%s*:%s(%s):%d:  assertin %s failed\n",
							ASSERT_LOG_DOMAIN,
							file,
							func,
							line,
							expr_text);
			abort(); /* dump core */
		}
	}
	inline void throwIfFail(bool expr,const char* expr_text,const char* file, const char* func, int line) throw(const char*)
	{
		if (!expr)
		{
			char throw_message[512];
			sprintf(throw_message,"\n*%.100s*:%.100s(%.100s):%d,  assertin %.100s failed\n\n",
							ASSERT_LOG_DOMAIN,
							file,
							func,
							line,
							expr_text);
			fprintf(ASSERT_LOG_STREAM,throw_message);
			throw throw_message;
		}
	}
	inline void warningIfFail(bool expr,const char* expr_text,const char* file, const char* func, int line) throw() 
	{
		if (!expr)
		{
			fprintf(ASSERT_LOG_STREAM,"*%s*:%s(%s):%d,  assertin %s failed\n",
							ASSERT_LOG_DOMAIN,
							file,
							func,
							line,
							expr_text);
		}
	}
	
	inline void watchImpl(bool e, const char* expr, const char* file, const char* func, int line)
	{
		fprintf(WATCH_LOG_STREAM,"*watch*: %s = %s\t:%s(%s):%d\n",expr,e?"true":"false",file,func,line);
	}

	inline void watchImpl(int e, const char* expr, const char* file, const char* func, int line)
	{
		fprintf(WATCH_LOG_STREAM,"*watch*: %s = %d\t:%s(%s):%d\n",expr,e,file,func,line);
	}
	
	inline void watchxImpl(int e, const char* expr, const char* file, const char* func, int line)
	{
		fprintf(WATCH_LOG_STREAM,"*watch*: %s = %x\t:%s(%s):%d\n",expr,e,file,func,line);
	}
	
	inline void watchImpl(char e, const char* expr, const char* file, const char* func, int line)
	{
		fprintf(WATCH_LOG_STREAM,"*watch*: %s = '%c'\t:%s(%s):%d\n",expr,e,file,func,line);
	}

	inline void watchImpl(void* e, const char* expr, const char* file, const char* func, int line)
	{
		fprintf(WATCH_LOG_STREAM,"*watch*: %s = %p\t:%s(%s):%d\n",expr,e,file,func,line);
	}

	inline void watchtImpl(const char* e, const char* expr, const char* file, const char* func, int line)
	{
		fprintf(WATCH_LOG_STREAM,"*watch*: %s = %s\t:%s(%s):%d\n",expr,e,file,func,line);
	}

	inline void watchtextImpl(const char* e, int len, const char* expr, const char* file, const char* func, int line)
	{
		fprintf(WATCH_LOG_STREAM,"*watch*: %s = '",expr);
		fwrite(e,len,1,WATCH_LOG_STREAM);
		fprintf(WATCH_LOG_STREAM,"'\t:%s(%s):%d\n",file,func,line);
	}
	
	inline void traceImpl(const char* e, const char* file, const char* func, int line)
	{
		fprintf(TRACE_LOG_STREAM,"*trace*: %s\t:%s(%s):%d\n",e,file,func,line);
	}
};
};

#endif /* __Cpp_Header__smsc_util_debug_h__ */

