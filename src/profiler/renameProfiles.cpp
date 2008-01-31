#define NOLOGGERPLEASE
#include "core/buffers/File.hpp"
#include <map>
#include <string>
#include "sms/sms.h"
#include <ctype.h>
#include "profiler/profiler-types.hpp"
#include "sms/sms_util.h"

int main(int argc,char* argv[])
{
  if(argc<3)
  {
    printf("Usage: %0 {profilestorefile} {addressesmappingfile} [{addressesmappingfile}...]\n",argv[0]);
    return 1;
  }
  const char sig[]="SMSCPROF";
#ifdef SMSEXTRA
  const uint32_t ver=0x00010100;
#else
  const uint32_t ver=0x00010001;
#endif
  smsc::core::buffers::File store;
  smsc::core::buffers::File input;
  using namespace smsc::sms;
  using namespace smsc::profiler;

  typedef std::map<std::string,std::string> StrMap;
  StrMap remap;
  for(int i=2;i<argc;i++)
  {
    try{
      input.ROpen(argv[i]);
      std::string line;
      char oldaddr[64],newaddr[64];
      while(input.ReadLine(line))
      {
        if(sscanf(line.c_str(),"%20s %20s",oldaddr,newaddr)!=2)
        {
          printf("Unrecognized input line:%s\n",line.c_str());
          continue;
        }
        if(!isdigit(oldaddr[0]))
        {
          continue;
        }
        StrMap::iterator it=remap.find(oldaddr);
        if(it!=remap.end() && it->second!=newaddr)
        {
          printf("Duplicate conflicting entry:%s->%s\n",oldaddr,newaddr);
          continue;
        }
        remap.insert(StrMap::value_type(oldaddr,newaddr));
      }
    }catch(std::exception& e)
    {
      printf("Exception:%s\n",e.what());
    }
  }
  printf("Loaded %d addresses\n",remap.size());
  try{
    store.RWOpen(argv[1]);
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
    int cnt=0;
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
      if(addr.getLength()>1)
      {
        StrMap::iterator it=remap.find(addr.value+1);
        if(it!=remap.end())
        {
          if(it->second.length()<sizeof(addr.value))
          {
            strcpy(addr.value+1,it->second.c_str());
            store.Seek(store.Pos()-AddressSize());
            WriteAddress(store,addr);
            cnt++;
          }
        }
      }
      pos+=Profile::Size();
      store.SeekCur(Profile::Size());
    }
    printf("Renamed %d profiles\n",cnt);
  }catch(std::exception& e)
  {
    printf("Exception:%s\n",e.what());
  }
}


