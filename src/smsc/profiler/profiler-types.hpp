#ifndef __SMSC_PROFILER_PROFILER_TYPES_HPP__
#define __SMSC_PROFILER_PROFILER_TYPES_HPP__

#include <algorithm>
#include <string>

#include "smsc/smpp/smpp.h"
#include "sms/sms.h"
#include "core/buffers/File.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace smsc{
namespace profiler{

using namespace smsc::sms;
using smsc::core::buffers::File;

namespace ProfileReportOptions{
  const int ReportNone  =0;
  const int ReportFull  =1;
  const int ReportFinal =3;
}

namespace ProfileCharsetOptions{
  const int Default    =smsc::smpp::DataCoding::SMSC7BIT;
  const int Latin1     =smsc::smpp::DataCoding::LATIN1;
  const int Ucs2       =smsc::smpp::DataCoding::UCS2;
  const int Ucs2AndLat =smsc::smpp::DataCoding::UCS2 | smsc::smpp::DataCoding::LATIN1;
  const int UssdIn7Bit =0x80;
}

/*
  if((profile_dcs&UssdIn7Bit)==UssdIn7Bit)
  {
    if(sms.ussd_service_op)
    {
      profile_dcs=Default;
    }else
    {
      profile_dcs=profile_dcs&(~UssdIn7Bit);
    }
  }
*/

namespace ProfilerMatchType{
  static const int mtDefault=0;
  static const int mtMask   =1;
  static const int mtExact  =2;
}

namespace HideOption{
  static const int hoDisabled=0;
  static const int hoEnabled =1;
  static const int hoSubstitute=2;
}

inline const char* HideOptionToText(int ho)
{
  switch(ho)
  {
    case HideOption::hoDisabled:return "N";
    case HideOption::hoEnabled:return "Y";
    case HideOption::hoSubstitute:return "S";
  }
  return "U";
}

struct Profile{
  std::string divert;
  smsc::core::buffers::FixedLengthString<6> locale;
  uint8_t codepage;
  uint8_t reportoptions;
  uint8_t hide;
  bool hideModifiable:1;
  bool divertActive:1;
  bool divertActiveAbsent:1;
  bool divertActiveBlocked:1;
  bool divertActiveBarred:1;
  bool divertActiveCapacity:1;
  bool divertModifiable:1;
  bool udhconcat:1;
  bool translit:1;
  uint32_t closedGroupId;
  uint32_t accessMaskIn,accessMaskOut;
#ifdef SMSEXTRA
  uint32_t subscription;
  uint8_t sponsored;
  smsc::core::buffers::FixedLengthString<11> nick;
#endif

  File::offset_type offset;

  Profile()
  {
    codepage=0;
    reportoptions=ProfileReportOptions::ReportFull;
    locale="";
    hide=0;
    hideModifiable=false;

    divert="";
    divertActive=false;
    divertModifiable=false;
    divertActiveAbsent=false;
    divertActiveBlocked=false;
    divertActiveBarred=false;
    divertActiveCapacity=false;
    udhconcat=true;
    translit=true;

    closedGroupId=0;
    accessMaskIn=1;
    accessMaskOut=1;
#ifdef SMSEXTRA
    subscription=0;
    sponsored=0;
#endif


    offset=-1;
  }

  Profile(const Profile& src)
  {
    *this=src;
  }

  Profile& operator=(const Profile& src)
  {
    if ( this != &src ) {
      codepage=src.codepage;
      reportoptions=src.reportoptions;
      locale=src.locale;
      hide=src.hide;
      hideModifiable=src.hideModifiable;

      divert=src.divert;
      divertModifiable=src.divertModifiable;
      divertActive=src.divertActive;
      divertActiveAbsent=src.divertActiveAbsent;
      divertActiveBlocked=src.divertActiveBlocked;
      divertActiveBarred=src.divertActiveBarred;
      divertActiveCapacity=src.divertActiveCapacity;

      udhconcat=src.udhconcat;
      translit=src.translit;
      closedGroupId=src.closedGroupId;
      accessMaskIn=src.accessMaskIn;
      accessMaskOut=src.accessMaskOut;
      offset=src.offset;
#ifdef SMSEXTRA
      subscription=src.subscription;
      sponsored=src.sponsored;
      nick=src.nick;
#endif
    }
    return *this;
  }

  bool operator==(const Profile& src)const
  {
    return codepage==src.codepage &&
           reportoptions==src.reportoptions &&
           locale==src.locale &&
           hide==src.hide &&
           hideModifiable==src.hideModifiable &&
           divert==src.divert &&
           divertActive==src.divertActive &&
           divertModifiable==src.divertModifiable &&
           divertActiveAbsent==src.divertActiveAbsent &&
           divertActiveBlocked==src.divertActiveBlocked &&
           divertActiveBarred==src.divertActiveBarred &&
           divertActiveCapacity==src.divertActiveCapacity &&
           udhconcat==src.udhconcat &&
           translit==src.translit &&
           closedGroupId==src.closedGroupId &&
           accessMaskIn==src.accessMaskIn &&
           accessMaskOut==src.accessMaskOut
#ifdef SMSEXTRA
           && subscription==src.subscription
           && sponsored==src.sponsored
           && nick==src.nick
#endif
           ;
  }

  void assign(const Profile& src)
  {
    File::offset_type saveoff=offset;
    *this=src;
    offset=saveoff;
  }

  void Write(File& f)const
  {


    f.WriteNetInt32(codepage);
    f.WriteNetInt32(reportoptions);
    f.WriteNetInt32(hide);

    char buf[32]={0,};
    memcpy(buf,locale.c_str(),std::min(sizeof(buf),locale.length()));
    f.Write(buf,sizeof(buf));

    f.WriteByte(hideModifiable);

    memset(buf,0,sizeof(buf));
    memcpy(buf,divert.c_str(),std::min(sizeof(buf),divert.length()));
    f.Write(buf,sizeof(buf));

    f.WriteByte(divertActive);
    f.WriteByte(divertActiveAbsent);
    f.WriteByte(divertActiveBlocked);
    f.WriteByte(divertActiveBarred);
    f.WriteByte(divertActiveCapacity);
    f.WriteByte(divertModifiable);
    f.WriteByte(udhconcat);
    f.WriteByte(translit);
    f.WriteNetInt32(closedGroupId);
    f.WriteNetInt32(accessMaskIn);
    f.WriteNetInt32(accessMaskOut);
#ifdef SMSEXTRA
    f.WriteNetInt32(subscription);
    f.WriteByte(sponsored);
    memset(buf,0,sizeof(buf));
    memcpy(buf,nick.c_str(),std::min(sizeof(buf),nick.length()));
    f.Write(buf,10);
#endif
  }
  void Read(File& f)
  {

    offset=f.Pos();
    codepage=f.ReadNetInt32();
    reportoptions=f.ReadNetInt32();
    hide=f.ReadNetInt32();
    char buf[32]={0,};
    f.Read(buf,sizeof(buf));
    locale=(const char*)buf;

    hideModifiable=f.ReadByte();

    memset(buf,0,sizeof(buf));
    f.Read(buf,sizeof(buf));
    divert=buf;


    divertActive=f.ReadByte();
    divertActiveAbsent=f.ReadByte();
    divertActiveBlocked=f.ReadByte();
    divertActiveBarred=f.ReadByte();
    divertActiveCapacity=f.ReadByte();
    divertModifiable=f.ReadByte();
    udhconcat=f.ReadByte();
    translit=f.ReadByte();
    closedGroupId=f.ReadNetInt32();
    accessMaskIn=f.ReadNetInt32();
    accessMaskOut=f.ReadNetInt32();
#ifdef SMSEXTRA
    subscription=f.ReadNetInt32();
    sponsored=f.ReadByte();
    memset(buf,0,sizeof(buf));
    f.Read(buf,10);
    nick=(const char*)buf;
#endif

  }
  static uint32_t Size()
  {
    return 4+4+4+32+1+32+1+1+1+1+1+1+1+1+4+4+4
#ifdef SMSEXTRA
    +4+1+10
#endif
    ;
  }

  string toString()const
  {
    string rv;
    char buf[64];
    sprintf(buf,"r=%d;",reportoptions);
    rv+=buf;
    sprintf(buf,"dc=%d;",codepage);
    rv+=buf;
    sprintf(buf,"l=%s;",locale.c_str());
    rv+=buf;
    sprintf(buf,"h=%d;",hide);
    rv+=buf;
    sprintf(buf,"hm=%c;",hideModifiable?'Y':'N');
    rv+=buf;
    sprintf(buf,"d=%s;",divert.length()==0?"(NULL)":divert.c_str());
    rv+=buf;
    sprintf(buf,"da=%c;",divertActive?'Y':'N');
    rv+=buf;
    sprintf(buf,"dm=%c;",divertModifiable?'Y':'N');
    rv+=buf;
    sprintf(buf,"cgId=%u;",closedGroupId);
    rv+=buf;
    sprintf(buf,"amI=%08x;",accessMaskIn);
    rv+=buf;
    sprintf(buf,"amO=%08x;",accessMaskOut);
    rv+=buf;
    return rv;
  }

};


const int pusUpdated=1;
const int pusInserted=2;
const int pusUnchanged=3;
const int pusError=4;

class ProfilerInterface{
public:
  virtual Profile& lookup(const Address& address)=0;
  virtual Profile& lookupEx(const Address& address,int& matchType,std::string& matchAddr)=0;
  virtual int update(const Address& address,const Profile& profile)=0;
  virtual int updatemask(const Address& address,const Profile& profile)=0;
  virtual void add(const Address& address,const Profile& profile)=0;
  virtual void remove(const Address& address)=0;

};//ProfilerInterface

}//profiler
}//smsc

#endif
