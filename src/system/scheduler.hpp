#ifndef __SMSC_SYSTEM_SCHEDULER_HPP__
#define __SMSC_SYSTEM_SCHEDULER_HPP__

#include "system/event_queue.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Mutex.hpp"
#include "system/sched_timer.hpp"
//#include "store/StoreManager.h"
#include "logger/Logger.h"
#include <map>
#include <list>
#include <vector>
#include <iterator>
#include "store/MessageStore.h"
#include "core/buffers/XHash.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "sms/sms_serializer.h"
#include "core/buffers/File.hpp"
#include "util/config/Manager.h"
#include "core/threads/Thread.hpp"
#include "smeman/smeproxy.h"
#include "core/buffers/FixedLengthString.hpp"
#include "store/FileStorage.h"
#include "system/dpfTracker.hpp"


namespace smsc{
namespace system{

using smsc::store::MessageStore;
using smsc::store::NoSuchMessageException;
using smsc::store::DuplicateMessageException;
using smsc::store::StorageException;

using namespace smsc::smeman;
using smsc::core::buffers::Array;
using smsc::core::buffers::IntHash;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::MutexGuard;

class Smsc;
class Scheduler;

class LocalFileStore:public smsc::core::threads::Thread{
public:

  static const char storeSig[10];
  static const uint32_t storeVer;

  typedef std::pair<SMSId,uint32_t> IdSeqPair;
  typedef std::list<IdSeqPair> IdSeqPairList;

  LocalFileStore(Scheduler& sc):sched(sc),loadup(true),running(true),rolling(false)
  {
  }

  void Stop()
  {
    mon.Lock();
    running=false;
    mon.notify();
    mon.Unlock();
  }

  int Execute();

  void InitPrimaryFile(const std::string& fn)
  {
    primaryFile.SetUnbuffered();
    primaryFile.WOpen(fn.c_str());
    primaryFile.Write(storeSig,sizeof(storeSig)-1);
    primaryFile.WriteNetInt32(storeVer);
    fileSize=0;
  }

  void Init(smsc::util::config::Manager* cfgman,Smsc* smsc);
  bool Save(smsc::sms::SMSId id,uint32_t seq,const char* smsBuf,int smsBufSize,bool final=false);
  bool StartRoll(const IdSeqPairList& argSnap);

protected:
  smsc::core::buffers::File primaryFile;
  uint64_t fileSize;
  Mutex mtx;
  Scheduler& sched;
  bool running;
  bool loadup;
  bool rolling;
  int maxStoreSize;
  time_t minRollTime,lastRollTime;
  IdSeqPairList snap;
  std::string rolFile;
  EventMonitor mon;
};

class Scheduler:public smsc::core::threads::ThreadedTask, public SmeProxy,public MessageStore
{
public:
  //Scheduler(EventQueue& eq):queue(eq)
  static smsc::logger::Logger* log;
  Scheduler():firstQueue(1000),localFileStore(*this)
  {
    smeSeq=0;
    prxmon=0;
    smsCount=0;
    log=smsc::logger::Logger::getInstance("sched");
    queueOrder=qoTimeLine;
    idSeq=0;
    lastIdSeqFlush=0;
    //currentSnap=new LocalFileStore::IdSeqPairList();
    lastRejectTime=0;
    lastRejectReschedule=0;
    delayInit=false;
  }
  ~Scheduler()
  {
    dpfTracker.stop();
    localFileStore.Stop();
    localFileStore.WaitFor();
    for(std::vector<StoreData*>::iterator it=storeDataPool.begin();it!=storeDataPool.end();it++)
    {
      delete *it;
    }
    for(std::vector<MultiChain*>::iterator it=mcPool.begin();it!=mcPool.end();it++)
    {
      delete *it;
    }
    SMSId key;
    StoreData* value;
    store.First();
    while(store.Next(key,value))
    {
      delete value;
    }
    for(TimeLine::TimeMap::iterator it=timeLine.tmap.begin();it!=timeLine.tmap.end();it++)
    {
      it->second->Clear();
      delete it->second;
    }
  }
  int Execute();
  const char* taskName(){return "scheduler";}

  void InitMsgId(smsc::util::config::Manager* cfgman)
  {
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
  }
  void Init(Smsc* psmsc,smsc::util::config::Manager* cfgman);
  void DelayInit(Smsc* psmsc,smsc::util::config::Manager* cfgman);

  void AddScheduledSms(SMSId id,const SMS& sms,SmeIndex idx)
  {
    MutexGuard guard(mon);

    Chain* c=GetProcessingChain(id);

    if(!c)c=GetChain(sms.getDealiasedDestinationAddress());
    if(!c)
    {
      c=CreateChain(sms.getNextTime(),sms.getDealiasedDestinationAddress(),idx);
    }
    SchedulerData sd(id,sms.getValidTime());
    ChainAddTimed(c,sms.getNextTime(),sd);
    debug2(log,"AddScheduledSms: time=%d, id=%lld,addr=%s,c=%p",sms.getNextTime(),id,sms.getDealiasedDestinationAddress().toString().c_str(),c);
    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
  }

  void UpdateSmsSchedule(SMSId id,const SMS& sms)
  {
    MutexGuard guard(mon);

    Chain* c=GetChain(sms.getDealiasedDestinationAddress());
    if(!c)
    {
      warn2(log,"UpdateSmsSchedule: chain for %s not found",sms.getDealiasedDestinationAddress().toString().c_str());
      return;
    }
    debug2(log,"UpdateSmsSchedule: time=%d,id=%lld,addr=%s,c=%p",sms.getNextTime(),id,sms.getDealiasedDestinationAddress().toString().c_str(),c);
    if(c->CancelMsgId(id))DecSme(c);
    ChainAddTimed(c,sms.getNextTime(),SchedulerData(id,sms.getValidTime()));
    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
  }

  void CancelSms(SMSId id,const Address& addr)
  {
    MutexGuard guard(mon);

    Chain* c=GetChain(addr);
    if(!c)
    {
      warn2(log,"CancelSms: chain for %s not found",addr.toString().c_str());
      return;
    }
    debug2(log,"CancelSms: id=%lld, c=%p, addr=%s",id,c,addr.toString().c_str());
    if(c->CancelMsgId(id))DecSme(c);
    if(c->Count()==0)
    {
      debug2(log,"Try to delete chain %p/%s",c,c->addr.toString().c_str());
      DeleteChain(c);
      return;
    }
    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
  }



  void DeliveryOk(SMSId id)
  {
    MutexGuard guard(mon);


    Chain* c=GetProcessingChain(id);
    if(!c)
    {
      debug2(log,"DeliverOk - chain not found for %lld",id);

      IdToTimeMap::iterator it=firstQueueProcessing.find(id);
      if(it!=firstQueueProcessing.end())
      {
        firstQueueProcessing.erase(it);
      }
      return;
    }
    debug2(log,"DeliverOk: id=%lld, c=%p",id,c);
    if(c->Count()==0)
    {
      debug2(log,"Try to delete chain %p/%s",c,c->addr.toString().c_str());
      DeleteChain(c);
      return;
    }
    RescheduleChain(c,time(NULL));
  }

  void InvalidSms(SMSId id)
  {
    MutexGuard guard(mon);

    Chain* c=GetProcessingChain(id);
    if(!c)
    {
      IdToTimeMap::iterator it=firstQueueProcessing.find(id);
      if(it==firstQueueProcessing.end())
      {
        warn2(log,"InvalidSms: processing chain for %lld not found!",id);
        return;
      }
      firstQueueProcessing.erase(it);
      return;
    }
    if(c->Count()==0)
    {
      debug2(log,"Try to delete chain %p/%s",c,c->addr.toString().c_str());
      DeleteChain(c);
      return;
    }

    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
  }

  void AddFirstTimeForward(SMSId id,const SMS& sms)
  {
    MutexGuard guard(mon);
    firstQueue.Push(SchedulerData(id,sms.getValidTime()));
  }

  void RejectForward(SMSId id)
  {
    MutexGuard guard(mon);

    Chain* c=GetProcessingChain(id);

    if(!c)
    {
      IdToTimeMap::iterator it=firstQueueProcessing.find(id);
      if(it==firstQueueProcessing.end())
      {
        warn2(log,"RejectForward: processing chain for %lld not found!",id);
        return;
      }
      firstQueue.Push(SchedulerData(it->first,it->second));
      firstQueueProcessing.erase(it);
      return;
    }
    time_t now=time(NULL);
    if(now==lastRejectTime)
    {
      lastRejectReschedule++;
    }else
    {
      lastRejectTime=now;
      lastRejectReschedule=now+5;
    }
    ChainSetHead(c,lastRejectReschedule,SchedulerData(id,c->lastValidTime));
    UpdateChainChedule(c);
    //c->headTime=lastRejectReschedule;
    RescheduleChain(c,lastRejectReschedule);
  }

  time_t RescheduleSms(SMSId id,SMS& sms,int smeIndex)
  {
    MutexGuard guard(mon);

    Chain* c=GetProcessingChain(id);

    if(!c)
    {
      IdToTimeMap::iterator it=firstQueueProcessing.find(id);
      if(it!=firstQueueProcessing.end())
      {
        firstQueueProcessing.erase(it);
      }
      c=GetChain(sms.getDealiasedDestinationAddress());
    }
    if(!c)
    {
      c=CreateChain(sms.getNextTime(),sms.getDealiasedDestinationAddress(),smeIndex);
    }

    time_t oldntt=sms.getNextTime();
    bool sethead=false;
    if(sms.attempts==0)
    {
      ChainPush(c,SchedulerData(id,sms.getValidTime()));
      debug2(log,"Resched: push sms %lld to tail (%d), c=%p",id,c->headTime,c);
    }else
    {
      debug2(log,"Resched: set sms %lld as head (%d),c=%p",id,sms.getNextTime(),c);
      ChainSetHead(c,sms.getNextTime(),SchedulerData(id,sms.getValidTime()));
      sethead=true;
    }
    //smsCount++;


    UpdateChainChedule(c);
    RescheduleChain(c,c->headTime);
    info2(log,"Resched: smsId=%lld,oa=%s,oldntt=%lu,newntt=%lu,hdt=%lu,sethead=%s",id,sms.getOriginatingAddress().toString().c_str(),oldntt,sms.getNextTime(),c->headTime,sethead?"true":"false");

    if(c->timedQueue.size()>0)
      return c->headTime;
    else
      return 0;
  }

  int getSmeCount(SmeIndex idx,time_t time)
  {
    MutexGuard g(mon);
    SmeStatMap::iterator it=smeStatMap.find(idx);
    if(it!=smeStatMap.end())
      return it->second.count;
    else
      return -1;
  }

  struct SchedulerCounts{
    int timeLineCount;
    int timeLineSize;
    int firstTimeCount;
    int regSize;
    int inProcCount;
  };

  void getCounts(SchedulerCounts& scCnt)
  {
    MutexGuard g(mon);

    scCnt.timeLineCount=smsCount;
    scCnt.timeLineSize=(int)timeLine.size();
    scCnt.firstTimeCount=firstQueue.Count();
    scCnt.regSize=(int)chainRegistry.size();
    scCnt.inProcCount=(int)procMap.size();
  }

  virtual void close(){}
  virtual void putCommand(const SmscCommand& command)
  {
    MutexGuard g(mon);
    outQueue.Push(command);
    mon.notify();

  }


  virtual bool getCommand(SmscCommand& cmd)
  {
    MutexGuard guard(mon);
    SchedulerData sd;
    if(queueOrder==qoFirstTime && ftPop(sd))
    {
      queueOrder=qoTimeLine;
    }else
    if(queueOrder==qoTimeLine && tlPop(sd))
    {
      queueOrder=qoFirstTime;
    }else if(!ftPop(sd) && !tlPop(sd))
    {
      return false;
    }
    cmd=SmscCommand::makeForward(sd.id,sd.resched);
    return true;
  }

  virtual int getCommandEx(std::vector<SmscCommand>& cmds,int& mx,SmeProxy* prx)
  {
    MutexGuard guard(mon);
    int cnt=0;
    bool order=true;
    SchedulerData sd;
    for(int i=0;i<mx;i++)
    {
      if((order && (ftPop(sd) || tlPop(sd))) || (!order && (tlPop(sd) || ftPop(sd))))
      {
        SmscCommand cmd=SmscCommand::makeForward(sd.id,sd.resched);
        cmd.setProxy(prx);
        cmds.push_back(cmd);
        cnt++;
        order=!order;
      }else
      {
        break;
      }
    }
    mx=cnt;
    return 0;
  }


  virtual SmeProxyState getState()const
  {
    return VALID;
  };
  virtual void init(){};
  virtual bool hasInput()const
  {
    MutexGuard guard(mon);
    return timeLine.size()>0 && timeLine.headTime()<=time(NULL);
  }
  virtual void attachMonitor(ProxyMonitor* monitor)
  {
    prxmon=monitor;
  }

  virtual bool attached()
  {
    return prxmon!=NULL;
  }
  virtual uint32_t getNextSequenceNumber()
  {
    MutexGuard g(seqMtx);
    return smeSeq++;
  }
  virtual const char * getSystemId()const
  {
    return "scheduler";
  }

  time_t getScheduleByAddr(const Address& addr)
  {
    MutexGuard g(mon);
    Chain *c=GetChain(addr);
    if(!c)return 0;
    return c->headTime;
  }


  struct ReplaceIfPresentKey{
    Address org;
    Address dst;
    FixedLengthString<6> esvctype;
    ReplaceIfPresentKey(const SMS& argSms)
    {
      org=argSms.getOriginatingAddress();
      dst=argSms.getDealiasedDestinationAddress();
      esvctype=argSms.getEServiceType();
    }
    string dump()const
    {
      return org.toString()+"/"+
             dst.toString()+"/"+
             esvctype.c_str();
    }
    bool operator<(const ReplaceIfPresentKey& rhs)const
    {
      return org<rhs.org ||
             (
               org==rhs.org &&
               dst<rhs.dst
             ) ||
             (
               org==rhs.org &&
               dst==rhs.dst &&
               esvctype<rhs.esvctype
             );
    }
  };
  typedef std::multimap<ReplaceIfPresentKey,SMSId> ReplaceIfPresentMap;
  ReplaceIfPresentMap replMap;


  struct StoreData
  {
    char* smsBuf;
    int smsBufSize;
    int seq;
    ReplaceIfPresentMap::iterator rit;
    LocalFileStore::IdSeqPairList::iterator it;

    void SaveSms(const SMS& argSms)
    {
      if(smsBuf!=0)
      {
        delete [] smsBuf;
        smsBuf=0;
      }
      BufOps::SmsBuffer buf(0);
      Serialize(argSms,buf);
      smsBufSize=(int)buf.GetPos();
      smsBuf=new char[smsBufSize];
      memcpy(smsBuf,buf.get(),smsBufSize);
    }

    void LoadSms(SMS& sms)
    {
      BufOps::SmsBuffer buf(smsBuf,smsBufSize);
      Deserialize(buf,sms,LocalFileStore::storeVer);
    }

    StoreData(char* argBuf,int argBufSize,int argSeq=0):seq(argSeq)
    {
      smsBuf=argBuf;
      smsBufSize=argBufSize;

    }

    StoreData(const SMS& argSms,int argSeq=0):seq(argSeq)
    {
      smsBuf=0;
      smsBufSize=0;
      SaveSms(argSms);
    }
    ~StoreData()
    {
      if(smsBuf)
      {
        delete [] smsBuf;
      }
    }
    protected:
      StoreData(const StoreData&){}
  };
  struct SMSIdHashFunc{
    static inline unsigned int CalcHash(SMSId key)
    {
      return (unsigned int)key;
    }
  };
  smsc::core::buffers::XHash<SMSId,StoreData*,SMSIdHashFunc> store;
  Mutex storeMtx;
  std::vector<StoreData*> storeDataPool;

  int getStoreSize()
  {
    return store.Count();
  }

  StoreData* newStoreData(const SMS& argSms,int argSeq=0)
  {
    if(storeDataPool.empty())
    {
      return new StoreData(argSms,argSeq);
    }else
    {
      StoreData* sd=storeDataPool.back();
      storeDataPool.pop_back();
      sd->SaveSms(argSms);
      sd->seq=argSeq;
      return sd;
    }
  }
  void delStoreData(StoreData* sd)
  {
    if(storeDataPool.size()<10000)
    {
      storeDataPool.push_back(sd);
    }else
    {
      delete sd;
    }
  }

  enum{MessageIdSequenceExtent=1000};
  SMSId idSeq;
  int   lastIdSeqFlush;
  File  idFile;
  Mutex idMtx;
  LocalFileStore::IdSeqPairList currentSnap;


  friend class LocalFileStore;
  LocalFileStore localFileStore;

  void LocalFileStoreSave(smsc::sms::SMSId id,StoreData* sd,bool final=false)
  {
    while(delayInit)
    {
      sched_yield();
    }
    if(localFileStore.Save(id,sd->seq,sd->smsBuf,sd->smsBufSize,final))
    {
      localFileStore.StartRoll(currentSnap);
    }
  }

  void StoreSms(smsc::sms::SMSId id,uint32_t seq)
  {
    MutexGuard mg(storeMtx);
    StoreData** sd=store.GetPtr(id);
    if(!sd)return;
    if((*sd)->seq!=seq)return;
    localFileStore.Save(id,(*sd)->seq,(*sd)->smsBuf,(*sd)->smsBufSize);
  }


        virtual SMSId getNextId();

        virtual SMSId createSms(SMS& sms, SMSId id,const smsc::store::CreateMode flag = smsc::store::CREATE_NEW)
                throw(StorageException, DuplicateMessageException);

        virtual void retriveSms(SMSId id, SMS &sms)
                throw(StorageException, NoSuchMessageException);

        virtual void replaceSms(SMSId id, const Address& oa,
            const uint8_t* newMsg, uint8_t newMsgLen,
            uint8_t deliveryReport, time_t validTime = 0, time_t nextTime = 0)
                throw(StorageException, NoSuchMessageException)
        {
          __warning__("replaceSms unimplemented");
          fprintf(stderr,"replaceSms unimplemented");
        }
        virtual void replaceSms(SMSId id, SMS& sms)
                throw(StorageException, NoSuchMessageException);

        virtual void changeSmsStateToEnroute(SMSId id,
            const Descriptor& dst, uint32_t failureCause,
            time_t nextTryTime, uint32_t attempts)
                throw(StorageException, NoSuchMessageException);


        virtual void changeSmsStateToDelivered(SMSId id,
            const Descriptor& dst)
                throw(StorageException, NoSuchMessageException);

        virtual void changeSmsStateToUndeliverable(SMSId id,
            const Descriptor& dst, uint32_t failureCause)
                throw(StorageException, NoSuchMessageException);


        virtual void changeSmsStateToExpired(SMSId id)
                throw(StorageException, NoSuchMessageException);

        virtual void changeSmsStateToDeleted(SMSId id)
                throw(StorageException, NoSuchMessageException);

        virtual void createFinalizedSms(SMSId id, SMS& sms)
                throw(StorageException, DuplicateMessageException)
        {
          sms.lastTime = time(NULL);
          if (sms.needArchivate) archiveStorage.createRecord(id, sms);
          //if (sms.billingRecord) billingStorage.createRecord(id, sms);
        }

        virtual void changeSmsConcatSequenceNumber(SMSId id, int8_t inc=1)
                throw(StorageException, NoSuchMessageException);


        virtual int getConcatMessageReference(const Address& dda)
                throw(StorageException)
        {
          return 0;
        }
        virtual void destroySms(SMSId id)
                throw(StorageException, NoSuchMessageException)
        {
          __warning__("destroySms unimplemented");
          fprintf(stderr,"destroySms unimplemented");

        }
        virtual smsc::store::IdIterator* getReadyForDelivery(const Address& da)
                throw(StorageException)
        {
          __warning__("getReadyForDelivery unimplemented");
          fprintf(stderr,"getReadyForDelivery unimplemented");
          return 0;
        }
        virtual smsc::store::IdIterator* getReadyForCancel(const Address& oa,
            const Address& da, const char* svcType = 0)
                throw(StorageException)
        {
          __warning__("getReadyForCancel unimplemented");
          fprintf(stderr,"getReadyForCancel unimplemented");
          return 0;
        }
        virtual smsc::store::TimeIdIterator* getReadyForRetry(time_t retryTime, bool immediate=false)
                throw(StorageException)
        {
          __warning__("getReadyForRetry unimplemented");
          fprintf(stderr,"getReadyForRetry unimplemented");
          return 0;
        }
        virtual time_t getNextRetryTime()
                throw(StorageException)
        {
          __warning__("getNextRetryTime unimplemented");
          fprintf(stderr,"getNextRetryTime unimplemented");
          return 0;
        }

        void doFinalizeSms(SMSId id,smsc::sms::State state,int lastResult,const Descriptor& dstDsc=Descriptor());


        void getMassCancelIds(const SMS& sms,Array<SMSId>& ids)
        {
          MutexGuard mg(storeMtx);
          ReplaceIfPresentMap::iterator from=replMap.lower_bound(sms);
          if(from!=replMap.end())
          {
            ReplaceIfPresentMap::iterator to=replMap.upper_bound(sms);
            for(;from!=to;from++)
            {
              debug2(log,"id for cancel:%lld",from->second);
              ids.Push(from->second);
            }
          }else
          {
            debug2(log,"getMassCancelIds nothing found:%s",ReplaceIfPresentKey(sms).dump().c_str());
          }
        }

        void InitDpfTracker(const char* storeLocation,int to1179,int to1044,int mxch,int mxt)
        {
          dpfTracker.Init(storeLocation,to1179,to1044,mxch,mxt);
        }

        void registerSetDpf(const smsc::sms::Address& abonent,const smsc::sms::Address &smeAddr,int errCode,const char* smeId)
        {
          dpfTracker.registerSetDpf(abonent,smeAddr,errCode,smeId);
        }

public:
  struct StartupItem{
    SMSId id;
    time_t schedTime;
    int attCount;
    Address addr;
    time_t validTime;
    char smeId[32];
  };

  Smsc* smsc;

  //EventQueue &queue;

  mutable EventMonitor mon;
  Array<SmscCommand> outQueue;

  struct SchedulerData{
    SchedulerData():id(0),expDate(0),resched(false){}
    SchedulerData(const SchedulerData& d)
    {
      id=d.id;
      resched=d.resched;
      expDate=d.expDate;
    }
    SchedulerData(SMSId argId,time_t argExpDate,bool argRes=false):id(argId),expDate(argExpDate),resched(argRes){}
    SMSId id;
    time_t expDate;
    bool resched;

    bool operator<(const SchedulerData& rhs)const
    {
      return expDate<rhs.expDate;
    }
  };

  bool tlPop(SchedulerData& d)
  {
    time_t t=time(NULL);
    if(timeLine.size()!=0)
    {
      static time_t lastUpdate=time(NULL)-1;
      if(lastUpdate!=t)
      {
        debug2(log,"headTime=%d, now=%d",timeLine.headTime(),t);
        lastUpdate=t;
      }
    }else
    {
      //debug(log,"Sc: queue empty");
    }
    if(timeLine.size()==0 || timeLine.headTime()>t)
    {
      return false;
    }
    Chain* c=timeLine.Pop(this);
    if(!c)return false;
    //d.fake=false;
    if(!ChainPop(c,d))
    {
      /*
      if(d.fake)
      {
        sendAlertNotification(d.id,1);
        try{
          changeSmsStateToDeleted(d.id);
        }catch(std::exception& e)
        {
          warn2(log,"Failed to delete fake sms %lld",d.id);
        }
      }
      */

      UpdateChainChedule(c);
      debug2(log,"Chain::Pop failed, rescheduled to %d",c->headTime);
      if(c->Count()==0)
      {
        if(!DeleteChain(c))
        {
          warn2(log,"SHIT HAPPENED WITH CHAIN %p, addr=%s, intl=%s,inproc=%d",
            c,c->addr.toString().c_str(),c->inTimeLine?"true":"false",c->inProcMap);
        }
      }else
      {
        timeLine.Add(c,this);
      }
      return false;
    }

    debug2(log,"makeFwd: id=%lld, addr=%s, c=%p",d.id,c->addr.toString().c_str(),c);
    AddProcessingChain(d.id,c);
    return true;
  }

  bool ftPop(SchedulerData& sd)
  {
    if(firstQueue.Count())
    {
      firstQueue.Pop(sd);
      firstQueueProcessing.insert(IdToTimeMap::value_type(sd.id,sd.expDate));
      debug2(log,"firstForward: id=%lld",sd.id);
      return true;
    }
    return false;
  }

  struct Chain{
    Address addr;
    time_t headTime;
    time_t lastValidTime;
    time_t inProcMap;
    int smeIndex;
    int queueSize;
    bool inTimeLine;

    typedef std::multiset<SchedulerData> ScQueue;
    ScQueue queue;
    typedef std::multimap<time_t,SchedulerData> ScTimedQueue;
    ScTimedQueue timedQueue;

    struct Iterators{
      ScQueue::iterator li;
      ScTimedQueue::iterator mi;
    };
    typedef std::map<SMSId,Iterators> Id2IteratorsMap;
    Id2IteratorsMap imap;

    Chain(time_t sctime,const Address& argAddr,int argSmeIndex)
    {
      addr=argAddr;
      smeIndex=argSmeIndex;
      headTime=sctime;
      inTimeLine=false;
      inProcMap=0;
      queueSize=0;
    }

    bool AddTimed(time_t sctime,const SchedulerData& d)
    {
      if(imap.find(d.id)!=imap.end())
      {
        debug2(Scheduler::log,"Chain::AddTimed, id=%lld - already in the chain!!!",d.id);
        return false;
      }
      Iterators its;
      debug2(Scheduler::log,"Chain::AddTimed, time=%d, id=%lld",sctime,d.id);
      its.mi=timedQueue.insert(ScTimedQueue::value_type(sctime,d));
      its.li=queue.end();
      imap.insert(Id2IteratorsMap::value_type(d.id,its));
      queueSize++;
      return true;
    }

    time_t Reschedule(time_t newtime)
    {
      if(!timedQueue.empty())
      {
        time_t frontTime=timedQueue.begin()->first;
        if(frontTime<newtime)newtime=frontTime;
      }
      if(!queue.empty())
      {
        time_t frontTime=queue.begin()->expDate;
        if(frontTime<newtime)newtime=frontTime;
      }
      headTime=newtime;
      return headTime;
    }

    bool Push(const SchedulerData& d)
    {
      if(imap.find(d.id)!=imap.end())
      {
        debug2(Scheduler::log,"Chain::Push, id=%lld - already in the chain!!!",d.id);
        return false;
      }
      debug2(Scheduler::log,"Chain::Push, id=%lld",d.id);
      Iterators its;
      its.li=queue.insert(d);
      its.mi=timedQueue.end();
      imap.insert(Id2IteratorsMap::value_type(d.id,its));
      queueSize++;
      return true;
    }

    bool SetHead(time_t sctime,const SchedulerData& d)
    {
      if(imap.find(d.id)!=imap.end())
      {
        debug2(Scheduler::log,"Chain::SetHead, id=%lld - already in the chain!!!",d.id);
        return false;
      }
      Iterators its;
      its.li=queue.insert(d);
      its.mi=timedQueue.end();
      imap.insert(Id2IteratorsMap::value_type(d.id,its));
      if(!inTimeLine)
      {
        time_t expDate=queue.begin()->expDate;
        if(expDate>sctime)
        {
          headTime=sctime;
        }else
        {
          headTime=expDate;
        }
      }
      debug2(Scheduler::log,"Chain::SetHead, time=%d/%d, id=%lld",sctime,headTime,d.id);
      queueSize++;
      return true;
    }

    bool Pop(SchedulerData& d)
    {
      if(!timedQueue.empty())
      {
        ScTimedQueue::iterator i=timedQueue.begin();
        if(i->first<=time(NULL))
        {
          d=i->second;
          debug2(Scheduler::log,"Chain::Pop(timed), id=%lld",d.id);
          timedQueue.erase(i);
          imap.erase(imap.find(d.id));
          queueSize--;
          lastValidTime=d.expDate;
          return true;
        }
      }
      if(queue.empty())return false;
      d=*queue.begin();
      lastValidTime=d.expDate;
      debug2(Scheduler::log,"Chain::Pop(queue), id=%lld",d.id);
      queue.erase(queue.begin());
      imap.erase(imap.find(d.id));
      queueSize--;
      return true;
    }

    bool CancelMsgId(SMSId id)
    {
      Id2IteratorsMap::iterator it=imap.find(id);
      if(it==imap.end())
      {
        return false;
      }
      if(it->second.li!=queue.end())
      {
        queue.erase(it->second.li);
      }else
      {
        timedQueue.erase(it->second.mi);
      }
      imap.erase(it);
      queueSize--;
      return true;
    }

    int Count()
    {
      return queueSize;
    }

    time_t getTimedHead()
    {
      if(timedQueue.empty())return 0;
      return timedQueue.begin()->first;
    }

  };

  typedef std::map<Address,Chain*> ChainRegistry;
  ChainRegistry chainRegistry;

  Chain* GetChain(const Address& addr)
  {
    ChainRegistry::iterator it=chainRegistry.find(addr);
    if(it==chainRegistry.end())
    {
      return 0;
    }
    return it->second;
  }

  Chain* CreateChain(time_t sctime,const Address& addr,int smeIndex)
  {
    Chain* rv=new Chain(sctime,addr,smeIndex);
    debug2(log,"newChain: %p/%s",rv,addr.toString().c_str());
    chainRegistry.insert(ChainRegistry::value_type(addr,rv));
    return rv;
  }

  bool DeleteChain(Chain* c)
  {
    if(c->inTimeLine || c->inProcMap)
    {
      debug2(log,"Do not delete c=%p",c);
      return false;
    }
    ChainRegistry::iterator it=chainRegistry.find(c->addr);
    if(it!=chainRegistry.end())
    {
      chainRegistry.erase(it);
    }
    SmeStatMap::iterator ssit=smeStatMap.find(c->smeIndex);
    if(ssit!=smeStatMap.end())
    {
      SmeStat& ss=ssit->second;
      if(ss.chainSet.find(c)!=ss.chainSet.end())
      {
        warn2(log,"ERROR, chain %p found in chainset of smsstat!",c);
        ss.chainSet.erase(c);
      }
    }
    debug2(log,"deleteChain: %p",c);
    delete c;
    return true;
  }

  struct SmeStat{
    int count;
    typedef std::set<Chain*> ChainSet;
    ChainSet chainSet;

    SmeStat():count(0){}

    void Inc(Chain* c)
    {
      count++;
      chainSet.insert(c);
    }
    void Dec(Chain* c)
    {
      count--;
      if(c->Count()==0)
      {
        ChainSet::iterator it=chainSet.find(c);
        if(it!=chainSet.end())
        {
          chainSet.erase(it);
        }
      }
    }
  };

  typedef std::map<int,SmeStat> SmeStatMap;
  SmeStatMap smeStatMap;

  void IncSme(Chain* c)
  {
    smsCount++;
    SmeStatMap::iterator it=smeStatMap.find(c->smeIndex);
    if(it!=smeStatMap.end())
    {
      it->second.Inc(c);
    }else
    {
      //warn2(log,"IncSme - smeIndex %d not found in stat map!",c->smeIndex);
      it=smeStatMap.insert(SmeStatMap::value_type(c->smeIndex,SmeStat())).first;
      it->second.Inc(c);

    }
  }
  void DecSme(Chain* c)
  {
    smsCount--;
    SmeStatMap::iterator it=smeStatMap.find(c->smeIndex);
    if(it!=smeStatMap.end())
    {
      it->second.Dec(c);
    }else
    {
      warn2(log,"DecSme - smeIndex %d not found in stat map!",c->smeIndex);
    }
  }

  typedef std::map<SMSId,Chain*> ProcessingMap;
  ProcessingMap procMap;

  void AddProcessingChain(SMSId id,Chain* c)
  {
    c->inProcMap=time(NULL);
    procMap.insert(ProcessingMap::value_type(id,c));
  }

  Chain* GetProcessingChain(SMSId id)
  {
    ProcessingMap::iterator it=procMap.find(id);
    if(it==procMap.end())return 0;
    Chain* rv=it->second;
    procMap.erase(it);
    rv->inProcMap=0;
    return rv;
  }

  struct MultiChain{
    typedef std::multimap<Address,Chain*> ChainMap;
    ChainMap cmap;
    void Push(Chain* c)
    {
      ChainMap::iterator it=cmap.lower_bound(c->addr);
      if(it==cmap.end() || it->first!=c->addr)
      {
        cmap.insert(it,ChainMap::value_type(c->addr,c));
        c->inTimeLine=true;
        debug2(Scheduler::log,"mc:push: c=%p",c);
      }else
      {
        warn2(Scheduler::log,"!!! CHAIN ALREADY IN MULTICHAIN(%s): %p <-> %p!",c->addr.toString().c_str(),c,it->second);
      }
    }
    Chain* Pop()
    {
      if(cmap.size()==0)return 0;
      ChainMap::iterator it=cmap.begin();
      Chain* rv=it->second;
      cmap.erase(it);
      rv->inTimeLine=false;
      debug2(Scheduler::log,"mc:pop: c=%p",rv);
      return rv;
    }
    void RemoveChain(Chain* c)
    {
      ChainMap::iterator i=cmap.find(c->addr);
      if(i==cmap.end())
      {
        debug2(Scheduler::log,"Chain not found in multichain %p/%s",c,c->addr.toString().c_str());
        return;
      }
      debug2(Scheduler::log,"mc:remove: c=%p",i->second);
      i->second->inTimeLine=false;
      cmap.erase(i);
    }

    void Clear()
    {
      for(ChainMap::iterator it=cmap.begin();it!=cmap.end();it++)
      {
        delete it->second;
      }
    }
  };

  std::vector<MultiChain*> mcPool;

  MultiChain* newMultiChain()
  {
    if(mcPool.size()>0)
    {
      MultiChain* rv=mcPool.back();
      mcPool.pop_back();
      return rv;
    }
    return new MultiChain;
  }
  void deleteMultiChain(MultiChain* mc)
  {
    if(mcPool.size()<5000)
    {
      mcPool.push_back(mc);
    }else
    {
      delete mc;
    }
  }


  struct TimeLine{
    typedef std::multimap<const time_t,MultiChain*> TimeMap;
    TimeMap tmap;


    void Add(Chain* c,Scheduler* sc)
    {
      if(c->inTimeLine || c->inProcMap)
      {
        return;
      }
      debug2(Scheduler::log,"Sc: add to timeLine %p, addr=%s, headTime=%d",c,c->addr.toString().c_str(),c->headTime);
      TimeMap::iterator it=tmap.lower_bound(c->headTime);
      if(it==tmap.end() || it->first!=c->headTime)
      {
        MultiChain* mc=sc->newMultiChain();
        debug2(Scheduler::log,"Sc: create new multichain %p",mc);
        mc->Push(c);
        tmap.insert(it,TimeMap::value_type(c->headTime,mc));
      }else
      {
        it->second->Push(c);
      }
    }

    Chain* Pop(Scheduler* sc)
    {
      if(tmap.size()==0)
      {
        debug1(Scheduler::log,"Sc: pop return 0, tmap is empty");
        return 0;
      }
      TimeMap::iterator i=tmap.begin();
      MultiChain* mc=i->second;
      Chain* rv=mc->Pop();
      if(rv)
      {
        debug2(Scheduler::log,"TimeLine::Pop %d/%p (headTime=%d,now=%d)",i->first,i->second,rv->headTime,time(NULL));
        return rv;
      }
      debug1(Scheduler::log,"Sc: deleting empty multichain");
      sc->deleteMultiChain(mc);
      tmap.erase(i);
      return 0;
    }

    void RemoveChain(Chain* c)
    {
      TimeMap::iterator i=tmap.find(c->headTime);
      if(i==tmap.end())
      {
        debug2(Scheduler::log,"Multichain not found for chain %p/%s, at time %d",c,c->addr.toString().c_str(),c->headTime);
        return;
      }
      i->second->RemoveChain(c);
    }

    size_t size()const
    {
      return tmap.size();
    }

    time_t headTime()const
    {
      if(tmap.size()>0)
      {
        return tmap.begin()->first;
      }else
      {
        return 0;
      }
    }

  };

  CyclicQueue<SchedulerData> firstQueue;
  TimeLine timeLine;
  typedef std::map<SMSId,time_t> IdToTimeMap;
  IdToTimeMap firstQueueProcessing;

  void RescheduleChain(Chain* c,time_t sctime)
  {
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
        return;
      }
    }
    if(c->inTimeLine)
    {
      timeLine.RemoveChain(c);
    }
    c->Reschedule(sctime);
    timeLine.Add(c,this);
  }

  void UpdateChainChedule(Chain* c)
  {
    time_t sctime=c->getTimedHead();
    if(sctime!=0)
    {
      if(sctime<c->headTime || (sctime>c->headTime && c->queue.empty()))
      {
        if(!c->inTimeLine)
        {
          c->headTime=sctime;
        }
      }
    }
  }

  void ChainPush(Chain* c,const SchedulerData& d)
  {
    if(c->Push(d))IncSme(c);
  }
  void ChainSetHead(Chain* c,time_t sctime,const SchedulerData& d)
  {
    if(c->SetHead(sctime,d))IncSme(c);
  }
  void ChainAddTimed(Chain* c,time_t sctime,const SchedulerData& d)
  {
    if(c->AddTimed(sctime,d))IncSme(c);
  }
  bool ChainPop(Chain* c,SchedulerData& d)
  {
    if(c->Pop(d))
    {
      DecSme(c);
      return true;
    }else
    {
      return false;
    }

  }

  time_t tlHeadTime()
  {
    MutexGuard mg(mon);
    return timeLine.headTime();
  }

  //void sendAlertNotification(SMSId id,int status);

  int smsCount;

  enum QueueOrder{qoTimeLine,qoFirstTime};
  QueueOrder queueOrder;


  Mutex seqMtx;
  int smeSeq;
  ProxyMonitor* prxmon;

  //smsc::store::BillingStorage  billingStorage;
  smsc::store::ArchiveStorage  archiveStorage;
  DpfTracker dpfTracker;

  time_t lastRejectTime;
  time_t lastRejectReschedule;

  bool delayInit;
  public:
  bool getChainInfo(const Address& addr,Chain& out)
  {
    MutexGuard g(mon);
    Chain *c=GetChain(addr);
    if(!c)return false;
    out=*c;
    return true;
  }
};

}//system
}//smsc


#endif
