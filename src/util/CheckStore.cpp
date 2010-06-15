#include "sms/sms_serializer.h"
#include "core/buffers/File.hpp"
#include "core/buffers/XHash.hpp"
#include <vector>

using namespace smsc::sms;

struct LoadUpInfo;
typedef std::vector<LoadUpInfo*> LoadUpVector;

struct LoadUpInfo{
  SMSId id;
  uint32_t seq;
  bool final;
  SMS sms;
};

typedef XHash<SMSId,LoadUpInfo> LoadUpHash;



const char storeSig[10]="SMSCSTORE";
const uint32_t storeVer=0x10001;

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  if(argc==1)
  {
    printf("Usage: %s infile [infiles ...]\n",argv[0]);
    return -1;
  }
  for(int i=1;i<argc;i++)
  {

    //SMS sms;
    BufOps::SmsBuffer buf(0);
    File f;

    LoadUpHash luHash;
    LoadUpVector luVector;

    try{
      f.ROpen(argv[i]);
    }catch(std::exception& e)
    {
      printf("Failed to open %s:'%s'\n",argv[i],e.what());
      continue;
    }
    char sig[32]={0,};
    f.Read(sig,9);
    if(strcmp(sig,storeSig)!=0)
    {
      printf("Invalid signature in file %s\n",argv[i]);
      continue;
    }
    //printf("sig=%s\n",sig);
    int ver=f.ReadNetInt32();
    //printf("ver=0x%08x\n",ver);
    LoadUpInfo item;
    while(f.Pos()<f.Size())
    {
      File::offset_type off=f.Pos();
      uint32_t sz=f.ReadNetInt32();
      if(sz<8+4+1 || off+sz>f.Size())
      {
        f.Seek(off+1);
        printf("!");fflush(stdout);
        continue;
      }
      item.id=f.ReadNetInt64();
      item.seq=f.ReadNetInt32();
      item.final=f.ReadByte();

      //printf("sz1=%d,id=%lld, seq=%d, fin=%s\n",sz,id,seq,fin?"true":"false");fflush(stdout);
      buf.setSize(sz-8-4-1);
      f.Read(buf.get(),sz-8-4-1);
      buf.SetPos(0);
      int sz2=f.ReadNetInt32();
      if(sz!=sz2)
      {
        f.Seek(off+1);
        printf("!");fflush(stdout);
        continue;
      }
      if(sz>70000)
      {
        printf("Skipped record at %s:%llx, msgId=%lld too large(%d)\n",argv[i],off,item.id,sz);
        continue;
      }

      LoadUpInfo* itemPtr=luHash.GetPtr(item.id);
      if(itemPtr)
      {
        if(itemPtr->final || itemPtr->seq>item.seq)
        {
          continue;
        }
      }

      try{
        Deserialize(buf,item.sms,ver);
        if(itemPtr)
        {
          *itemPtr=item;
        }else
        {
          luHash.Insert(item.id,item);
          itemPtr=luHash.GetPtr(item.id);
          luVector.push_back(itemPtr);
        }
      }catch(std::exception& e)
      {
        printf("exception: %s\n",e.what());
      }
    }
    time_t now=time(NULL);
    int cnt=0;
    for(LoadUpVector::iterator it=luVector.begin();it!=luVector.end();it++)
    {
      if((*it)->final)continue;
      SMS& sms=(*it)->sms;
      if(!sms.Invalidate(__FILE__,__LINE__))
      {
        printf("Validation failed for msgId=%lld",(*it)->id);
      }
      cnt++;
      if(sms.validTime<now)
      {
        tm* t=localtime(&sms.validTime);
        printf("Expired sms: msgId=%lld, oa=%s, da=%s, validTime=%02d.%02d.%04d %02d:%02d\n",(*it)->id,sms.getOriginatingAddress().toString().c_str(),sms.getDealiasedDestinationAddress().toString().c_str(),
               t->tm_mday,t->tm_mon+1,t->tm_year+1900,t->tm_hour,t->tm_min);
      }
    }
    printf("sms found:%d\n",cnt);
  }
  return 0;
}
