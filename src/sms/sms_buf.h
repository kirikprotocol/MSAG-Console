#ifndef __SMSC_SMS_SMSBUF_H__
#define __SMSC_SMS_SMSBUF_H__

#include <string.h>
#include <string>

namespace smsc{
namespace sms{

class SmsPropertyBuf{
public:
  SmsPropertyBuf():buf(0),strlength(0),size(0){}
  SmsPropertyBuf(const char* str):buf(0),strlength(0),size(0)
  {
    assign(str,strlen(str));
  }
  SmsPropertyBuf(const SmsPropertyBuf& rhs)
  {
    strlength=rhs.strlength;
    size=strlength+1;
    buf=new char[size];
    memcpy(buf,rhs.buf,strlength);
    buf[strlength]=0;
  }
  SmsPropertyBuf& operator=(const SmsPropertyBuf& rhs)
  {
    assign(rhs.c_str(),rhs.length());
    return *this;
  }
  SmsPropertyBuf& operator=(const char* str)
  {
    assign(str,strlen(str));
    return *this;
  }
  operator std::string ()const
  {
    return std::string(buf?buf:"",strlength);
  }
  ~SmsPropertyBuf()
  {
    if(buf)delete [] buf;
  }
  bool operator==(const char* str)const
  {
    return buf && str && strcmp(buf,str)==0;
  }
  void assign(const char* str,size_t len)
  {
    if(len>=size)
    {
      if(buf)delete [] buf;
      buf=new char[len+1];
      size=len+1;
    }
    memcpy(buf,str,len);
    strlength=len;
    buf[strlength]=0;
  }
  size_t length()const
  {
    return strlength;
  }
  const char* c_str()const
  {
    return buf?buf:"";
  }
  const char* data()const
  {
    return buf?buf:"";
  }

protected:
  char* buf;
  size_t strlength;
  size_t size;
};

}//sms
}//smsc

#endif
