#ifndef __SMSC_PROFILER_PROFILER_TYPES_HPP__
#define __SMSC_PROFILER_PROFILER_TYPES_HPP__

#include "sms/sms.h"
#include "smpp/smpp.h"

namespace smsc{
namespace profiler{

using namespace smsc::sms;

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
  int codepage;
  int reportoptions;
  int hide;
  std::string locale;
  bool hideModifiable;

  std::string divert;
  bool divertActive;
  bool divertActiveAbsent;
  bool divertActiveBlocked;
  bool divertActiveBarred;
  bool divertActiveCapacity;
  bool divertModifiable;

  bool udhconcat;
  bool translit;

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
  }

  Profile(const Profile& src)
  {
    *this=src;
  }

  Profile& operator=(const Profile& src)
  {
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
    return *this;
  }

  bool operator==(const Profile& src)
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
           translit==src.translit
           ;
  }

  void assign(const Profile& src)
  {
    *this=src;
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
