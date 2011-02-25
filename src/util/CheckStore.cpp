#include "sms/sms_serializer.h"
#include "core/buffers/File.hpp"
#include "core/buffers/XHash.hpp"
#include <vector>

using namespace smsc::sms;

struct LoadUpInfo;
typedef std::vector<LoadUpInfo*> LoadUpVector;

struct LoadUpInfo{
  LoadUpInfo():sms(0),final(false){}
  SMSId id;
  uint32_t seq;
  bool final;
  SMS* sms;
};

typedef XHash<SMSId,LoadUpInfo> LoadUpHash;



const char storeSig[10]="SMSCSTORE";
const uint32_t storeVer=0x10001;

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  if(argc==1)
  {
    printf("Usage: %s [-n] [-v] infile [infiles ...]\n\t-n do not load sms\n\t-v verbose\n",argv[0]);
    return -1;
  }
  bool loadSms=true;
  bool verbose=false;
  for(int i=1;i<argc;i++)
  {
    if(strcmp(argv[i],"-n")==0)
    {
      loadSms=false;
      continue;
    }
    if(strcmp(argv[i],"-v")==0)
    {
      printf("verbose!\n");
      verbose=true;
      continue;
    }
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
    printf("Reading %s\n",argv[i]);
    //printf("sig=%s\n",sig);
    int ver=f.ReadNetInt32();
    //printf("ver=0x%08x\n",ver);
    LoadUpInfo item;
    File::offset_type size=f.Size();
    File::offset_type off;
    long bytesSkipped=0;
    int lastPrc=0;
    while((off=f.Pos())<size)
    {
      int prc=(int)(10*off/size);
      if(prc!=lastPrc)
      {
        printf("Progress:%d0%%\n",prc);
        lastPrc=prc;
      }
      if(size-off<4)
      {
        printf("Extra data at the end of file.\n");
        break;
      }
      uint32_t sz=f.ReadNetInt32();
      if(sz<8+4+1 || off+sz>size)
      {
        f.Seek(off+1);
        //printf("!");fflush(stdout);
        bytesSkipped++;
        continue;
      }
      item.id=f.ReadNetInt64();
      item.seq=f.ReadNetInt32();
      item.final=f.ReadByte();
      //printf("id=%lld,seq=%d,final=%d\n",item.id,item.seq,(int)item.final);

      //printf("sz1=%d,id=%lld, seq=%d, fin=%s\n",sz,id,seq,fin?"true":"false");fflush(stdout);
      buf.setSize(sz-8-4-1);
      f.Read(buf.get(),sz-8-4-1);
      buf.SetPos(0);
      int sz2=f.ReadNetInt32();
      if(sz!=sz2)
      {
        f.Seek(off+1);
        //printf("!");fflush(stdout);
        bytesSkipped++;
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
        if(itemPtr)
        {
          *itemPtr=item;
        }else
        {
          luHash.Insert(item.id,item);
          itemPtr=luHash.GetPtr(item.id);
          luVector.push_back(itemPtr);
        }
        if(loadSms)
        {
          if(!itemPtr->sms)
          {
            itemPtr->sms=new SMS;
          }
          Deserialize(buf,*itemPtr->sms,ver);
        }
      }catch(std::exception& e)
      {
        printf("exception: %s\n",e.what());
      }
    }
    if(bytesSkipped)
    {
      printf("bytes skipped:%ld\n",bytesSkipped);
    }
    time_t now=time(NULL);
    int cnt=0;
    for(LoadUpVector::iterator it=luVector.begin();it!=luVector.end();it++)
    {
      if((*it)->final)continue;
      cnt++;
      if(!loadSms)
      {
        continue;
      }
      SMS& sms=*(*it)->sms;
      if(!sms.Invalidate(__FILE__,__LINE__))
      {
        printf("Validation failed for msgId=%lld",(*it)->id);
      }
      if(sms.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
      {
        sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,0);
      }else
      {
        sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,0);
      }
      if(sms.validTime<now)
      {
        tm* t=localtime(&sms.validTime);
        if(verbose)
        {
          printf("Expired sms: msgId=%lld, oa=%s, da=%s, validTime=%02d.%02d.%04d %02d:%02d\n",(*it)->id,sms.getOriginatingAddress().toString().c_str(),sms.getDealiasedDestinationAddress().toString().c_str(),
              t->tm_mday,t->tm_mon+1,t->tm_year+1900,t->tm_hour,t->tm_min);
        }
      }
    }
    printf("sms found:%d\n",cnt);
  }
  return 0;
}
