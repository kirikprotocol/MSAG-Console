#ifndef __SMSC_CORE_BUFFERS_FIXEDLENGTHSTRING_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CORE_BUFFERS_FIXEDLENGTHSTRING_HPP__

#include <string.h>
#include <string>

namespace smsc{
namespace core{
namespace buffers{

template <size_t N>
struct FixedLengthString{
    static const size_t npos = N;
    static const size_t MAX_SZ = N;

  char str[N];
  FixedLengthString()
  {
    str[0]=0;
  }
  FixedLengthString(const char* argStr)
  {
    if(argStr==0)
    {
      str[0]=0;
      return;
    }
    *this=argStr;
  }
  template <size_t M>
  FixedLengthString(const FixedLengthString<M>& argStr)
  {
    *this=argStr.str;
  }

  FixedLengthString& operator+=(const char arg_char)
  {
    size_t len = strlen(str);
    if ((len + 1) < N)
    {
      str[len] = arg_char;
      str[len + 1] = 0;
    }
    return *this;
  }

  FixedLengthString& operator+=(const char* argStr)
  {
    strncat(str, argStr, N-strlen(str)-1);
    return *this;
  }

  template <size_t M>
  FixedLengthString& operator+=(const FixedLengthString<M>& argStr)
  {
    return operator+=(argStr.str);
  }

  FixedLengthString& operator=(const char* argStr)
  {
    strncpy(str,argStr,N-1);
    str[N-1]=0;
    return *this;
  }

  template <size_t M>
  FixedLengthString& operator=(const char (&argStr)[M])
  {
    strncpy(str,argStr,N-1);
    str[N-1]=0;
    return *this;
  }

  FixedLengthString& operator=(const std::string& argStr)
  {
    *this=argStr.c_str();
    return *this;
  }

  template <size_t M>
  bool operator==(const FixedLengthString<M>& argStr)const
  {
    return !strcmp(str,argStr.str);
  }

  bool operator==(const char* argStr)const
  {
    return !strcmp(str,argStr);
  }

  bool operator==(const std::string& argStr)const
  {
    return argStr==str;
  }

  bool operator!=(const char* argStr)const
  {
    return strcmp(str,argStr)!=0;
  }

  template <size_t M>
  bool operator!=(const FixedLengthString<M>& argStr)const
  {
    return strcmp(str,argStr.str)!=0;
  }

  template <size_t M>
  bool operator<(const FixedLengthString<M>& argStr)const
  {
    return strcmp(str,argStr.str)<0;
  }


  bool operator<(const char* argStr)const
  {
    return strcmp(str,argStr)<0;
  }

  /*operator char*()
  {
    return str;
  }
  operator const char*()const
  {
    return str;
  }*/

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
  void clear()
  {
    str[0]=0;
  }
  bool empty() const
  {
    return !str[0];
  }
  size_t capacity() const { return N; }

  template <size_t M>
  void swap( FixedLengthString<M>& s )
  {
    if ( &s == this )
    {
      return;
    }
    char tmpbuf[M];
    size_t minlen = std::min(M,N);
    memcpy(tmpbuf,str,minlen);
    memcpy(str,s.str,minlen);
    memcpy(s.str,tmpbuf,minlen);
    s.str[--minlen] = '\0';
    str[minlen] = '\0';
  }

};

template< size_t N >
const size_t FixedLengthString< N >::npos;

template< size_t N >
const size_t FixedLengthString< N >::MAX_SZ;

}//namespace buffers
}//namespace core
}//namespace smsc


#endif
