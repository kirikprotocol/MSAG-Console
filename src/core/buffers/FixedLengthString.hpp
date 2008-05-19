#ifndef __SMSC_CORE_BUFFERS_FIXEDLENGTHSTRING_HPP__
#define __SMSC_CORE_BUFFERS_FIXEDLENGTHSTRING_HPP__

#include <string.h>
#include <string>

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

  template <int M>
  FixedLengthString& operator=(const char (&argStr)[M])
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

  bool operator!=(const char* argStr)const
  {
    return strcmp(str,argStr)!=0;
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

  char operator[](size_t index)const
  {
    return str[index];
  }
  char& operator[](size_t index)
  {
    return str[index];
  }

  void assign(const char* argStr,size_t len)
  {
    if(len<N)
    {
      memcpy(str,argStr,len);
      str[len]=0;
    }else
    {
      memcpy(str,argStr,N-1);
      str[N-1]=0;
    }
  }

  operator std::string()const
  {
    return str;
  }

  const char* c_str()const
  {
    return str;
  }
  size_t length()const
  {
    return strlen(str);
  }
};

}//namespace buffers
}//namespace core
}//namespace smsc


#endif
