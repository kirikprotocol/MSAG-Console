#ifndef __UTIL_EXCEPTION_HPP__
#define __UTIL_EXCEPTION_HPP__

#include <exception>
#include <string>
#include <stdarg.h>
#include <stdio.h>

#define SMSC_UTIL_EX_FILL(fmt) \
    va_list arglist;\
    va_start(arglist,fmt);\
    fill(fmt,arglist);\
    va_end(arglist);


namespace smsc{
namespace util{

class Exception:public std::exception{
public:
  Exception(){}
  Exception(const char* fmt,...)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
  inline void fill(const char* fmt,va_list arglist)
  {
    int size=strlen(fmt)*4;
    char *buf=new char[size];
    int res;
    res=vsnprintf( buf, size,fmt,arglist);
    if(res>size)
    {
      delete [] buf;
      size=res+4;
      buf=new char[size];
      res=vsnprintf( buf, size,fmt,arglist);
    }
    message=buf;
    delete [] buf;
  }
  virtual ~Exception()throw(){}
  virtual const char* what()const throw(){return message.c_str();}
protected:
  std::string message;
};

};//util
};//smsc

#endif
