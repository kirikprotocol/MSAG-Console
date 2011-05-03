#ifndef __SMSC_UTIL_SMSTEXT_H__
#define __SMSC_UTIL_SMSTEXT_H__

#include "sms/sms.h"
#include "smpp/smpp_mandatory.h"
#include "util/recoder/recode_dll.h"
#include "core/buffers/Array.hpp"
#include "util/debug.h"


namespace smsc{
namespace util{

using namespace smsc::sms;
using namespace smsc::smpp;
// using namespace smsc::core::buffers;

inline int getSmsText(SMS* sms,char* buf,size_t bufsize,ConvEncodingEnum enc=CONV_ENCODING_CP1251)
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
    if(len/2>=bufsize)return -((int)len/2+1);
    ConvertUCS2ToMultibyte((const short*)data,len,buf,bufsize,enc);
    len/=2;
  }else if(coding==DataCoding::SMSC7BIT)
  {
    if(len>=bufsize)return -(int)(len+1);
    len=ConvertSMSC7BitToLatin1(data,len,buf);
  }
  else
  {
    if(len>=bufsize)return -(int)(len+1);
    memcpy(buf,data,len);
  }
  buf[len]=0;
  return len;
}

inline int getSmsText(SMS* sms,string& res,ConvEncodingEnum enc=CONV_ENCODING_CP1251)
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


static inline int getPduText(PduXSm* pdu,char* buf,size_t bufsize)
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

  if(len==0)
  {
    buf[0]=0;
    return 0;
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
    delete [] tmp;
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

static inline bool hasHighBit(const char* buf,size_t len)
{
  for(size_t i=0;i<len;i++)
  {
    if((((unsigned char)buf[i])&0x80)!=0)return true;
  }
  return false;
}

int splitSms(SMS* tmplSms,const char *text,size_t length,ConvEncodingEnum encoding,int datacoding,
             smsc::core::buffers::Array<SMS*>& dest);

size_t fillSms(SMS* sms,const char *text,size_t length,ConvEncodingEnum encoding,int datacoding,int trimLen=65535);

void transLiterateSms(SMS* sms,int datacoding);

inline bool splitString(/*in,out*/string& head,/*out*/string& tail)
{
  string::size_type pos=head.find(' ');
  if(pos==string::npos)return false;
  string::size_type firstPos=pos;
  while(head[pos]==' ')pos++;
  tail=head.substr(pos);
  head.erase(firstPos);
  return true;
}

enum{
  psSingle,
  psMultiple,
  psErrorLength,
  psErrorUdhi,
  psErrorUssd
};

#pragma pack(1)
struct ConcatInfo{
  uint8_t num;
  uint16_t off[1];
  void setOff(int idx,uint16_t val)
  {
    val=htons(val);
    memcpy(off+idx,&val,sizeof(val));
  }
  uint16_t getOff(int idx)
  {
    uint16_t val;
    memcpy(&val,off+idx,sizeof(val));
    val=ntohs(val);
    return val;
  }
};
#pragma pack()

int partitionSms(SMS* sms);
bool extractSmsPart(SMS* sms,int partnum);

inline bool smsCanBeTransliterated(SMS* sms)
{
  int dc=sms->getIntProperty(Tag::SMPP_DATA_CODING);
  if(dc!=DataCoding::UCS2)return false;
  unsigned char* body;
  unsigned len;
  if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    body=(unsigned char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }else
  {
    body=(unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  }
  if((sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)==0x40)
  {
    int udhlen=*body;
    body+=udhlen+1;
    len-=udhlen+1;
  }
  unsigned short* ucsbody=(unsigned short*)body;
  len/=2;
  for(unsigned i=0;i<len;i++)
  {
    uint16_t chr;
    memcpy(&chr,ucsbody+i,2);
    if((chr&0xff00)!=0x0000 && (chr&0xff00)!=0x0400)
    {
      __trace2__("cantranslit=false, %x",(unsigned int)chr);
      return false;
    }
  }
  return true;
}

}
}

#endif
