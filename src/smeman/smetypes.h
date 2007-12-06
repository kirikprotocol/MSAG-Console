/*
  $Id$
*/

#if !defined __Cpp_Header__smeman_smetyes_h__
#define __Cpp_Header__smeman_smetyes_h__

#include <string>
#include <inttypes.h>
#include <exception>

namespace smsc {
namespace smeman {

//typedef const char* SmeSystemId;
typedef std::string SmeSystemId;
typedef std::string SmePassword;
typedef int32_t SmeIndex;
typedef int SmeNType;

const int32_t INVALID_SME_INDEX = -1;

const int32_t MAX_SME_PROXIES=2000;


class SmeError : public std::exception
{
  virtual const char* what() const  throw()
  {
    return "SmeError";
  }
};

}
}


#endif
