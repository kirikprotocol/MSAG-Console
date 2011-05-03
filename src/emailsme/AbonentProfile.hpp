#ifndef __SMSC_EMAILSME_ABONENTPROFILE_HPP__
#define __SMSC_EMAILSME_ABONENTPROFILE_HPP__

#include "util/int.h"
#include <string>
#include "sms/sms.h"
#include "sms/sms_util.h"
#include "core/buffers/File.hpp"
#include "core/buffers/TmpBuf.hpp"

namespace smsc{
namespace emailsme{

using namespace smsc::core::buffers;

enum LimitType{ltDay,ltWeek,ltMonth};

template <class T>
inline void WriteString(T& f,const std::string& str)
{
  f.WriteNetInt16((uint16_t)str.length());
  f.Write(str.c_str(),str.length());
}

template <class T>
inline void ReadString(T& f,std::string& str)
{
  uint16_t len=f.ReadNetInt16();
  TmpBuf<char,64> buf(len);
  f.Read(buf.get(),len);
  str.assign(buf.get(),len);
}

struct AbonentProfile{
  smsc::sms::Address addr;
  std::string user;
  std::string forwardEmail;
  std::string realName;
  LimitType ltype;
  time_t  limitDate;
  bool numberMap;
  int limitValue;
  int limitCountGsm2Eml;
  int limitCountEml2Gsm;

  File::offset_type offset;



  template <class T>
  void Write(T& f)const
  {
    WriteAddress(f,addr);
    WriteString(f,user);
    WriteString(f,forwardEmail);
    WriteString(f,realName);
    f.WriteByte(ltype);
    f.WriteByte(numberMap?1:0);
    f.WriteNetInt32((uint32_t)limitDate);
    f.WriteNetInt32(limitValue);
    f.WriteNetInt32(limitCountGsm2Eml);
    f.WriteNetInt32(limitCountEml2Gsm);
  }
  template <class T>
  void Read(T& f)
  {
    ReadAddress(f,addr);
    ReadString(f,user);
    ReadString(f,forwardEmail);
    ReadString(f,realName);
    ltype=(LimitType)f.ReadByte();
    numberMap=f.ReadByte()?true:false;
    limitDate=f.ReadNetInt32();
    limitValue=f.ReadNetInt32();
    limitCountGsm2Eml=f.ReadNetInt32();
    limitCountEml2Gsm=f.ReadNetInt32();
  }
};

}//emailsme
}//smsc

#endif
