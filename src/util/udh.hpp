#ifndef __SMSC_UTIL_UDH_HPP_
#define __SMSC_UTIL_UDH_HPP_

#include <inttypes.h>
#include <memory>
#include "sms/sms.h"
#include "util/debug.h"
#include "core/buffers/TmpBuf.hpp"

namespace smsc{
namespace util{

using namespace smsc::sms;

using smsc::core::buffers::TmpBuf;

inline bool findConcatInfo(unsigned char* body,uint16_t& mr,uint8_t& idx,uint8_t& num,bool& havemoreudh)
{
  unsigned int udhLen=*body;
  unsigned int i=1;
  bool haveconcat=false;
  havemoreudh=false;
  while(i<udhLen)
  {
    if(body[i]==0) // 8 bit concat info
    {
      mr=body[i+2];
      num=body[i+3];
      idx=body[i+4];
      if(idx==0 || idx>num)
      {
        __warning2__("concat info seqnum=%u!!!",num);
        break;
      }
      haveconcat=true;
    }else if(body[i]==8) //16 concat info
    {
      mr=(body[i+2]<<8)|body[i+3];
      num=body[i+4];
      idx=body[i+5];
      if(idx==0 || idx>num)
      {
        __warning2__("concat info seqnum=%u!!!",num);
        break;
      }
      haveconcat=true;
    }
    else //something else
    {
      havemoreudh=true;
    }
    i=i+2+body[i+1];
  }
  return haveconcat;
}

inline bool findConcatInfo(const SMS& sms,uint16_t& mr,uint8_t& idx,uint8_t& num,bool& havemoreudh)
{
  unsigned char* body;
  if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    body=(unsigned char*)sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,0);
  }else
  {
    body=(unsigned char*)sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,0);
  }
  return findConcatInfo(body,mr,idx,num,havemoreudh);
}


inline bool extractPortsFromUdh(SMS& sms)
{
  if(!(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40))return true;
  unsigned char* body;
  unsigned int len;
  if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    body=(unsigned char*)sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }else
  {
    body=(unsigned char*)sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  }
  bool allSarsAreHere=sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS) &&
                      sms.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM) &&
                      sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
  unsigned char* ptr=body+1;
  unsigned char* end=body+body[0]+1;
  if(body[0]<2)return false;
  bool needRemove=false;
  bool haveChanges=false;
  int udhLen=body[0];
  while(ptr<end)
  {
    needRemove=false;
    if(*ptr==4)
    {
      sms.setIntProperty(Tag::SMPP_DESTINATION_PORT,ptr[2]);
      sms.setIntProperty(Tag::SMPP_SOURCE_PORT,ptr[3]);
      needRemove=true;
    }else if (*ptr==5)
    {
      sms.setIntProperty(Tag::SMPP_DESTINATION_PORT,(((int)ptr[2])<<8)|ptr[3]);
      sms.setIntProperty(Tag::SMPP_SOURCE_PORT,(((int)ptr[4])<<8)|ptr[5]);
      needRemove=true;
    }else if(allSarsAreHere && (*ptr==0 || *ptr==8))//8 bit concat info
    {
      ////
      //
      // If both sar and udh-concat are present, remove udh.
      // It will be reinserted from sars later
      //
      needRemove=true;
    }
    if(needRemove)
    {
      /*
         +---+---+----+---
         |TAG|LEN|DATA|TAG ...
         +---+---+----+---
         ^            ^
         |            |
         PTR          SRC

      */
      unsigned char* src=ptr+ptr[1]+2;
      if(src>end)
      {
        return false;
      }
      memmove(ptr,src,body+len-src);
      end-=src-ptr;
      len-=src-ptr;
      udhLen-=src-ptr;
      haveChanges=true;
      continue;
    }
    ptr+=ptr[1]+2;
  }
  if(ptr!=end)return false;
  if(!haveChanges)return true;
  if(udhLen>0)
  {
    body[0]=udhLen;
  }else
  {
    // udh exhausted, remove it
    memmove(body,body+1,len-1);
    len--;
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x40));
  }
  if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,(char*)body,len);
  }else
  {
    sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)body,len);
    sms.setIntProperty(Tag::SMPP_SM_LENGTH,len);
  }
  return true;
}

inline bool convertSarToUdh(SMS& sms)
{
  if(!(
      sms.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM) &&
      sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS) &&
      sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)
      )
    )
  {
    return true;
  }
  unsigned char userdata[256];
  unsigned int len;
  unsigned int udhilen=0;
  const char* msg;
  if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    msg=sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
  }else
  {
    sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
  }
  unsigned int off=1;
  if(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
  {
    udhilen=1+*((unsigned char*)msg);
    off=udhilen;
    memcpy(userdata,msg,off);
  }
  if(sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM)<256)
  {
    if(off>255-5)
    {
      trace("udh length>255");
      return false;
    }
    __trace2__("SAR2UDH: 8 bit mr=%d",sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM));
    userdata[off++]=0;
    userdata[off++]=3;
    userdata[off++]=sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);
    userdata[off++]=sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
    userdata[off++]=sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
  }else
  {
    if(off>255-6)
    {
      trace("udh length>255");
      return false;
    }
    __trace2__("SAR2UDH: 16 bit mr=%d",sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM));
    userdata[off++]=8;
    userdata[off++]=4;
    userdata[off++]=(sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM)>>8)&0xff;
    userdata[off++]=sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM)&0xff;
    userdata[off++]=sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
    userdata[off++]=sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM);
  }

  userdata[0]=off-1;
  TmpBuf<char,256> buf(off+len-udhilen);
  memcpy(buf.get(),userdata,off);
  memcpy(buf.get()+off,msg+udhilen,len-udhilen);
  len-=udhilen;
  len+=off;
  sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x40);
  if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,buf.get(),len);
    sms.setIntProperty(Tag::SMPP_SM_LENGTH,0);
  }else
  {
    sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,buf.get(),len);
    sms.setIntProperty(Tag::SMPP_SM_LENGTH,len);
  }
  return true;
}

} //util
} //smsc

#endif
