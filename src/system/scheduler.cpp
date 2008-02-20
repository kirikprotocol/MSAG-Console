#include "system/smsc.hpp"
#include "system/scheduler.hpp"
#include "util/Exception.hpp"
#include <string>
#include <vector>
#include "core/buffers/XHash.hpp"
#include "cluster/Interconnect.h"
#include "core/threads/Thread.hpp"

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
  SMS* sms;
};

typedef XHash<SMSId,LoadUpInfo> LoadUpHash;

const char LocalFileStore::storeSig[10]="SMSCSTORE";
const uint32_t LocalFileStore::storeVer=0x10001;


void LocalFileStore::Init(smsc::util::config::Manager* cfgman,Smsc* smsc)
{

  using namespace std;

  maxStoreSize=cfgman->getInt("MessageStore.LocalStore.maxStoreSize");
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

  bool delayInit=sched.delayInit;

  if(delayInit)
  {
    __trace__("schedulerMon->Unlock();");
    sched.mon.Unlock();
  }
  try{
    if(File::Exists(mainFileName.c_str()))
    {

      File *f=new File();
      f->ROpen(mainFileName.c_str());
      f->Rename((mainFileName+".bak").c_str());
      vector<File*> files;
      files.push_back(f);

      if(File::Exists(rolFileName.c_str()))
      {
        f=new File();
        f->ROpen(rolFileName.c_str());
        f->Rename((rolFileName+".bak").c_str());
        files.push_back(f);
      }

      InitPrimaryFile(mainFileName);
      if(sched.delayInit)sched.delayInit=false;

      for(vector<File*>::iterator it=files.begin();it!=files.end();it++)
      {
        File* pf=*it;
        char sigBuf[sizeof(storeSig)];
        uint32_t fileVer;
        LoadUpInfo item;
        item.sms=0;
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
            __warning2__("File version doesn't match current version:%d<%d",storeVer,fileVer);
            abort();
          }
          while(fPos<fSize)
          {
            sz=pf->ReadNetInt32();
            fPos+=4;
            if(fPos+sz>fSize)
            {
              __warning2__("Incomplete record detected, fPos=%lld, fSize=%lld, recSize=%d",fPos,fSize,sz);
              break;
            }
            if(sz<=8+4+1)
            {
              __warning2__("Store broken at fPos=%lld, fSize=%lld, recSize=%d",fPos,fSize,sz);
              abort();
            }
            item.id=pf->ReadNetInt64();
            fPos+=8;
            item.seq=pf->ReadNetInt32();
            fPos+=4;
            item.final=pf->ReadByte();
            fPos+=1;
            smsBuf.setSize(sz-8-4-1);
            pf->Read(smsBuf.get(),sz-8-4-1);
            fPos+=sz-8-4-1;
            sz2=pf->ReadNetInt32();
            fPos+=4;

            __trace2__("StoreInit: msgId=%lld, seq=%d, final=%s",item.id,item.seq,item.final?"true":"false");
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
                if(itemPtr->sms)
                {
                  delete itemPtr->sms;
                  itemPtr->sms=0;
                }
                itemPtr->final=true;
                continue;
              }
            }

            smsBuf.SetPos(0);
            if(!item.final)
            {
              item.sms=new SMS;
              Deserialize(smsBuf,*item.sms,fileVer);
            }else
            {
              item.sms=0;
            }

            if(itemPtr)
            {
              if(itemPtr->sms)delete itemPtr->sms;
              *itemPtr=item;
            }else
            {
              luHash.Insert(item.id,item);
              itemPtr=luHash.GetPtr(item.id);
              luVector.push_back(itemPtr);
            }
          };
          __trace2__("getfn=%s",pf->getFileName().c_str());
          toDelete.push_back(pf->getFileName());
        }catch(exception& e)
        {
          __warning2__("Operative storage read failed %s:%s",pf->getFileName().c_str(),e.what());
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
    __warning2__("Exception during storage init %s",e.what());
  }
  if(delayInit)
  {
    __trace__("schedulerMon->Lock();");
    sched.mon.Lock();
  }

  __trace2__("Local store loaded. %d messages found.",luVector.size());

  int cnt=0;
  for(LoadUpVector::iterator it=luVector.begin();it!=luVector.end();it++)
  {
    if((*it)->final)continue;
    if(!(*it)->sms)
    {
      __warning2__("Loading error!!! Sms point==NULL!!! msgId=%lld",(*it)->id);
      continue;
    }
    {
      MutexGuard mg(sched.storeMtx);
      Save((*it)->id,(*it)->seq,*(*it)->sms);
    }
    SMS& sms=*(*it)->sms;
    __trace2__("srcsmeid=%s",sms.getSourceSmeId());
    try{
      int smeIndex=smsc->getSmeIndex(sms.getSourceSmeId());
      if(delayInit)
      {
        sched.mon.Unlock();
      }
      try{
        sched.AddScheduledSms((*it)->id,sms,smeIndex);
      }catch(std::exception& e)
      {
        __warning2__("Exception in AddScheduledSms:'%s'",e.what());
      }

      cnt++;
      if(cnt%10)
      {
        smsc::core::threads::Thread::Yield();
      }

      Scheduler::StoreData* sd=new Scheduler::StoreData(sms,(*it)->seq);
      if(delayInit)
      {
        sched.mon.Lock();
      }

      {
        MutexGuard mg(sched.storeMtx);
        sched.store.Insert((*it)->id,sd);
        sd->rit=sched.replMap.insert(Scheduler::ReplaceIfPresentMap::value_type(&sd->sms,(*it)->id));
        sd->it=sched.currentSnap.insert(sched.currentSnap.begin(),IdSeqPair((*it)->id,(*it)->seq));
      }
    }catch(...)
    {
      __warning2__("systemId=%s not found. sms %lld dropped",sms.getSourceSmeId(),(*it)->id);
    }
    if((*it)->sms)
    {
      delete (*it)->sms;
      (*it)->sms=0;
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


bool LocalFileStore::Save(smsc::sms::SMSId id,uint32_t seq,const smsc::sms::SMS& sms,bool final)
{
  __trace2__("Save:%lld, %s->%s",id,
    sms.getOriginatingAddress().toString().c_str(),
    sms.getDestinationAddress().toString().c_str()
  );
  using namespace smsc::sms;
  using namespace smsc::sms::BufOps;
  SmsBuffer smsbuf(0);
  uint32_t sz=0;
  uint8_t fin=final?1:0;

  smsbuf<<sz<<id<<seq<<fin;

  Serialize(sms,smsbuf);
  sz=(uint32_t)(smsbuf.GetPos()-sizeof(sz));
  smsbuf.SetPos(0);
  smsbuf<<sz;
  smsbuf.SetPos(sz+sizeof(sz));
  smsbuf<<sz;
  MutexGuard mg(mtx);
  primaryFile.Write(smsbuf.get(),smsbuf.GetPos());
  fileSize+=smsbuf.GetPos();
  return !loadup && fileSize>maxStoreSize && time(NULL)-lastRollTime>minRollTime;
}


int LocalFileStore::Execute()
{
  MutexGuard mg(mon);
  while(running)
  {
    mon.wait();
    if(running && rolling)
    {
      info1(Scheduler::log,"Rolling strated");
      mon.Unlock();
      bool ok=true;
      try{
        int i=0;
        for(IdSeqPairList::iterator it=snap.begin();it!=snap.end();it++)
        {
          __trace2__("roll:id=%lld, seq=%d",it->first,it->second);
          sched.StoreSms(it->first,it->second);
          i++;
          if((i%10)==0)
          {
            Thread::Yield();
          }
        }
      }catch(exception& e)
      {
        warn2(Scheduler::log,"Exception during rolling:%s\n",e.what());
        ok=false;
      }
      mon.Lock();
      //delete snapPtr;
      //snapPtr=0;
      snap.swap(IdSeqPairList());
      rolling=false;
      if(ok)
      {
        info1(Scheduler::log,"Rolling finished ok");
        File::Unlink(rolFile.c_str());
      }else
      {
        File::Rename(rolFile.c_str(),(rolFile+".bad").c_str());
        warn1(Scheduler::log,"Rolling finished with error");
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
  archiveStorage.init(*cfgman);

}


void Scheduler::DelayInit(Smsc* psmsc,smsc::util::config::Manager* cfgman)
{
  archiveStorage.init(*cfgman);
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
          info2(log,"SMEALERT for %d",idx);
          int cnt=0;
          time_t sctime=time(NULL);
          SmeStatMap::iterator it=smeStatMap.find(idx);
          if(it==smeStatMap.end())continue;
          SmeStat::ChainSet::iterator cit=it->second.chainSet.begin();
          for(;cit!=it->second.chainSet.end();cit++)
          {
            Chain* c=*cit;
            RescheduleChain(c,sctime);
            cnt++;
            if(cnt==5)
            {
              sctime++;
              cnt=0;
            }
          }
        }catch(std::exception& e)
        {
          warn2(log,"Exception during SMEALERT:%s",e.what());
        }
      }else if(cmd->cmdid==HLRALERT)
      {
        info2(log,"HLRALERT: %s",cmd->get_address().toString().c_str());
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
          if(c->dpfPresent)
          {
            if(c->CancelMsgId(c->dpfId))DecSme(c);
            SMSId dpfId=c->dpfId;
            if(c->Count()==0)
            {
              debug2(log,"Try to delete chain %p/%s",c,c->addr.toString().c_str());
              DeleteChain(c);
            }else
            {
              RescheduleChain(c,time(NULL));
            }
            mon.Unlock();
            try{
              sendAlertNotification(dpfId,0);
            }catch(...){}
            mon.Lock();
            changeSmsStateToDeleted(dpfId);
          }else
          {
            RescheduleChain(c,time(NULL));
          }
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
  MutexGuard mg(storeMtx);

  SMSId rv=id;

  if(flag==smsc::store::SMPP_OVERWRITE_IF_PRESENT)
  {
    ReplaceIfPresentMap::iterator it=replMap.find(&sms);
    if(it!=replMap.end())
    {
      StoreData *ptr=store.Get(it->second);
      ptr->it->second=++(ptr->seq);
      LocalFileStoreSave(it->second,ptr->seq,ptr->sms,true);
      currentSnap.erase(ptr->it);
      replMap.erase(it);
      store.Delete(it->second);
      delStoreData(ptr);
    }
  }
  if(flag!=smsc::store::CREATE_NEW_NO_CLEAR)
  {
    sms.lastTime = 0;
    sms.lastResult = 0;
    sms.attempts = 0;
  }
  StoreData* sd=newStoreData(sms);
  store.Insert(id,sd);
  sd->rit=replMap.insert(ReplaceIfPresentMap::value_type(&sd->sms,id));
  sd->it=currentSnap.insert(currentSnap.end(),LocalFileStore::IdSeqPair(id,0));

  LocalFileStoreSave(id,0,sms);
  return rv;
}

void Scheduler::retriveSms(SMSId id, SMS &sms)
        throw(StorageException, NoSuchMessageException)
{
  MutexGuard mg(storeMtx);
  StoreData** ptr=store.GetPtr(id);
  if(!ptr)throw NoSuchMessageException();
  sms=(*ptr)->sms;
}

void Scheduler::changeSmsStateToEnroute(SMSId id,
    const Descriptor& dst, uint32_t failureCause,
    time_t nextTryTime, uint32_t attempts)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToEnroute: msgId=%lld",id);
  MutexGuard mg(storeMtx);
  StoreData** ptr=store.GetPtr(id);
  if(!ptr)throw NoSuchMessageException();
  SMS& sms=(*ptr)->sms;
  sms.state = ENROUTE;
  sms.destinationDescriptor=dst;
  sms.lastTime=time(NULL);
  sms.setLastResult(failureCause);
  sms.setNextTime(nextTryTime);
  sms.setAttemptsCount(attempts);
  (*ptr)->it->second=++(*ptr)->seq;
  LocalFileStoreSave(id,(*ptr)->seq,sms);
}

void Scheduler::changeSmsStateToDelivered(SMSId id,
    const Descriptor& dst)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToDelivered: msgId=%lld",id);
  MutexGuard mg(storeMtx);
  doFinalizeSms(id,smsc::sms::DELIVERED,smsc::system::Status::OK,dst);
}

void Scheduler::changeSmsStateToUndeliverable(SMSId id,
    const Descriptor& dst, uint32_t failureCause)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToUndeliverable: msgId=%lld",id);
  MutexGuard mg(storeMtx);
  doFinalizeSms(id,smsc::sms::UNDELIVERABLE,failureCause);
}

void Scheduler::changeSmsStateToExpired(SMSId id)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToExpired: msgId=%lld",id);
  MutexGuard mg(storeMtx);
  doFinalizeSms(id,smsc::sms::EXPIRED,smsc::system::Status::EXPIRED);
}

void Scheduler::changeSmsStateToDeleted(SMSId id)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToDeleted: msgId=%lld",id);
  MutexGuard mg(storeMtx);
  doFinalizeSms(id,smsc::sms::DELETED,smsc::system::Status::DELETED);
}

void Scheduler::changeSmsConcatSequenceNumber(SMSId id, int8_t inc)
        throw(StorageException, NoSuchMessageException)
{
  MutexGuard mg(storeMtx);
  StoreData** ptr=store.GetPtr(id);
  if(!ptr)throw NoSuchMessageException();
  SMS& sms=(*ptr)->sms;
  sms.setConcatSeqNum(sms.getConcatSeqNum()+inc);
  (*ptr)->it->second=++(*ptr)->seq;
  LocalFileStoreSave(id,(*ptr)->seq,sms);
}

void Scheduler::doFinalizeSms(SMSId id,smsc::sms::State state,int lastResult,const Descriptor& dstDsc)
{
  StoreData** ptr=store.GetPtr(id);
  if(!ptr)throw NoSuchMessageException();
  currentSnap.erase((*ptr)->it);
  StoreData* sd=*ptr;
  store.Delete(id);
  replMap.erase(sd->rit);
  sd->sms.state=state;
  sd->sms.lastResult=lastResult;
  sd->sms.lastTime = time(NULL);
  sd->sms.nextTime = 0;
  sd->sms.destinationDescriptor=dstDsc;

  LocalFileStoreSave(id,++sd->seq,sd->sms,true);
  if (sd->sms.needArchivate)
  {
    try{
      archiveStorage.createRecord(id, sd->sms);
    }catch(std::exception& e)
    {
      warn2(log,"archiveStorage.createRecord failed:%s",e.what());
    }
  }
  //if (sd->sms.billingRecord) billingStorage.createRecord(id, sd->sms);
  delStoreData(sd);
}


void Scheduler::replaceSms(SMSId id, SMS& sms)
                throw(StorageException, NoSuchMessageException)
{
  MutexGuard mg(storeMtx);
  StoreData** ptr=store.GetPtr(id);
  if(!ptr)throw NoSuchMessageException();
  (*ptr)->sms=sms;
  (*ptr)->it->second=++(*ptr)->seq;
  LocalFileStoreSave(id,(*ptr)->seq,sms);
}


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


}//system
}//smsc
