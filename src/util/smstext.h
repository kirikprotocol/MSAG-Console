#ifndef __SMSC_UTIL_SMSTEXT_H__
#define __SMSC_UTIL_SMSTEXT_H__

#include "sms/sms.h"
#include "smpp/smpp.h"
#include "util/recoder/recode_dll.h"
#include "core/buffers/Array.hpp"
#include "util/debug.h"


namespace smsc{
namespace util{

using namespace smsc::sms;
using namespace smsc::smpp;
using namespace smsc::core::buffers;

static inline int getSmsText(SMS* sms,char* buf,unsigned bufsize)
{
  int coding = sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING);
  //int len = sms->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
  unsigned len;
  const char *data=sms->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,&len);
  if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)==0x40)
  {
    int l=(unsigned char)*data;
    data+=l;
    len-=l;
  }
  __trace2__("getSmsText: dc=%d, len=%d",coding,len);
  if(coding==DataCoding::UCS2)
  {
    ConvertUCS2ToMultibyte((const short*)data,len,buf,bufsize,CONV_ENCODING_CP1251);
    len/=2;
    __require__(len<bufsize);
  }else if(coding==DataCoding::SMSC7BIT)
  {
    len=ConvertSMSC7BitToLatin1(data,len,buf);
    __require__(len<bufsize);
  }
  else
  {
    __require__(len<bufsize);
    memcpy(buf,data,len);
  }
  buf[len]=0;
  return len;
}

static inline int getPduText(PduXSm* pdu,char* buf,unsigned bufsize)
{
  int coding=pdu->get_message().get_dataCoding();
  unsigned len=pdu->get_message().get_smLength();
  const char *data=pdu->get_message().get_shortMessage();
  if(coding==DataCoding::UCS2)
  {
    ConvertUCS2ToMultibyte((const short*)data,len,buf,bufsize,CONV_ENCODING_CP1251);
    len/=2;
    __require__(len<bufsize);
  }else if(coding==DataCoding::SMSC7BIT)
  {
    len=ConvertSMSC7BitToLatin1(data,len,buf);
    __require__(len<bufsize);
  }
  else
  {
    __require__(len<bufsize);
    memcpy(buf,data,len);
  }
  buf[len]=0;
  return len;
}

static inline bool hasHighBit(const char* buf,int len)
{
  for(int i=0;i<len;i++)
  {
    if((((unsigned char)buf[i])&0x80)!=0)return true;
  }
  return false;
}

int splitSms(SMS* tmplSms,const char *text,int length,ConvEncodingEnum encoding,int datacoding,
             Array<SMS*>& dest);

int trimSms(SMS* sms,const char *text,int length,ConvEncodingEnum encoding,int datacoding);

void transLiterateSms(SMS* sms);

};
};

#endif
