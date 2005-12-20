#include <stdio.h>
#include "profiler/profiler-types.hpp"
#include "core/buffers/File.hpp"
#include "sms/sms_util.h"

using namespace smsc::core::buffers;
using namespace smsc::profiler;
using namespace smsc::sms;

static char profileMagic[]="SmScPrOf";


int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  try{
    if(argc!=3)
    {
      printf("Usage: %s {inputfilename} {outputfilename}\n",argv[0]);
      return 0;
    }
    std::string storeFileName=argv[1];
    std::string repFileName=argv[2];
    const char* filename=argv[1];
    const char sig[]="SMSCPROF";
    const uint32_t ver=0x00010000;
    //storeFileName=filename;
    hrtime_t st=gethrtime();

    File storeFile;

    File repFile;

    if(!File::Exists(storeFileName.c_str()))
    {
      printf("Input store file not found:%s\n",storeFileName.c_str());
      return 0;
    }
    if(File::Exists(repFileName.c_str()))
    {
      printf("Output file already exists:%s\n",repFileName.c_str());
      return 0;
    }

    repFile.RWCreate(repFileName.c_str());
    repFile.Write(sig,8);
    repFile.WriteNetInt32(ver);
    repFile.Flush();


    storeFile.ROpen(filename);

    char fileSig[9]={0,};
    storeFile.Read(fileSig,8);
    if(strcmp(fileSig,sig))
    {
      __warning2__("Invalid or corrupted store file:%s",filename);
      throw Exception("Invalid or corrupted store file:%s",filename);
    }

    uint32_t fileVer;
    fileVer=storeFile.ReadNetInt32();

    if(ver!=fileVer)
    {
      __warning2__("Incompatible version of profiler store file(%x!=%x):%s",fileVer,ver,filename);
      throw Exception("Incompatible version of profiler store file(%x!=%x):%s",fileVer,ver,filename);
    }

    File::offset_type sz=storeFile.Size();
    File::offset_type pos=storeFile.Pos();
    Profile p;
    Address addr;
    __trace2__("pos=%lld, sz=%lld",pos,sz);

    char magic[9]={0,};

    while(pos<sz)
    {
      uint8_t used=storeFile.ReadByte();
      storeFile.Read(magic,8);
      if(memcmp(profileMagic,magic,8))
      {
        pos+=1;
        storeFile.Seek(pos);
        printf("skip:%lld\n",pos);
        continue;
      }
      ReadAddress(storeFile,addr);
      p.Read(storeFile);
      printf("write from %lld\n",pos);
      if(used)
      {
        repFile.WriteByte(1);
        repFile.Write(profileMagic,8);
        WriteAddress(repFile,addr);
        p.Write(repFile);
        repFile.Flush();
      }else
      {
        printf("skip unused at %lld\n",pos);
      }
      pos+=1+8+AddressSize()+Profile::Size();
    }
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
  return 0;
}
