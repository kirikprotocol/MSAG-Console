#ifndef __UTIL_EXCEPTION_HPP__
#define __UTIL_EXCEPTION_HPP__

#include <exception>
#include <string>
#include <stdarg.h>
#include <stdio.h>

namespace smsc{
namespace util{

class Exception:public std::exception{
public:
  Exception(const char* fmt,...)
  {
    int size=strlen(fmt)*4;
    char *buf=new char[size];
    va_list arglist;
    int res;
    do{
      va_start(arglist,fmt);
      res=vsnprintf( buf, size,fmt,arglist);
      va_end(arglist);
      if(res<0)
      {
        delete [] buf;
        size-=res;
        size+=4;
        buf=new char[size];
      }
    }while(res<0);
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
