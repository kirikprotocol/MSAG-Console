#ifndef __SMSC_CORE_BUFFERS_FIXEDLENGTHSTRING_HPP__
#define __SMSC_CORE_BUFFERS_FIXEDLENGTHSTRING_HPP__

#include <string.h>

namespace smsc{
namespace core{
namespace buffers{

template <int N>
struct FixedLengthString{
  char str[N];
  FixedLengthString()
  {
    str[0]=0;
  }
  FixedLengthString(const char* argStr)
  {
    *this=argStr;
  }
  template <int M>
  FixedLengthString(const FixedLengthString<M>& argStr)
  {
    *this=argStr.str;
  }
  FixedLengthString& operator=(const char* argStr)
  {
    strncpy(str,argStr,N);
    str[N-1]=0;
    return *this;
  }

  template <class StdString>
  FixedLengthString& operator=(const StdString& argStr)
  {
    *this=argStr.c_str();
    return *this;
  }

  template <int M>
  bool operator==(const FixedLengthString<M>& argStr)const
  {
    return !strcmp(str,argStr.str);
  }

  bool operator==(const char* argStr)const
  {
    return !strcmp(str,argStr);
  }

  template <int M>
  bool operator<(const FixedLengthString<M>& argStr)const
  {
    return strcmp(str,argStr.str)<0;
  }


  bool operator<(const char* argStr)const
  {
    return strcmp(str,argStr)<0;
  }

  operator char*()
  {
    return str;
  }
  operator const char*()const
  {
    return str;
  }

  char operator[](int index)const
  {
    return str[index];
  }
  char& operator[](int index)
  {
    return str[index];
  }

  const char* c_str()const
  {
    return str;
  }
};

}//namespace buffers
}//namespace core
}//namespace smsc


#endif
