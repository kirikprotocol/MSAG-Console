#ifndef __SMSC_SMS_SERIALIZER_H__
#define __SMSC_SMS_SERIALIZER_H__

#include "sms/sms.h"
#include "core/buffers/TmpBuf.hpp"
#include "util/Exception.hpp"

namespace smsc{
namespace sms{

namespace BufOps{

  typedef smsc::core::buffers::TmpBuf<char,2048> SmsBuffer;
  inline SmsBuffer& operator<<(SmsBuffer& buf,const Address& addr)
  {
    buf.Append((char*)&addr.length,1);
    buf.Append((char*)&addr.type,1);
    buf.Append((char*)&addr.plan,1);
    buf.Append(addr.value,addr.length);
    return buf;
  }

  inline SmsBuffer& operator>>(SmsBuffer& buf,Address& addr)
  {
    buf.Read((char*)&addr.length,1);
    buf.Read((char*)&addr.type,1);
    buf.Read((char*)&addr.plan,1);
    buf.Read(addr.value,addr.length);
    addr.value[addr.length]=0;
    return buf;
  }

  inline SmsBuffer& operator<<(SmsBuffer& buf,const char* str)
  {
    uint8_t len=(uint8_t)strlen(str);
    buf.Append((char*)&len,1);
    buf.Append(str,len);
    return buf;
  }

  struct SmallCharBuf{
    char* buf;
    size_t len;
    SmallCharBuf(char* argBuf,size_t argLen):buf(argBuf),len(argLen){}
  };

  inline SmsBuffer& operator>>(SmsBuffer& buf,SmallCharBuf& scb)
  {
    uint8_t len;
    buf.Read((char*)&len,1);
    if(len>scb.len)throw smsc::util::Exception("Attempt to read %d byte in buffer with size %d",(int)len,(int)scb.len);
    buf.Read(scb.buf,len);
    scb.buf[len]=0;
    return buf;
  }

  inline SmsBuffer& operator<<(SmsBuffer& buf,const uint32_t& val)
  {
    uint32_t nval=ntohl(val);
    buf.Append((char*)&nval,4);
    return buf;
  }

  inline SmsBuffer& operator>>(SmsBuffer& buf,uint32_t& val)
  {
    buf.Read((char*)&val,4);
    val=htonl(val);
    return buf;
  }

  inline SmsBuffer& operator<<(SmsBuffer& buf,const int32_t& val)
  {
    uint32_t nval=ntohl(val);
    buf.Append((char*)&nval,4);
    return buf;
  }

  inline SmsBuffer& operator>>(SmsBuffer& buf,int32_t& val)
  {
    buf.Read((char*)&val,4);
    val=htonl(val);
    return buf;
  }

  inline SmsBuffer& operator<<(SmsBuffer& buf,const uint64_t& val)
  {
    uint32_t nval=ntohl((uint32_t)(val>>32)&0xFFFFFFFF);
    buf.Append((char*)&nval,4);
    nval=ntohl((uint32_t)(val&0xFFFFFFFF));
    buf.Append((char*)&nval,4);
    return buf;
  }

  inline SmsBuffer& operator>>(SmsBuffer& buf,uint64_t& val)
  {
    uint32_t nval;
    buf.Read((char*)&nval,4);
    val=htonl(nval);
    val<<=32;
    buf.Read((char*)&nval,4);
    nval=htonl(nval);
    val|=nval;
    return buf;
  }


  inline SmsBuffer& operator<<(SmsBuffer& buf,time_t val)
  {
    uint32_t tmp=ntohl((uint32_t)val);
    buf.Append((char*)&tmp,4);
    return buf;
  }

  inline SmsBuffer& operator>>(SmsBuffer& buf,time_t& val)
  {
    uint32_t tmp;
    buf.Read((char*)&tmp,4);
    val=htonl(tmp);
    return buf;
  }

  inline SmsBuffer& operator<<(SmsBuffer& buf,uint16_t val)
  {
    uint16_t nval=ntohs((uint16_t)val);
    buf.Append((char*)&nval,2);
    return buf;
  }

  inline SmsBuffer& operator>>(SmsBuffer& buf,uint16_t& val)
  {
    buf.Read((char*)&val,2);
    val=htons(val);
    return buf;
  }


  inline SmsBuffer& operator<<(SmsBuffer& buf,const uint8_t& val)
  {
    buf.Append((char*)&val,1);
    return buf;
  }

  inline SmsBuffer& operator>>(SmsBuffer& buf,uint8_t& val)
  {
    buf.Read((char*)&val,1);
    return buf;
  }

  inline SmsBuffer& operator<<(SmsBuffer& buf,const Descriptor& val)
  {
    buf<<val.msc<<val.imsi<<val.sme;
    return buf;
  }

  inline SmsBuffer& operator>>(SmsBuffer& buf,Descriptor& val)
  {
    SmallCharBuf scbMsc(val.msc,sizeof(val.msc));
    SmallCharBuf scbImsi(val.imsi,sizeof(val.imsi));
    buf>>scbMsc>>scbImsi>>val.sme;
    val.mscLength=(uint8_t)strlen(val.msc);
    val.imsiLength=(uint8_t)strlen(val.imsi);
    return buf;
  }

}

inline void Serialize(const SMS& sms,BufOps::SmsBuffer& dst)
{
  using namespace BufOps;

  dst<<(uint8_t)sms.state<<sms.submitTime<<sms.validTime<<sms.lastTime
     <<sms.nextTime<<sms.attempts<<sms.lastResult
     <<sms.getOriginatingAddress()<<sms.getDestinationAddress()
     <<sms.getDealiasedDestinationAddress()<<sms.messageReference
     <<sms.eServiceType<<sms.deliveryReport<<sms.billingRecord
     <<sms.originatingDescriptor<<sms.destinationDescriptor
     <<sms.routeId<<sms.serviceId<<sms.priority
     <<sms.srcSmeId<<sms.dstSmeId
     <<sms.concatMsgRef<<sms.concatSeqNum;

  //added in version 0x010001;
  dst<<(uint8_t)sms.needArchivate;

  const PropertySet& prop=sms.messageBody.getProperties();

  size_t szPos=dst.GetPos();
  uint32_t dummy=0;
  dst<<dummy;

  for(size_t i=0;i<=SMS_LAST_TAG;i++)
  {
    if ( i == Body::unType(Tag::SMPP_SHORT_MESSAGE) ) continue;
    if ( i == Body::unType(Tag::SMPP_MESSAGE_PAYLOAD) ) continue;
    if(prop.properties[i].isSet())
    {
      switch(prop.properties[i].type)
      {
        case SMS_INT_TAG:
        {
          uint16_t tag=(uint16_t)i;
          dst<<tag;
          dst<<(uint32_t)prop.properties[i].iValue;
        }break;
        case SMS_STR_TAG:
        {
          uint16_t tag=(uint16_t)(i|(SMS_STR_TAG<<8));
          dst<<tag;
          uint32_t len=(uint32_t)(prop.properties[i].xValue.length()+1);
          dst<<len;
          dst.Append(prop.properties[i].xValue.c_str(),len);
        }break;
        case SMS_BIN_TAG:
        {
          uint16_t tag=(uint16_t)(i|(SMS_BIN_TAG<<8));
          dst<<tag;
          uint32_t len=(uint32_t)(prop.properties[i].xValue.length());
          dst<<len;
          dst.Append(prop.properties[i].xValue.data(),len);
        }break;
      }
    }
  }

  size_t savePos=dst.GetPos();
  uint32_t bodySize=(uint32_t)(savePos-szPos-4);
  dst.SetPos(szPos);
  dst<<bodySize;
  dst.SetPos(savePos);

 /* uint32_t bodyLength=sms.messageBody.getBufferLength();
  dst<<bodyLength;
  dst.Append((char*)sms.messageBody.getBuffer(),bodyLength);*/
}

inline void Deserialize(BufOps::SmsBuffer& src,SMS& sms,int ver)
{
  using namespace BufOps;

  uint8_t smsState;
  SmallCharBuf scbServiceType(sms.eServiceType,sizeof(sms.eServiceType));
  SmallCharBuf scbRouteId(sms.routeId,sizeof(sms.routeId));
  SmallCharBuf scbSrcSmeId(sms.srcSmeId,sizeof(sms.srcSmeId));
  SmallCharBuf scbDstSmeId(sms.dstSmeId,sizeof(sms.dstSmeId));

  src>>smsState>>sms.submitTime>>sms.validTime>>sms.lastTime
     >>sms.nextTime>>sms.attempts>>sms.lastResult
     >>sms.getOriginatingAddress()>>sms.getDestinationAddress()
     >>sms.getDealiasedDestinationAddress()>>sms.messageReference
     >>scbServiceType
     >>sms.deliveryReport>>sms.billingRecord
     >>sms.originatingDescriptor>>sms.destinationDescriptor
     >>scbRouteId
     >>sms.serviceId>>sms.priority
     >>scbSrcSmeId
     >>scbDstSmeId
     >>sms.concatMsgRef>>sms.concatSeqNum;

  if(ver>0x010000)
  {
    uint8_t na;
    src>>na;
    sms.needArchivate=na;
  }

  sms.state=(State)smsState;

  uint32_t bodyLength;//=sms.messageBody.getBufferLength();
  src>>bodyLength;//dummy

  PropertySet& prop=sms.getMessageBody().getProperties();

  sms.getMessageBody().Clear();

  size_t bodyStart=src.GetPos();

  while(src.GetPos()-bodyStart<bodyLength)
  {
    uint16_t tag;
    src>>tag;
    switch(tag>>8)
    {
      case SMS_INT_TAG:
      {
        uint32_t val;
        src>>val;
        prop.properties[Body::unType(tag)].setInt(val);
      }break;
      case SMS_STR_TAG:
      {
        uint32_t len;
        src>>len;
        prop.properties[Body::unType(tag)].setStr(src.GetCurPtr());
        src.SetPos(src.GetPos()+len);
      }break;
      case SMS_BIN_TAG:
      {
        uint32_t len;
        src>>len;
        prop.properties[Body::unType(tag)].setBin(src.GetCurPtr(),len);
        src.SetPos(src.GetPos()+len);
      }break;
    }
  }
  /*
  uint8_t* bodyBuffer = new uint8_t[bodyLength];
  src.Read((char*)bodyBuffer,bodyLength);
  sms.messageBody.setBuffer(bodyBuffer, bodyLength);*/
}

}//namespace sms
}//namespace smsc
#endif
