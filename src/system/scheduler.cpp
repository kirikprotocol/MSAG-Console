#include "system/smsc.hpp"
#include "system/scheduler.hpp"
#include "util/Exception.hpp"
#include <string>
#include <vector>
#include "core/buffers/XHash.hpp"

namespace smsc{
namespace system{

smsc::logger::Logger* Scheduler::log;

using namespace std;

struct LoadUpInfo;
typedef vector<LoadUpInfo*> LoadUpVector;

struct LoadUpInfo{
  SMSId id;
  uint32_t seq;
  bool final;
  SMS sms;
};

typedef XHash<SMSId,LoadUpInfo> LoadUpHash;

const char LocalFileStore::storeSig[10]="SMSCSTORE";
const uint32_t LocalFileStore::storeVer=0x10000;


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

    for(vector<File*>::iterator it=files.begin();it!=files.end();it++)
    {
      File* pf=*it;
      char sigBuf[sizeof(storeSig)];
      uint32_t fileVer;
      LoadUpInfo item;
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
        if(fileVer!=storeVer)
        {
          __warning2__("File version doesn't match current version:%d!=%d",storeVer,fileVer);
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
            if(itemPtr->final || itemPtr->seq>item.seq)continue;
          }

          smsBuf.SetPos(0);
          Deserialize(smsBuf,item.sms);

          if(itemPtr)
          {
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
  }


  InitPrimaryFile(mainFileName);

  __trace2__("Local store loaded. %d messages found.",luVector.size());

  for(LoadUpVector::iterator it=luVector.begin();it!=luVector.end();it++)
  {
    if((*it)->final)continue;
    Save((*it)->id,(*it)->seq,(*it)->sms);
    SMS& sms=(*it)->sms;
    __trace2__("srcsmeid=%s",sms.getSourceSmeId());
    try{
      int smeIndex=smsc->getSmeIndex(sms.getSourceSmeId());
      sched.AddScheduledSms((*it)->id,sms,smeIndex);
      Scheduler::StoreData* sd=new Scheduler::StoreData(sms,(*it)->seq);
      sched.store.Insert((*it)->id,sd);
      sd->rit=sched.replMap.insert(Scheduler::ReplaceIfPresentMap::value_type(&sd->sms,(*it)->id));
      sd->it=sched.currentSnap.insert(sched.currentSnap.begin(),IdSeqPair((*it)->id,(*it)->seq));
    }catch(...)
    {
      __warning2__("systemId=%s not found. sms %lld dropped",sms.getSourceSmeId(),(*it)->id);
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
  sz=smsbuf.GetPos()-sizeof(sz);
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
      __trace__("rolling strated");
      mon.Unlock();
      bool ok=true;
      try{
        for(IdSeqPairList::iterator it=snap.begin();it!=snap.end();it++)
        {
          __trace2__("roll:id=%lld, seq=%d",it->first,it->second);
          sched.StoreSms(it->first,it->second);
        }
      }catch(exception& e)
      {
        __warning2__("Exception during rolling:%s\n",e.what());
        ok=false;
      }
      mon.Lock();
      //delete snapPtr;
      //snapPtr=0;
      snap.swap(IdSeqPairList());
      rolling=false;
      if(ok)
      {
        __trace__("rolling finished ok");
        File::Unlink(rolFile.c_str());
      }else
      {
        File::Rename(rolFile.c_str(),(rolFile+".bad").c_str());
        __trace__("rolling finished error");
      }
    }
  }
  return 0;
}

void Scheduler::Init(Smsc* psmsc,smsc::util::config::Manager* cfgman)
{
  smsc=psmsc;
  localFileStore.Init(cfgman,psmsc);
  billingStorage.init(*cfgman);
  archiveStorage.init(*cfgman);

  const char* idFileName=cfgman->getString("MessageStore.LocalStore.msgidfile");
  if(File::Exists(idFileName))
  {
    idFile.RWOpen(idFileName);
    idSeq=idFile.ReadNetInt64();
    idSeq+=MessageIdSequenceExtent;
    idFile.Seek(0);
    idFile.WriteNetInt64(idSeq);
  }else
  {
    idFile.WOpen(idFileName);
    idSeq=0;
    idFile.WriteNetInt64(idSeq);
  }
  idFile.SetUnbuffered();

  /*
  smsc::store::TimeIdIterator* it=st->getReadyForRetry(time(NULL)+30*24*60*60);
  if(it)
  {
    debug1(log,"Scheduler: start init");
    MutexGuard guard(mon);
    SMSId id;
    try{
      StartupItem si;
      while(it->getNextId(id))
      {
        FullAddressValue ddabuf;
        if(it->getDstSmeId(si.smeId) && it->getDda(ddabuf))
        {
          si.id=id;
          si.schedTime=it->getTime();
          si.attCount=it->getAttempts();
          si.addr=ddabuf;
          si.validTime=it->getValidTime();
          startupCache.Push(si);
          if(startupCache.Count()%1000==0)debug2(log,"Loading scheduler - %d",startupCache.Count());
        }
      }
      info2(log,"Scheduler: init ok - %d messages for rescheduling",startupCache.Count());
    }catch(std::exception& e)
    {
      warn2(log,"Scheduler:failed to init scheduler timeline:%s",e.what());
    }catch(...)
    {
      warn1(log,"Scheduler:failed to init scheduler timeline:unknown");
    }
    delete it;
  }else
  {
    info1(log,"Scheduler: init - No messages found");
  }
  */
}


int Scheduler::Execute()
{
  smsc::logger::Logger* smsLog=smsc::logger::Logger::getInstance("sms.trace");
  mon.Lock();
  try{
    for(int i=0;i<startupCache.Count();i++)
    {
      StartupItem& si=startupCache[i];
      SmeIndex idx=INVALID_SME_INDEX;
      try{
        idx=smsc->getSmeIndex(si.smeId);
      }catch(...)
      {
        warn2(log,"failed to get sme index for %s",si.smeId);
        continue;
      }

      if(idx!=INVALID_SME_INDEX)
      {
        Chain* c=GetChain(si.addr);
        if(!c)
        {
          c=CreateChain(si.schedTime,si.addr,idx);
        }
        if(si.attCount==0)
        {
          ChainAddTimed(c,si.schedTime,SchedulerData(si.id,si.validTime));
        }else
        {
          ChainPush(c,SchedulerData(si.id,si.validTime));
        }
        timeLine.Add(c,this);
        //smsCount++;
      }
      mon.Unlock();
      sched_yield();
      mon.Lock();;
    }
  }catch(std::exception& e)
  {
    warn2(log,"error during scheduler initialization: %s",e.what());
  }
  mon.Unlock();

  time_t t=time(NULL);

  time_t lastBillRoll=t;
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
        int idx=cmd->get_smeIndex();
        debug2(log,"SMEALERT for %d",idx);
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
      }else if(cmd->cmdid==HLRALERT)
      {
        info2(smsLog,"SCALERT:%s",cmd->get_address().toString().c_str());
        Chain* c=GetChain(cmd->get_address());
        if(!c)continue;
        if(c->inProcMap)continue;
        RescheduleChain(c,time(NULL));
      }
    }

    if(t-lastBillRoll>billingStorage.getStorageInterval())
    {
      billingStorage.roll();
      lastBillRoll=t;
    }
    if(t-lastArcRoll>archiveStorage.getStorageInterval())
    {
      archiveStorage.roll();
      lastArcRoll=t;
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
  billingStorage.close();
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
  sms.lastTime = 0;
  sms.lastResult = 0;
  sms.attempts = 0;
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
  sms.destinationDescriptor=dst;
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
  doFinalizeSms(id,smsc::sms::DELIVERED,smsc::system::Status::OK);
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

void Scheduler::doFinalizeSms(SMSId id,smsc::sms::State state,int lastResult)
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

  LocalFileStoreSave(id,++sd->seq,sd->sms,true);
  if (sd->sms.needArchivate) archiveStorage.createRecord(id, sd->sms);
  if (sd->sms.billingRecord) billingStorage.createRecord(id, sd->sms);
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


}//system
}//smsc
