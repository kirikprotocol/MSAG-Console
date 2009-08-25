#include <stdio.h>
#include "sms/sms.h"
#include "sms/sms_util.h"
#include "core/buffers/File.hpp"
#include "core/buffers/FixedLengthString.hpp"
#include <algorithm>

static char profileMagic[]="SmScPrOf";
static const char sig[]="SMSCPROF";
static const uint32_t oldVer=0x00010001;
static const uint32_t newVer=0x00010100;


using smsc::core::buffers::File;
using namespace smsc::sms;

namespace ProfileReportOptions{
  const int ReportNone  =0;
  const int ReportFull  =1;
  const int ReportFinal =3;
}


struct ProfileOld{
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
#endif

  File::offset_type offset;

  ProfileOld()
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

  ProfileOld(const ProfileOld& src)
  {
    *this=src;
  }

  ProfileOld& operator=(const ProfileOld& src)
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
#endif
    }
    return *this;
  }

  bool operator==(const ProfileOld& src)const
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
#endif
           ;
  }

  void assign(const ProfileOld& src)
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
#endif

  }
  static uint32_t Size()
  {
    return 4+4+4+32+1+32+1+1+1+1+1+1+1+1+4+4+4
#ifdef SMSEXTRA
    +4+1
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

  Profile& operator=(const ProfileOld& src)
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
      closedGroupId=src.closedGroupId;
      accessMaskIn=src.accessMaskIn;
      accessMaskOut=src.accessMaskOut;
      offset=src.offset;
#ifdef SMSEXTRA
      subscription=src.subscription;
      sponsored=src.sponsored;
#endif
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



int main(int argc,char* argv[])
{
  if(argc!=3)
  {
    printf("Profiler storage file convertation utility\n");
    printf("Usage: %s infile outfile\n",argv[0]);
    return 1;
  }
  std::string inFileName=argv[1];
  std::string outFileName=argv[2];
  if(!File::Exists(inFileName.c_str()))
  {
    printf("Input file '%s' doesn't exists\n",inFileName.c_str());
    return 1;
  }
  File inFile,outFile;
  try{
    inFile.ROpen(inFileName.c_str());

    if(!File::Exists(outFileName.c_str()))
    {
      outFile.RWCreate(outFileName.c_str());
      outFile.Write(sig,8);
      outFile.WriteNetInt32(newVer);
      outFile.Flush();
    }else
    {
      printf("Output file '%s' already exists\n",outFileName.c_str());
      return 1;
    }
    char oldSig[9]={0,};
    inFile.Read(oldSig,8);
    if(strcmp(oldSig,sig))
    {
      printf("Invalid signature of input file\n");
      return 1;
    }
    uint32_t ver=inFile.ReadNetInt32();
    if(ver!=oldVer)
    {
      printf("Unexpected version of input file:%x, expected %x\n",ver,oldVer);
      return 1;
    }

    File::offset_type sz=inFile.Size();
    File::offset_type pos=inFile.Pos();
    ProfileOld p;
    Profile p2;
    Address addr;
    char magic[9]={0,};

    int count=0;

    while(pos<sz)
    {
      bool used=inFile.ReadByte();
      if(used)
      {
        inFile.Read(magic,8);
        if(memcmp(profileMagic,magic,8))
        {
          __warning2__("Broken profiles storage file at offset %lld",pos);
          break;
        }
        ReadAddress(inFile,addr);
        p.Read(inFile);
        p2=p;
        outFile.WriteByte(1);
        outFile.Write(profileMagic,8);
        WriteAddress(outFile,addr);
        p2.Write(outFile);
        count++;
      }else
      {
        inFile.SeekCur(8+AddressSize()+ProfileOld::Size());
      }
      pos+=1+8+AddressSize()+ProfileOld::Size();
      if(sz-pos<1+8+AddressSize()+ProfileOld::Size())
      {
        //holes.push_back(pos);
        break;
      }
    }
    printf("Processed %d records\n",count);
  }catch(std::exception& e)
  {
    printf("Exception: %s\n",e.what());
  }


  return 0;
}
