#ifndef __UTIL_EXCEPTION_HPP__
#define __UTIL_EXCEPTION_HPP__

#include <exception>
#include <string>

namespace smsc{
namespace util{

class Exception:public std::exception{
public:
  Exception(const char* msg):message(msg){}
  virtual ~Exception()throw(){}
  virtual const char* what()const throw(){return message.c_str();}
protected:
  std::string message;
};

};//util
};//smsc

#endif
