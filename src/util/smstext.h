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

static inline int getSmsText(SMS* sms,char* buf,unsigned bufsize,ConvEncodingEnum enc=CONV_ENCODING_CP1251)
{
  int coding = sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING);
  //int len = sms->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
  unsigned len;
  const char *data=sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  if(len==0 && sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    data=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }
  if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)==0x40)
  {
    int l=(unsigned char)*data;
    data+=l+1;
    len-=l+1;
  }
  __trace2__("getSmsText: dc=%d, len=%d",coding,len);
  if(coding==DataCoding::UCS2)
  {
    if(len/2>=bufsize)return -len/2;
    ConvertUCS2ToMultibyte((const short*)data,len,buf,bufsize,enc);
    len/=2;
  }else if(coding==DataCoding::SMSC7BIT)
  {
    if(len>=bufsize)return -len;
    len=ConvertSMSC7BitToLatin1(data,len,buf);
  }
  else
  {
    if(len>=bufsize)return -len;
    memcpy(buf,data,len);
  }
  buf[len]=0;
  return len;
}

static inline int getSmsText(SMS* sms,string& res,ConvEncodingEnum enc=CONV_ENCODING_CP1251)
{
  int coding = sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING);
  //int len = sms->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
  unsigned len;
  const char *data=sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  if(len==0 && sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    data=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }
  if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)==0x40)
  {
    int l=(unsigned char)*data;
    data+=l+1;
    len-=l+1;
  }
  __trace2__("getSmsText: dc=%d, len=%d",coding,len);
  if(coding==DataCoding::UCS2)
  {
    int bsz=len/2+2;
    auto_ptr<char> buf(new char[bsz]);
    ConvertUCS2ToMultibyte((const short*)data,len,buf.get(),bsz,enc);
    len/=2;
    res.assign(buf.get(),len);
  }else if(coding==DataCoding::SMSC7BIT)
  {
    int bsz=len*2+2;
    auto_ptr<char> buf(new char[bsz]);
    len=ConvertSMSC7BitToLatin1(data,len,buf.get());
    res.assign(buf.get(),len);
  }
  else
  {
    res.assign(data,len);
  }
  return len;
}


static inline int getPduText(PduXSm* pdu,char* buf,unsigned bufsize)
{
  int coding;
  unsigned len;
  const char* data;
  bool udhi;

  if(pdu->get_header().get_commandId()==SmppCommandSet::DATA_SM)
  {
    PduDataSm *dpdu=(PduDataSm *)pdu;
    coding=dpdu->get_data().get_dataCoding();
    len=dpdu->optional.size_messagePayload();
    data=dpdu->optional.get_messagePayload();
    udhi=dpdu->get_data().get_esmClass()&0x40;
  }else
  {
    coding=pdu->get_message().get_dataCoding();
    len=pdu->get_message().get_smLength();
    data=pdu->get_message().get_shortMessage();
    if(len==0 && pdu->optional.has_messagePayload())
    {
      len=pdu->optional.size_messagePayload();
      data=pdu->optional.get_messagePayload();
    }
    udhi=pdu->get_message().get_esmClass()&0x40;
  }

  if(udhi)
  {
    int udhilen=*((unsigned char*)data);
    udhilen++;
    data+=udhilen;
    len-=udhilen;
  }
  if(coding==DataCoding::UCS2)
  {
    if(len/2>bufsize)return -1;
    char *tmp=new char[len+1];
    UCS_ntohs(tmp,data,len,0);
    ConvertUCS2ToMultibyte((const short*)tmp,len,buf,bufsize,CONV_ENCODING_CP1251);
    delete tmp;
    len/=2;
  }else if(coding==DataCoding::SMSC7BIT)
  {
    if(len>bufsize)return -1;
    len=ConvertSMSC7BitToLatin1(data,len,buf);
  }
  else
  {
    if(len>bufsize)return -1;
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

int fillSms(SMS* sms,const char *text,int length,ConvEncodingEnum encoding,int datacoding,int trimLen=65535);

void transLiterateSms(SMS* sms,int datacoding);

enum{
  psSingle,
  psMultiple,
  psErrorLength,
  psErrorUdhi,
  psErrorUssd,
};

#pragma pack(1)
struct ConcatInfo{
  uint8_t num;
  uint16_t off[1];
};
#pragma pack()

int partitionSms(SMS* sms,int dstdc);
void extractSmsPart(SMS* sms,int partnum);

}
}

#endif
