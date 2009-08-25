#define NOLOGGERPLEASE
#include "core/buffers/File.hpp"
#include <map>
#include <string>
#include "sms/sms.h"
#include <ctype.h>
#include "profiler/profiler-types.hpp"
#include "sms/sms_util.h"

std::string dc2strrv;
using namespace smsc::sms;
using namespace smsc::profiler;

const char* dc2str(uint8_t dc)
{
  if(dc&ProfileCharsetOptions::UssdIn7Bit)
  {
    dc2strrv="ussd7bit&";
    dc&=~ProfileCharsetOptions::UssdIn7Bit;
  }else
  {
    dc2strrv="";
  }
  switch(dc)
  {
    case ProfileCharsetOptions::Default:dc2strrv+="default";break;
    case ProfileCharsetOptions::Latin1:dc2strrv+="latin1";break;
    case ProfileCharsetOptions::Ucs2:dc2strrv+="ucs2";break;
    case ProfileCharsetOptions::Ucs2AndLat:dc2strrv+="ucs2&latin1";break;
    default:dc2strrv+="unknown";break;
  }
  return dc2strrv.c_str();
}

const char* rep2str(uint8_t ro)
{
  switch(ro)
  {
    case ProfileReportOptions::ReportNone:return "none";
    case ProfileReportOptions::ReportFull:return "full";
    case ProfileReportOptions::ReportFinal:return "final";
    default:return "unknown";
  }
}

int main(int argc,char* argv[])
{
  if(argc<2)
  {
    printf("Usage: %s {profilestorefile}\n",argv[0]);
    return 1;
  }
  const char sig[]="SMSCPROF";
#ifdef SMSEXTRA
  const uint32_t ver=0x00010100;
#else
  const uint32_t ver=0x00010001;
#endif
  smsc::core::buffers::File store;

  try{
    store.ROpen(argv[1]);
    char filesig[9]={
      0,
    };
    uint32_t filever;
    store.Read(filesig,8);
    if(strcmp(filesig,sig))
    {
      printf("Invalid profiler store file signature\n");
      return 2;
    }
    filever=store.ReadNetInt32();
    if(filever!=ver)
    {
      printf("Invalid profiler store file version\n");
      return 2;
    }
    smsc::core::buffers::File::offset_type pos=store.Pos();
    smsc::core::buffers::File::offset_type size=store.Size();
    while(pos<size)
    {
      uint8_t used=store.ReadByte();
      pos++;
      if(!used)
      {
        store.SeekCur(8+AddressSize()+Profile::Size());
        pos+=8+AddressSize()+Profile::Size();
        continue;
      }
      store.SeekCur(8);
      pos+=8;
      Address addr;
      ReadAddress(store,addr);
      pos+=AddressSize();
      Profile p;
      p.Read(store);
      printf("%s:dc=%s,ro=%s,tr=%s,udh=%s\n",addr.toString().c_str(),dc2str(p.codepage),rep2str(p.reportoptions),p.udhconcat?"Y":"N",p.translit?"Y":"N");
      pos+=Profile::Size();
    }
  }catch(std::exception& e)
  {
    printf("Exception:%s\n",e.what());
  }
}


