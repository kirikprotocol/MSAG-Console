#define NOLOGGERPLEASE
#include "sms/sms_serializer.h"
#include "core/buffers/File.hpp"
#include "core/buffers/XHash.hpp"
#include <vector>

using namespace smsc::sms;
using namespace smsc::core::buffers;

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
  if(argc<3)
  {
    printf("Usage: %s outfile infile [infiles ...]\n",argv[0]);
    return -1;
  }
  if(File::Exists(argv[1]))
  {
    printf("Output file already exists\n");
    return -1;
  }
  SMS sms;
  BufOps::SmsBuffer buf(0);
  File f,g;
  try{
    g.WOpen(argv[1]);
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
    return -1;
  }

  LoadUpHash luHash;
  LoadUpVector luVector;

  g.Write(storeSig,9);
  g.WriteNetInt32(storeVer);

  for(int i=2;i<argc;i++)
  {
    try{
      f.ROpen(argv[i]);
    }catch(std::exception& e)
    {
      printf("exception:%s\n",e.what());
      return -1;
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
        if(!item.sms.Invalidate(__FILE__,__LINE__))
        {
          continue;
        }
        if(itemPtr)
        {
          *itemPtr=item;
        }else
        {
          luHash.Insert(item.id,item);
          itemPtr=luHash.GetPtr(item.id);
          luVector.push_back(itemPtr);
        }
        /*
        printf("from %s to %s, srcSmeId=%s\n",
          sms.getOriginatingAddress().toString().c_str(),
          sms.getDestinationAddress().toString().c_str(),
          sms.getSourceSmeId()
        );fflush(stdout);
        */
      }catch(std::exception& e)
      {
        printf("exception: %s\n",e.what());
      }
      //printf("submit time=%d\n",sms.submitTime);
      //printf("sz2=%d\n",sz,sz2);fflush(stdout);
    }
  }
  int count=0;
  for(LoadUpVector::iterator it=luVector.begin();it!=luVector.end();it++)
  {
    if((*it)->final)continue;
    count++;
    using namespace smsc::sms::BufOps;
    BufOps::SmsBuffer smsbuf(0);
    uint32_t sz=0;
    smsbuf<<sz<<(*it)->id<<(*it)->seq<<(uint8_t)(*it)->final;
    //printf("%d\n",smsbuf.GetPos());
    Serialize((*it)->sms,smsbuf);
    sz=smsbuf.GetPos()-sizeof(sz);
    smsbuf.SetPos(0);
    smsbuf<<sz;
    smsbuf.SetPos(sz+sizeof(sz));
    smsbuf<<sz;
    g.Write(smsbuf.get(),smsbuf.GetPos());
  }
  printf("%d sms recovered\n",count);
  return 0;
}
