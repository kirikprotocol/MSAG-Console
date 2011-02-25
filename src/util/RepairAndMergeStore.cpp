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
  LoadUpInfo():smsBuf(0),smsBufSize(0){}
  SMSId id;
  uint32_t seq;
  bool final;
  char* smsBuf;
  int smsBufSize;
  int smsSize;
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
  //SMS sms;
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

  int ver;

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
    ver=f.ReadNetInt32();
    //printf("ver=0x%08x\n",ver);
    LoadUpInfo item;
    File::offset_type off,size=f.Size();
    long bytesSkipped=0;

    printf("Reading %s\n",argv[i]);

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
        printf("Extra bytes at the end of file %s\n",argv[i]);
        break;
      }
      uint32_t sz=f.ReadNetInt32();
      if(sz<8+4+1 || off+sz>size)
      {
        f.Seek(off+1);
        bytesSkipped++;
        continue;
      }
      item.id=f.ReadNetInt64();
      item.seq=f.ReadNetInt32();
      item.final=f.ReadByte();

      //printf("sz1=%d,id=%lld, seq=%d, fin=%s\n",sz,id,seq,fin?"true":"false");fflush(stdout);
      int smsSize=sz-8-4-1;
      buf.setSize(smsSize);
      f.Read(buf.get(),smsSize);
      buf.SetPos(0);
      int sz2=f.ReadNetInt32();
      if(sz!=sz2)
      {
        f.Seek(off+1);
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
//        Deserialize(buf,item.sms,ver);
//        if(!item.sms.Invalidate(__FILE__,__LINE__))
//        {
//          continue;
//        }
        if(itemPtr)
        {
          *itemPtr=item;
        }else
        {
          luHash.Insert(item.id,item);
          itemPtr=luHash.GetPtr(item.id);
          luVector.push_back(itemPtr);
        }
        if(itemPtr->final)
        {
          if(itemPtr->smsBuf)
          {
            delete [] itemPtr->smsBuf;
          }
          itemPtr->smsBuf=0;
          itemPtr->smsBufSize=0;
          itemPtr->smsSize=0;
        }else
        {
          if(!itemPtr->smsBuf)
          {
            itemPtr->smsBuf=new char[smsSize];
            itemPtr->smsBufSize=smsSize;
            itemPtr->smsSize=smsSize;
          }else if(itemPtr->smsBufSize<smsSize)
          {
            delete [] itemPtr->smsBuf;
            itemPtr->smsBuf=new char[smsSize];
            itemPtr->smsBufSize=smsSize;
          }
          memcpy(itemPtr->smsBuf,buf.get(),smsSize);
          itemPtr->smsSize=smsSize;
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
    if(bytesSkipped)
    {
      printf("bytes skipped:%ld\n",bytesSkipped);
    }
  }
  int count=0;
  printf("Writing\n");
  int prc,prcLast=0;
  for(LoadUpVector::iterator it=luVector.begin();it!=luVector.end();it++)
  {
    prc=(int)(10l*(it-luVector.begin())/luVector.size());
    if(prc!=prcLast)
    {
      printf("Progress:%d0%%\n",prc);
      prcLast=prc;
    }
    if((*it)->final)continue;
    {
      buf.setExtBuf((*it)->smsBuf,(*it)->smsSize);
      SMS sms;
      Deserialize(buf,sms,ver);
      if(!sms.Invalidate(__FILE__,__LINE__))
      {
        printf("Invalid sms id=%lld skipped\n",(*it)->id);
        continue;
      }
    }
    count++;
    using namespace smsc::sms::BufOps;
    BufOps::SmsBuffer smsbuf(0);
    uint32_t sz=8+4+1+(*it)->smsSize;
    smsbuf<<sz<<(*it)->id<<(*it)->seq<<(uint8_t)(*it)->final;
    //printf("%d\n",smsbuf.GetPos());
    //Serialize((*it)->sms,smsbuf);
    smsbuf.Append((*it)->smsBuf,(*it)->smsBufSize);
    //sz=smsbuf.GetPos()-sizeof(sz);
    //smsbuf.SetPos(0);
    smsbuf<<sz;
    //smsbuf.SetPos(sz+sizeof(sz));
    //smsbuf<<sz;
    g.Write(smsbuf.get(),smsbuf.GetPos());
  }
  printf("%d sms recovered\n",count);
  return 0;
}
