#ifndef __SMSC_UTIL_SMSTEXT_H__
#define __SMSC_UTIL_SMSTEXT_H__

#include "sms/sms.h"
#include "smpp/smpp.h"
#include "util/recoder/recode_dll.h"
#include "core/buffers/Array.hpp"


namespace smsc{
namespace util{

using namespace smsc::sms;
using namespace smsc::smpp;
using namespace smsc::core::buffers;

static inline int getSmsText(SMS* sms,char* buf)
{
  int coding = sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING);
  //int len = sms->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
  unsigned len;
  const char *data=sms->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,&len);
  if(coding==DataCoding::UCS2)
  {
    ConvertUCS2ToMultibyte((const short*)data,len,buf,MAX_SHORT_MESSAGE_LENGTH,CONV_ENCODING_ANSI);
    len/=2;
  }else
  {
    memcpy(buf,data,len);
  }
  buf[len]=0;
  return len;
}

static inline int getPduText(PduXSm* pdu,char* buf)
{
  int coding=pdu->get_message().get_dataCoding();
  unsigned len=pdu->get_message().get_smLength();
  const char *data=pdu->get_message().get_shortMessage();
  if(coding==DataCoding::UCS2)
  {
    ConvertUCS2ToMultibyte((const short*)data,len,buf,MAX_SHORT_MESSAGE_LENGTH,CONV_ENCODING_ANSI);
    len/=2;
  }else
  {
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

};
};

#endif
