#ifndef __SMSC_SYSTEM_EXTRA_HPP__
#define __SMSC_SYSTEM_EXTRA_HPP__

#include <ctype.h>

#include "util/smstext.h"
#include "core/buffers/TmpBuf.hpp"
#include "sms/sms.h"

namespace smsc{
namespace extra{

namespace buf=smsc::core::buffers;
using namespace smsc::sms;

class ExtraInfo{
public:
  static void Init()
  {
    StaticHolder<0>::instance=new ExtraInfo;
  }
  static void Shutdown()
  {
    delete StaticHolder<0>::instance;
    StaticHolder<0>::instance=0;
  }

  static ExtraInfo& getInstance()
  {
    return *StaticHolder<0>::instance;
  }

  struct ServiceInfo{
    std::string prefix;
    uint32_t serviceBit;
    bool diverted;
    Address divertAddr;
  };

  void addPrefix(uint32_t bit,const char* prefix,const char* divertAddr)
  {
    std::string str=prefix;
    std::string::size_type pos=str.find(',');
    std::string::size_type lastPos=0;
    do{
      ServiceInfo info;
      info.serviceBit=bit;
      info.prefix=str.substr(lastPos,pos==std::string::npos?pos:pos-lastPos);
      if(info.prefix.length()==0)
      {
        __warning2__("empty prefix found:'%s' for extra bit %d, address %s",prefix,bit,divertAddr);
      }
      else
      {
        for(std::string::size_type i=0;i<info.prefix.length();i++)info.prefix[i]=tolower(info.prefix[i]);
        if(info.prefix.length()!=1 && info.prefix[info.prefix.length()-1]!=' ')info.prefix+=' ';
        info.diverted=divertAddr!=0;
        if(info.diverted)
        {
          info.divertAddr=divertAddr;
        }
        services.push_back(info);
        if(pos==std::string::npos)break;
      }
      lastPos=pos+1;
      pos=str.find(',',lastPos);
    }while(lastPos!=std::string::npos);
  }

  bool checkExtraService(SMS& sms,ServiceInfo& si)
  {
    std::list<ServiceInfo>::iterator it=services.begin();
    unsigned msglen=0;
    if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&msglen);
    }else
    {
      msglen=sms.getIntProperty(Tag::SMPP_SM_LENGTH);
    }
    buf::TmpBuf<char,1024> msgText(msglen+1);
    int len=smsc::util::getSmsText(&sms,msgText.get(),msglen+1);
    for(int i=0;i<len && msgText.get()[i]!=' ';i++)msgText.get()[i]=tolower(msgText.get()[i]);

    bool rv=false;
    for(;it!=services.end();it++)
    {
      if(memcmp(it->prefix.c_str(),msgText,it->prefix.length())==0)
      {
        si=*it;
        rv=true;
        break;
      }
    }
    if(!rv)return false;

    if(!si.diverted)
    {
      if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
      {
        memcpy(msgText.get(),sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&msglen),msglen);
      }else
      {
        memcpy(msgText.get(),sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&msglen),msglen);
      }

      unsigned char* ptr=(unsigned char*)msgText.get();
      int ptrlen=msglen;
      if(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
      {
        ptrlen-=*ptr+1;
        ptr+=*ptr+1;
      }
      if(sms.getIntProperty(Tag::SMPP_DATA_CODING)==8)
      {
        memmove(ptr,ptr+si.prefix.length()*2,ptrlen-si.prefix.length()*2);
        msglen-=si.prefix.length()*2;
      }else
      {
        memmove(ptr,ptr+si.prefix.length(),ptrlen-si.prefix.length());
        msglen-=si.prefix.length();
      }
      if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
      {
        sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,msgText.get(),msglen);
      }else
      {
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,msgText.get(),msglen);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,msglen);
      }
    }
    return true;
  }

protected:
  ExtraInfo()
  {
  }

  std::list<ServiceInfo> services;


  /*
  static int BitIndex(uint32_t bit)
  {
    int idx=0;
    while((bit&1)!=1){bit>>=1;idx++;}
    return idx;
  }
  */

  ExtraInfo(const ExtraInfo&);
  template <int N>
  struct StaticHolder{
    static ExtraInfo* instance;
  };
};

template <int N>
ExtraInfo* ExtraInfo::StaticHolder<N>::instance=0;

}//system
}//smsc

#endif
