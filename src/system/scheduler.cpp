#include "system/smsc.hpp"
#include "system/scheduler.hpp"
#include "util/Exception.hpp"
#include <string>
#include <vector>
#include "core/buffers/XHash.hpp"
#include "cluster/Interconnect.h"
#include "core/threads/Thread.hpp"
#include "util/vformat.hpp"
#include "util/sleep.h"

namespace smsc{
namespace system{

smsc::logger::Logger* Scheduler::log;

/*
static EventMonitor* schedulerMon=0;
static Mutex* storeMtx=0;
static bool* delayInitPtr=0;
*/

using namespace std;

struct LoadUpInfo;
typedef vector<LoadUpInfo*> LoadUpVector;

struct LoadUpInfo{
  SMSId id;
  uint32_t seq;
  bool final;
  char* smsBuf;
  int smsBufSize;
  void assignBuf(const char* argBuf,int argBufSize)
  {
    smsBuf=new char[argBufSize];
    smsBufSize=argBufSize;
    memcpy(smsBuf,argBuf,smsBufSize);
  }
};

typedef XHash<SMSId,LoadUpInfo> LoadUpHash;

const char LocalFileStore::storeSig[10]="SMSCSTORE";
const uint32_t LocalFileStore::storeVer=0x10001;


void LocalFileStore::Init(smsc::util::config::Manager* cfgman,Smsc* smsc)
{
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("store.init");
  using namespace std;
  bool haveSz=false;
  try{
    const char* szstr=cfgman->getString("MessageStore.LocalStore.maxStoreSize");
    haveSz=true;
    int sz;
    char c;
    if(sscanf(szstr,"%d%c",&sz,&c)!=2)
    {
      haveSz=false;
      smsc_log_warn(log,"Invalid maxStoreSize:%s",szstr);
      throw Exception("invalid maxStoreSize");
    }
    maxStoreSize=sz;
    switch(c)
    {
      case 'k':
      case 'K':maxStoreSize*=1024;break;
      case 'm':
      case 'M':maxStoreSize*=1024*1024;break;
      case 'g':
      case 'G':maxStoreSize*=1024*1024*1024;break;
      default:
      {
        smsc_log_warn(log,"Invalid maxStoreSize:%s",szstr);
        throw Exception("invalid maxStoreSize");
      }break;
    }
    smsc_log_warn(log,"maxStoreSize=%lld",maxStoreSize);
  }catch(...)
  {
  }
  if(!haveSz)
  {
    maxStoreSize=cfgman->getInt("MessageStore.LocalStore.maxStoreSize");
  }
  minRollTime=cfgman->getInt("MessageStore.LocalStore.minRollTime");

  string mainFileName=cfgman->getString("MessageStore.LocalStore.filename");
  string rolFileName=mainFileName;
  string::size_type pos=rolFileName.rfind('.');
  if(pos!=string::npos)
  {
    rolFileName.erase(pos);
  }
  rolFileName+=".rol";

  LoadUpHash luHash;
  LoadUpVector luVector;
  vector<string> toDelete;
  /*
  if(File::Exists((mainFileName+".bak").c_str()) ||
     File::Exists((rolFileName+".bak").c_str()))
  {
    __warning2__
    (
      "store %s file found! Previous startup attempt failed! Rename bak files and try again",
      File::Exists((mainFileName+".bak").c_str())?
         (mainFileName+".bak").c_str():
         (rolFileName+".bak").c_str()
    );
    //abort();
    smsc->stop();
    return;
  }
  */

  bool delayInit=sched.delayInit;

  if(delayInit)
  {
    sched.mon.Unlock();
  }
  uint32_t fileVer;
  try{
    if(File::Exists(mainFileName.c_str()))
    {
      vector<File*> files;
      int mainIdx=0;
      File *f;
      for(;;)
      {
        std::string backFile=mainFileName+format(".%03d",mainIdx);

        if(!File::Exists(backFile.c_str()))
        {
          break;
        }
        f=new File();
        f->ROpen(backFile.c_str());
        files.push_back(f);
        mainIdx++;
      }

      f=new File();
      f->ROpen(mainFileName.c_str());
      f->Rename((mainFileName+format(".%03d",mainIdx)).c_str());
      files.push_back(f);

      int rolIdx=0;
      for(;;)
      {
        std::string backFile=rolFileName+format(".%03d",rolIdx);
        if(!File::Exists(backFile.c_str()))
        {
          break;
        }
        f=new File();
        f->ROpen(backFile.c_str());
        files.push_back(f);
        rolIdx++;
      }

      if(File::Exists(rolFileName.c_str()))
      {
        f=new File();
        f->ROpen(rolFileName.c_str());
        f->Rename((rolFileName+format(".%03d",rolIdx)).c_str());
        files.push_back(f);
      }

      InitPrimaryFile(mainFileName);
      if(sched.delayInit)sched.delayInit=false;

      for(vector<File*>::iterator it=files.begin();it!=files.end();it++)
      {
        File* pf=*it;
        char sigBuf[sizeof(storeSig)];
        LoadUpInfo item;
        item.smsBuf=0;
        BufOps::SmsBuffer smsBuf(0);
        uint32_t sz,sz2;
        LoadUpInfo* itemPtr;
        File::offset_type fSize=pf->Size();
        File::offset_type fPos=0;
        try{
          fPos+=pf->Read(sigBuf,sizeof(storeSig)-1);
          sigBuf[sizeof(storeSig)-1]=0;
          if(strcmp(sigBuf,storeSig))
          {
            throw Exception("Storage file signature mismatch");
          }
          fileVer=pf->ReadNetInt32();
          fPos+=4;
          if(fileVer>storeVer)
          {
            smsc_log_warn(log,"File version doesn't match current version:%d<%d",storeVer,fileVer);
            abort();
          }
          while(fPos<fSize)
          {
            sz=pf->ReadNetInt32();
            fPos+=4;
            if(fPos+sz>fSize)
            {
              smsc_log_warn(log,"Incomplete record detected, fPos=%lld, fSize=%lld, recSize=%d",fPos,fSize,sz);
              break;
            }
            if(sz<=8+4+1)
            {
              smsc_log_warn(log,"Store broken at fPos=%lld, fSize=%lld, recSize=%d",fPos,fSize,sz);
              abort();
            }
            item.id=pf->ReadNetInt64();
            fPos+=8;
            item.seq=pf->ReadNetInt32();
            fPos+=4;
            item.final=pf->ReadByte();
            fPos+=1;
            int bufSz=sz-8-4-1;
            smsBuf.setSize(bufSz);
            pf->Read(smsBuf.get(),bufSz);
            fPos+=bufSz;
            sz2=pf->ReadNetInt32();
            fPos+=4;

            smsc_log_debug(log,"StoreInit: msgId=%lld, seq=%d, final=%s",item.id,item.seq,item.final?"true":"false");
            if(sz!=sz2)
            {
              throw Exception("Corrupted store file %s, record size mismatch:%u!=%u",pf->getFileName().c_str(),sz,sz2);
            }
            itemPtr=luHash.GetPtr(item.id);
            if(itemPtr!=0)
            {
              if(itemPtr->final || itemPtr->seq>item.seq)
              {
                continue;
              }
              if(item.final)
              {
                if(itemPtr->smsBuf)
                {
                  delete [] itemPtr->smsBuf;
                  itemPtr->smsBuf=0;
                }
                itemPtr->final=true;
                continue;
              }
            }

            if(!item.final)
            {
              item.assignBuf(smsBuf.get(),bufSz);
              smsc_log_debug(log,"allocated smsbuf=%p(%d) for id=%lld",item.smsBuf,item.smsBufSize,item.id);
            }else
            {
              item.smsBuf=0;
            }

            if(itemPtr)
            {
              if(itemPtr->smsBuf)
              {
                smsc_log_debug(log,"delete (before assign) smsbuf=%p for id=%lld",itemPtr->smsBuf,itemPtr->id);
                delete [] itemPtr->smsBuf;
              }
              *itemPtr=item;
            }else
            {
              luHash.Insert(item.id,item);
              itemPtr=luHash.GetPtr(item.id);
              smsc_log_debug(log,"insert smsBuf=%p(%d) %lld",item.smsBuf,item.smsBufSize,item.id);
              luVector.push_back(itemPtr);
            }
          };
          smsc_log_debug(log,"getfn=%s",pf->getFileName().c_str());
          toDelete.push_back(pf->getFileName());
        }catch(exception& e)
        {
          smsc_log_warn(log,"Operative storage read failed %s:%s",pf->getFileName().c_str(),e.what());
        }
        pf->Close();
        delete pf;
      }
    }else
    {
      InitPrimaryFile(mainFileName);
    }
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"Exception during storage init %s",e.what());
  }
  if(delayInit)
  {
    sched.mon.Lock();
  }

  smsc_log_debug(log,"Local store loaded. %d messages found.",luVector.size());

  int cnt=0;
  for(LoadUpVector::iterator it=luVector.begin();it!=luVector.end();it++)
  {
    LoadUpInfo& item=**it;
    if(item.final)continue;
    if(!item.smsBuf)
    {
      smsc_log_error(log,"Loading error!!! Sms point==NULL!!! msgId=%lld",(*it)->id);
      continue;
    }
    {
      MutexGuard mg(sched.storeMtx);
      Save(item.id,item.seq,item.smsBuf,item.smsBufSize);
    }
    SMS sms;
    smsc_log_debug(log,"init smsbuf from %p(%d) for id=%lld",item.smsBuf,item.smsBufSize,item.id);
    BufOps::SmsBuffer buf(item.smsBuf,item.smsBufSize);
    Deserialize(buf,sms,fileVer);

    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x02)//forward mode sms!
    {
      if(item.smsBuf)
      {
        delete [] item.smsBuf;
        item.smsBuf=0;
      }
      continue;
    }

    try{
      int smeIndex=smsc->getSmeIndex(sms.getDestinationSmeId());
      if(delayInit)
      {
        sched.mon.Unlock();
      }
      try{
        sched.AddScheduledSms((*it)->id,sms,smeIndex);
      }catch(std::exception& e)
      {
        smsc_log_warn(log,"Exception in AddScheduledSms:'%s'",e.what());
      }

      cnt++;
      if((cnt%200)==0)
      {
        //smsc::core::threads::Thread::Yield();
        millisleep(10);
      }

      Scheduler::StoreData* sd=new Scheduler::StoreData(item.smsBuf,item.smsBufSize,item.seq);
      smsc_log_debug(log,"init sd from %p(%d) for id=%lld",item.smsBuf,item.smsBufSize,item.id);
      item.smsBuf=0;
      item.smsBufSize=0;
      if(delayInit)
      {
        sched.mon.Lock();
      }

      {
        MutexGuard mg(sched.storeMtx);
        sched.store.Insert(item.id,sd);
        sd->rit=sched.replMap.insert(Scheduler::ReplaceIfPresentMap::value_type(sms,item.id));
        sd->it=sched.currentSnap.insert(sched.currentSnap.begin(),IdSeqPair(item.id,item.seq));
      }
    }catch(...)
    {
      smsc_log_warn(log,"systemId=%s not found. sms %lld dropped",sms.getSourceSmeId(),(*it)->id);
    }
    if(item.smsBuf)
    {
      delete [] item.smsBuf;
      item.smsBuf=0;
    }
  }

  for(vector<string>::iterator it=toDelete.begin();it!=toDelete.end();it++)
  {
    File::Unlink(it->c_str());
  }

  loadup=false;
  running=true;
  lastRollTime=time(NULL);

  Start();
}

bool LocalFileStore::StartRoll(const IdSeqPairList& argSnap)
{
  MutexGuard mg1(mtx);
  MutexGuard mg2(mon);
  if(rolling)return false;
  info1(Scheduler::log,"Preparing rolling");
  rolling=true;
  snap=argSnap;
  string pfn=primaryFile.getFileName();
  string rfn=pfn;
  string::size_type pos=rfn.rfind('.');
  if(pos!=string::npos)
  {
    rfn.erase(pos);
  }
  rfn+=".rol";
  primaryFile.Rename(rfn.c_str());
  rolFile=rfn.c_str();
  primaryFile.Close();
  InitPrimaryFile(pfn);
  lastRollTime=time(NULL);
  mon.notify();
  info1(Scheduler::log,"Rolling prepared");
  return true;
}


bool LocalFileStore::Save(smsc::sms::SMSId id,uint32_t seq,const char* smsBufPtr,int smsBufSize,bool final)
{
  using namespace smsc::sms;
  using namespace smsc::sms::BufOps;
  SmsBuffer smsbuf(4+8+4+1+smsBufSize+4);
  uint32_t sz=8+4+1+smsBufSize;
  uint8_t fin=final?1:0;

  smsbuf<<sz<<id<<seq<<fin;
  smsbuf.Append(smsBufPtr,smsBufSize);

  //Serialize(sms,smsbuf);
  //sz=(uint32_t)(smsbuf.GetPos()-sizeof(sz));
  //smsbuf.SetPos(0);
  //smsbuf<<sz;
  //smsbuf.SetPos(sz+sizeof(sz));
  smsbuf<<sz;
  MutexGuard mg(mtx);
  hrtime_t writeStart=gethrtime();
  primaryFile.Write(smsbuf.get(),smsbuf.GetPos());
  hrtime_t writeEnd=gethrtime();
  int writeTime=(int)((writeEnd-writeStart)/1000000);
  if(writeTime>50)
  {
    warn2(Scheduler::log,"disk write takes %d msec!",writeTime);
  }
  fileSize+=smsbuf.GetPos();
  return !loadup && fileSize>maxStoreSize && time(NULL)-lastRollTime>minRollTime;
}


int LocalFileStore::Execute()
{
  MutexGuard mg(mon);
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("store.roll");
  while(running)
  {
    mon.wait();
    if(running && rolling)
    {
      info1(log,"Rolling strated");
      mon.Unlock();
      bool ok=true;
      try{
        int i=0;
        uint64_t blockStart=getmillis();
        //Target: 2Mb/sec
        //Estimation: 512b/sms
        //4000 sms/sec =~ 2Mb/sec
        //interval 10ms -> 40/10ms

        for(IdSeqPairList::iterator it=snap.begin();it!=snap.end() && running;it++)
        {
          smsc_log_debug(log,"roll:id=%lld, seq=%d",it->first,it->second);
          sched.StoreSms(it->first,it->second);
          i++;
          if((i%40)==0)
          {
            uint64_t curTime=getmillis();
            int opTime=(int)(curTime-blockStart);
            int sleepTime=10-opTime;
            if(sleepTime>0)
            {
              millisleep(sleepTime);
            }
            blockStart=getmillis();
          }
        }
      }catch(exception& e)
      {
        warn2(log,"Exception during rolling:%s\n",e.what());
        ok=false;
      }
      mon.Lock();
      //delete snapPtr;
      //snapPtr=0;
      snap.swap(IdSeqPairList());
      rolling=false;
      if(ok)
      {
        info1(log,"Rolling finished ok");
        File::Unlink(rolFile.c_str());
      }else
      {
        File::Rename(rolFile.c_str(),(rolFile+".bad").c_str());
        warn1(log,"Rolling finished with error");
      }
    }
  }
  return 0;
}

void Scheduler::Init(Smsc* psmsc,smsc::util::config::Manager* cfgman)
{
  smsc=psmsc;
  localFileStore.Init(cfgman,psmsc);
  //billingStorage.init(*cfgman);
  archiveStorage.init(cfgman->getString("archiveDir"),cfgman->getInt("archiveInterval"));
  
}


void Scheduler::DelayInit(Smsc* psmsc,smsc::util::config::Manager* cfgman)
{
  archiveStorage.init(cfgman->getString("archiveDir"),cfgman->getInt("archiveInterval"));
  smsc=psmsc;
  delayInit=true;
}


int Scheduler::Execute()
{
  smsc::logger::Logger* smsLog=smsc::logger::Logger::getInstance("sms.trace");
  if(delayInit)
  {
    info1(log,"Start delayedInit");
    try{
      MutexGuard guard(mon);
      //schedulerMon=&mon;
      //delayInitPtr=&delayInit;
      Init(smsc,&smsc::util::config::Manager::getInstance());
    }catch(std::exception& e)
    {
      warn2(log,"Exception during delayed Scheduler init:%s",e.what());
    }
    delayInit=false;
  }

  time_t t=time(NULL);

  //time_t lastBillRoll=t;
  time_t lastArcRoll=t;

  while(!isStopping)
  {
    t=time(NULL);
    MutexGuard guard(mon);
    while(outQueue.Count())
    {
      SmscCommand cmd;
      outQueue.Shift(cmd);
      if(cmd->cmdid==SMEALERT)
      {
        try{
          int idx=cmd->get_smeIndex();
          info2(log,"SMEALERT for smeIdx=%d,smeId=%s",idx,Smsc::getInstance().getSmeInfo(idx).systemId.c_str());
          int cnt=0;
          time_t sctime=time(NULL);
          SmeStatMap::iterator it=smeStatMap.find(idx);
          if(it==smeStatMap.end())continue;
          SmeStat::ChainSet::iterator cit=it->second.chainSet.begin(),end=it->second.chainSet.end();
          for(;cit!=end;cit++)
          {
            Chain* c=*cit;
            RescheduleChain(c,sctime);
            cnt++;
            if(cnt==500)
            {
              sctime++;
              cnt=0;
            }
          }
          info2(log,"rescheduled %d sms for smeId=%s",(int)it->second.chainSet.size(),Smsc::getInstance().getSmeInfo(idx).systemId.c_str());

        }catch(std::exception& e)
        {
          warn2(log,"Exception during SMEALERT:%s",e.what());
        }
      }else if(cmd->cmdid==HLRALERT)
      {
        info2(log,"HLRALERT: %s",cmd->get_address().toString().c_str());
        {
          mon.Unlock();
          dpfTracker.hlrAlert(cmd->get_address());
          mon.Lock();
        }
        try{
          Chain* c=GetChain(cmd->get_address());
          if(!c)continue;
          if(c->inProcMap)
          {
            time_t now=time(NULL);
            if(now-c->inProcMap>60*60)
            {
              smsc_log_error(log,"CHAIN INPROCMAP STALL: %s",c->addr.toString().c_str());
              for(ProcessingMap::iterator it=procMap.begin();it!=procMap.end();it++)
              {
                if(it->second==c)
                {
                  procMap.erase(it);
                  break;
                }
              }
              c->inProcMap=0;
            }else
            {
              continue;
            }
          }
          RescheduleChain(c,time(NULL));
        }catch(std::exception& e)
        {
          warn2(log,"Exception during HLRALERT:%s",e.what());
        }
      }
    }

    try{
      /*
      if(t-lastBillRoll>billingStorage.getStorageInterval())
      {
        billingStorage.roll();
        lastBillRoll=t;
      }
      */
      if(t-lastArcRoll>archiveStorage.getStorageInterval())
      {
        archiveStorage.roll();
        lastArcRoll=t;
      }
    }catch(std::exception& e)
    {
      warn2(log,"Exception during archive/billing roll:%s",e.what());
    }

    if(timeLine.size()>0 && timeLine.headTime()<t)
    {
      if(prxmon)prxmon->Signal();
      mon.wait(100);
    }else
    {
      mon.wait(1000);
    }
  }
  warn1(log,"Scheduler exited");
  //billingStorage.roll();
  archiveStorage.roll();
  //billingStorage.close();
  archiveStorage.close();
  return 0;
}


SMSId Scheduler::getNextId()
{
  MutexGuard mg(idMtx);
  SMSId rv=idSeq++;
  lastIdSeqFlush++;
  if(lastIdSeqFlush>=MessageIdSequenceExtent)
  {
    idFile.Seek(0);
    idFile.WriteNetInt64(idSeq);
    lastIdSeqFlush=0;
  }
  return rv;
}

SMSId Scheduler::createSms(SMS& sms, SMSId id,const smsc::store::CreateMode flag )
        throw(StorageException, DuplicateMessageException)
{
  BufOps::SmsBuffer buf;

  if(flag!=smsc::store::CREATE_NEW_NO_CLEAR)
  {
    sms.lastTime = 0;
    sms.lastResult = 0;
    sms.attempts = 0;
  }

  Serialize(sms,buf);

  SDLockGuard lg(storeMtx);
  StoreData* sd;
  {
    MutexGuard mg(storeMtx);

    if(flag==smsc::store::SMPP_OVERWRITE_IF_PRESENT)
    {
      ReplaceIfPresentMap::iterator it=replMap.find(sms);
      if(it!=replMap.end())
      {
        StoreData *ptr=store.Get(it->second);
        ptr->it->second=++(ptr->seq);
        LocalFileStoreSave(it->second,ptr,true);
        currentSnap.erase(ptr->it);
        replMap.erase(it);
        store.Delete(it->second);
        delStoreData(ptr);
      }
    }
    sd=newStoreData(buf);
    store.Insert(id,sd);
    lg.Lock(sd);
    sd->rit=replMap.insert(ReplaceIfPresentMap::value_type(sms,id));
    sd->it=currentSnap.insert(currentSnap.end(),LocalFileStore::IdSeqPair(id,0));
  }

  LocalFileStoreSave(id,sd);
  return id;
}

void Scheduler::retriveSms(SMSId id, SMS &sms)
        throw(StorageException, NoSuchMessageException)
{
  BufOps::SmsBuffer buf;
  {
    MutexGuard mg(storeMtx);
    StoreData** ptr=store.GetPtr(id);
    if(!ptr)throw NoSuchMessageException(id);
    buf.setSize((*ptr)->smsBufSize);
    buf.Append((*ptr)->smsBuf,(*ptr)->smsBufSize);
  }
  buf.SetPos(0);
  Deserialize(buf,sms,LocalFileStore::storeVer);
  //(*ptr)->LoadSms(sms);
}

void Scheduler::changeSmsStateToEnroute(SMSId id,
    const Descriptor& dst, uint32_t failureCause,
    time_t nextTryTime, uint32_t attempts)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToEnroute: msgId=%lld",id);
  SDLockGuard lg(storeMtx);
  StoreData* sd;
  {
    MutexGuard mg(storeMtx);
    StoreData** ptr=store.GetPtr(id);
    if(!ptr)throw NoSuchMessageException(id);
    sd=*ptr;
    lg.Lock(sd);
  }
  //SMS& sms=(*ptr)->sms;
  SMS sms;
  sd->LoadSms(sms);
  sms.state = ENROUTE;
  sms.destinationDescriptor=dst;
  sms.lastTime=time(NULL);
  sms.setLastResult(failureCause);
  sms.setNextTime(nextTryTime);
  sms.setAttemptsCount(attempts);
  sd->it->second=++sd->seq;
  sd->SaveSms(sms);
  LocalFileStoreSave(id,sd);
}

void Scheduler::changeSmsStateToDelivered(SMSId id,
    const Descriptor& dst)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToDelivered: msgId=%lld",id);
  doFinalizeSms(id,smsc::sms::DELIVERED,smsc::system::Status::OK,dst);
}

void Scheduler::changeSmsStateToUndeliverable(SMSId id,
    const Descriptor& dst, uint32_t failureCause)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToUndeliverable: msgId=%lld",id);
  doFinalizeSms(id,smsc::sms::UNDELIVERABLE,failureCause);
}

void Scheduler::changeSmsStateToExpired(SMSId id)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToExpired: msgId=%lld",id);
  doFinalizeSms(id,smsc::sms::EXPIRED,smsc::system::Status::EXPIRED);
}

void Scheduler::changeSmsStateToDeleted(SMSId id)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToDeleted: msgId=%lld",id);
  doFinalizeSms(id,smsc::sms::DELETED,smsc::system::Status::DELETED);
}

void Scheduler::changeSmsConcatSequenceNumber(SMSId id, int8_t inc)
        throw(StorageException, NoSuchMessageException)
{
  SDLockGuard lg(storeMtx);
  StoreData* sd;
  {
    MutexGuard mg(storeMtx);
    StoreData** ptr=store.GetPtr(id);
    if(!ptr)throw NoSuchMessageException(id);
    sd=*ptr;
    lg.Lock(sd);
  }
  SMS sms;
  sd->LoadSms(sms);
  sms.setConcatSeqNum(sms.getConcatSeqNum()+inc);
  sd->it->second=++sd->seq;
  sd->SaveSms(sms);
  LocalFileStoreSave(id,sd);
}

void Scheduler::doFinalizeSms(SMSId id,smsc::sms::State state,int lastResult,const Descriptor& dstDsc)
{
  StoreData* sd;
  {
    MutexGuard mg(storeMtx);
    StoreData** ptr=store.GetPtr(id);
    if(!ptr)throw NoSuchMessageException(id);
    currentSnap.erase((*ptr)->it);
    sd=*ptr;
    store.Delete(id);
    replMap.erase(sd->rit);
  }
  SMS sms;
  sd->LoadSms(sms);
  sms.state=state;
  sms.lastResult=lastResult;
  sms.lastTime = time(NULL);
  sms.nextTime = 0;
  sms.destinationDescriptor=dstDsc;
  sd->SaveSms(sms);
  sd->seq++;
  LocalFileStoreSave(id,sd,true);
  if (sms.needArchivate)
  {
    try{
      archiveStorage.createRecord(id, sms);
    }catch(std::exception& e)
    {
      warn2(log,"archiveStorage.createRecord failed:%s",e.what());
    }
  }
  //if (sd->sms.billingRecord) billingStorage.createRecord(id, sd->sms);
  {
    MutexGuard mg(storeMtx);
    delStoreData(sd);
  }
}


void Scheduler::replaceSms(SMSId id, SMS& sms)
                throw(StorageException, NoSuchMessageException)
{
  SDLockGuard lg(storeMtx);
  StoreData* sd;
  {
    MutexGuard mg(storeMtx);
    StoreData** ptr=store.GetPtr(id);
    if(!ptr)throw NoSuchMessageException(id);
    sd=*ptr;
    lg.Lock(sd);
  }
  sd->SaveSms(sms);
  sd->it->second=++sd->seq;
  LocalFileStoreSave(id,sd);
}

/*
void Scheduler::sendAlertNotification(SMSId id,int status)
{
  SMS sms;
  try{
    retriveSms(id,sms);
    SmeProxy* proxy=smsc->getSmeProxy(sms.srcSmeId);
    debug2(log,"Sending AlertNotification to '%s'",sms.srcSmeId);
    if(proxy!=0)
    {
      proxy->putCommand(
        SmscCommand::makeAlertNotificationCommand
        (
          proxy->getNextSequenceNumber(),
          sms.getDestinationAddress(),
          sms.getOriginatingAddress(),
          status
        )
      );
    }else
    {
      warn2(log,"Sme %s requested dpf, but not connected at the moment",sms.srcSmeId);
    }
  }catch(std::exception& e)
  {
    warn2(log,"Failed to send AlertNotification to sme %s:'%s'",sms.srcSmeId,e.what());
  }
}
*/

}//system
}//smsc
