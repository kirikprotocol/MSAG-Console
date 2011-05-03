#include "core/buffers/DiskXTree.hpp"
#include <sms/sms.h>
#include "util/crc32.h"
#include "store/FileStorage.h"
#include "core/buffers/File.hpp"
#include "logger/Logger.h"
#include "util/sleep.h"

using namespace smsc::core::buffers;
using namespace smsc::sms;
using namespace smsc::util;
using namespace smsc::store;

#include "SmsKeys.hpp"


typedef DiskXTree<OffsetLtt> OffsetXTree;


struct RenameGuard{
  std::string fn;
  std::string normfn;
  bool released;
  bool needUnlink;
  RenameGuard(const char* f):fn(f),released(false)
  {
    std::string::size_type pos=fn.rfind('.');
    normfn=fn.substr(0,pos);
    needUnlink=File::Exists(normfn.c_str());
  }
  ~RenameGuard()
  {
    if(released)
    {
      try{
        if(needUnlink)File::Unlink(normfn.c_str());
        File::Rename(fn.c_str(),normfn.c_str());
      }catch(std::exception& e)
      {
        smsc_log_error(smsc::logger::Logger::getInstance("sms.idx"),"Failed to unlink+rename index file:'%s'",e.what());
      }
    }else
    {
      try{
        File::Unlink(fn.c_str());
      }catch(std::exception& e)
      {
        smsc_log_error(smsc::logger::Logger::getInstance("sms.idx"),"Failed to unlink index file:'%s'",e.what());
      }
    }
  }
  void release()
  {
    released=true;
  }
};

static std::string dir;
static int hour;
char* MakeFileName(const char* name,char* buf)
{
  printf(".");fflush(stdout);
  sprintf(buf,"%s/%s_%02d.idx.new",dir.c_str(),name,hour);
  return buf;
}

size_t XTDataSize(const OffsetLtt& data)
{
  return 5+4;
}

void XTWriteData(smsc::core::buffers::File& file,const OffsetLtt& data)
{
  //file.WriteByte(data.off.high);
  //file.WriteNetInt32(data.off.low);
  uint8_t high=(uint8_t)((data.off>>32)&0xff);
  uint32_t low=(uint32_t)(data.off&0xffffffffu);
  file.WriteByte(high);
  file.WriteNetInt32(low);
  file.WriteNetInt32(data.ltt);
}

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  if(argc==1)
  {
    printf("Usage: %s path_to_arc_dir [max read/write speed in kb/sec]\n",argv[0]);
    return 0;
  }
  try{
  dir=argv[1];
  if(dir.length() && *dir.rbegin()!='/')
  {
    dir+='/';
  }
  int maxFlushSpeed=argc>2?1024*atoi(argv[2]):0;
  if(maxFlushSpeed)
  {
    printf("Max read/write speed=%dKb/sec\n",maxFlushSpeed/1024);
  }else
  {
    printf("Maximum read/write speed enabled\n");
  }
  for(hour=0;hour<24;hour++)
  {
    char fnbuf[64];
    sprintf(fnbuf,"%02d.arc",hour);
    if(!File::Exists((dir+fnbuf).c_str()))
    {
      continue;
    }
    printf("Reading file %s\n",fnbuf);
    PersistentStorage st(dir,fnbuf);
    SMS sms;
    SMSId id;
    File::offset_type off=0;
    OffsetXTree idIdx;
    OffsetXTree srcIdIdx;
    OffsetXTree dstIdIdx;
    OffsetXTree routeIdIdx;
    OffsetXTree srcAddrIdx;
    OffsetXTree dstAddrIdx;

    int cnt=0;

    File::offset_type pieceStartOffset=0;
    uint64_t readStartMs=getmillis();

    while(st.readRecord(id,sms,&off))
    {
      if(maxFlushSpeed && off-pieceStartOffset>maxFlushSpeed)
      {
        uint64_t rdtm=getmillis()-readStartMs;
        if(rdtm<1000)
        {
          millisleep((unsigned)(1000-rdtm));
        }
        pieceStartOffset=off;
        readStartMs=getmillis();
      }
      OffsetLtt val(off,sms.lastTime);
      char keybuf[128];
      sprintf(keybuf,"%lld",id);
      idIdx.Insert(keybuf,val,true);
      srcIdIdx.Insert(sms.getSourceSmeId(),val);
      dstIdIdx.Insert(sms.getDestinationSmeId(),val);
      routeIdIdx.Insert(sms.getRouteId(),val);
      srcAddrIdx.Insert(sms.getOriginatingAddress().toString().c_str(),val);
      dstAddrIdx.Insert(sms.getDealiasedDestinationAddress().toString().c_str(),val);

      off=0;
      cnt++;
      if((cnt%1000)==0)
      {
        printf("Reading record %d\r",cnt);
        fflush(stdout);
      }
    }
    printf("\nWriting indexes:");

    char buf[1024];
    File f;
    f.SetBuffer(4*1024*1024);
    if(maxFlushSpeed)f.setMaxFlushSpeed(maxFlushSpeed);
    f.WOpen(MakeFileName("smsid",buf));
    RenameGuard smsidguard(buf);
    idIdx.WriteToFile(f);
    f.WOpen(MakeFileName("srcid",buf));
    RenameGuard srcidguard(buf);
    srcIdIdx.WriteToFile(f);
    f.WOpen(MakeFileName("dstid",buf));
    dstIdIdx.WriteToFile(f);
    RenameGuard dstidguard(buf);
    f.WOpen(MakeFileName("routeid",buf));
    RenameGuard routeidguard(buf);
    routeIdIdx.WriteToFile(f);
    f.WOpen(MakeFileName("srcaddr",buf));
    RenameGuard srcaddrguard(buf);
    srcAddrIdx.WriteToFile(f);
    f.WOpen(MakeFileName("dstaddr",buf));
    RenameGuard dstaddrguard(buf);
    dstAddrIdx.WriteToFile(f);
    printf("\n");

    smsidguard.release();
    srcidguard.release();
    dstidguard.release();
    routeidguard.release();
    srcaddrguard.release();
    dstaddrguard.release();
  }
  }catch(std::exception& e)
  {
    printf("Exception:'%s'\n",e.what());
  }
}

