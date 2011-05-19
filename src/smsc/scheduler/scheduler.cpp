#include <string>
#include <vector>

#include "core/buffers/HashTree.hpp"
#include "core/threads/Thread.hpp"
#include "smsc/smsc.hpp"
#include "util/Exception.hpp"
#include "util/vformat.hpp"

#include "scheduler.hpp"

namespace smsc{
namespace scheduler{

smsc::logger::Logger* Scheduler::log;

using namespace smsc::core::synchronization;

using namespace smsc::util;
using namespace std;

const char LocalFileStore::storeSig[10]="SMSCSTORE";
const uint32_t LocalFileStore::storeVer=0x10001;

struct LoadFileInfo{
  LoadFileInfo(const string& argFileName,const string& argTimeStamp):
    fileName(argFileName),timeStamp(argTimeStamp),badFile(false),store(0){}
  string fileName;
  string timeStamp;
  bool badFile;
  Scheduler::StoreFileData* store;

  bool operator<(const LoadFileInfo& other)const
  {
    return timeStamp<other.timeStamp;
  }

};

static std::string mkTimeStamp(time_t val=0)
{
  char buf[64];
  time_t now=val?val:time(0);
  tm t;
  localtime_r(&now,&t);
  sprintf(buf,"%04d%02d%02d%02d%02d%02d",t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
  return buf;
}


void LocalFileStore::Init(smsc::util::config::Manager* cfgman)
{
  Smsc* smsc=&Smsc::getInstance();
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

  primaryFileName=cfgman->getString("MessageStore.LocalStore.filename");

  typedef vector<LoadFileInfo> LoadFileVector;
  LoadFileVector files;

  typedef buf::HashTree<SMSId,Scheduler::StoreData*> StoreHT;
  StoreHT ht;

  bool delayInit=sched.delayInit;

  if(delayInit)
  {
    sched.mon.Unlock();
  }
  uint32_t fileVer;

  uint64_t totalCount=0;

  try{
    string bakFn;

    if(File::Exists(primaryFileName))
    {
      string ts=mkTimeStamp();
      bakFn=mkTSFileName(ts);
      files.push_back(LoadFileInfo(bakFn,ts));
      File::Rename(primaryFileName,bakFn);
    }

    InitPrimaryFile();
    if(sched.delayInit)sched.delayInit=false;


    string storeDir=primaryFileName;
    string storeFileName;
    string::size_type slashPos=primaryFileName.rfind('/');
    if(slashPos!=string::npos)
    {
      storeFileName=storeDir.substr(slashPos+1);
      string::size_type dotPos=storeFileName.find('.');
      if(dotPos!=string::npos)
      {
        storeFileName.erase(dotPos);
      }
      storeDir.erase(slashPos+1);
    }else
    {
      storeDir="./";
    }
    vector<string> fileNames;
    File::ReadDir(storeDir.c_str(),fileNames,File::rdfFilesOnly);
    for(size_t i=0;i<fileNames.size();i++)
    {
      string& fn=fileNames[i];
      string ffn=storeDir+fn;
      if(ffn==primaryFileName || ffn==bakFn)
      {
        continue;
      }
      string::size_type dotPos=storeFileName.length();
      if(fn.compare(0,dotPos,storeFileName)!=0)
      {
        continue;
      }
      if(fn[dotPos]!='.')
      {
        smsc_log_warn(log,"file '%s' starts with the same prefix as store file",fn.c_str());
        continue;
      }
      string ts;
      string::size_type dotPos2=fn.find('.',dotPos+1);
      if(dotPos2!=string::npos)
      {
        ts=fn.substr(dotPos+1,dotPos2-dotPos-1);
      }else
      {
        ts=fn.substr(dotPos+1);
      }
      if(ts.length()!=14)
      {
        smsc_log_warn(log,"file '%s' contains invalid time stamp(%s)",fn.c_str(),ts.c_str());
        continue;
      }
      files.push_back(LoadFileInfo(ffn,ts));
    }

    std::sort(files.begin(),files.end());


    char sigBuf[sizeof(storeSig)];
    BufOps::SmsBuffer smsBuf(0);
    uint32_t sz1,sz2;
    SMSId id;
    int seq;
    bool final;
    Scheduler::StoreData* itemPtr=0;
    Scheduler::StoreFileData* store;
    for(LoadFileVector::iterator it=files.begin();it!=files.end();it++)
    {
      try{
        smsc_log_info(log,"Reading store file %s",it->fileName.c_str());
        store=new Scheduler::StoreFileData;
        store->timestamp=it->timeStamp;
        it->store=store;
        sched.storeFiles.push_back(store);
        File f;
        f.ROpen(it->fileName.c_str());
        File::offset_type fSize=f.Size();
        File::offset_type fPos=0;
        fPos+=f.Read(sigBuf,sizeof(storeSig)-1);
        sigBuf[sizeof(storeSig)-1]=0;
        if(strcmp(sigBuf,storeSig))
        {
          throw smsc::util::Exception("Storage file signature mismatch");
        }
        fileVer=f.ReadNetInt32();
        fPos+=4;
        if(fileVer>storeVer)
        {
          throw smsc::util::Exception("File version doesn't match current version:%d<%d",storeVer,fileVer);
        }
        while(fPos<fSize)
        {
          if(!running)
          {
            return;
          }
          sz1=f.ReadNetInt32();
          fPos+=4;
          if(fPos+sz1>fSize)
          {
            throw smsc::util::Exception("Incomplete record detected, fPos=%lld, fSize=%lld, recSize=%d",fPos,fSize,sz1);
          }
          if(sz1<=8+4+1)
          {
            throw smsc::util::Exception("Store broken at fPos=%lld, fSize=%lld, recSize=%d",fPos,fSize,sz1);
          }
          id=f.ReadNetInt64();
          fPos+=8;
          seq=f.ReadNetInt32();
          fPos+=4;
          final=f.ReadByte();
          fPos+=1;
          int bufSz=sz1-8-4-1;
          smsBuf.setSize(bufSz);
          f.Read(smsBuf.get(),bufSz);
          smsBuf.SetPos(bufSz);
          fPos+=bufSz;
          sz2=f.ReadNetInt32();
          fPos+=4;

          smsc_log_debug(log,"StoreInit: msgId=%lld, seq=%d, final=%s",id,seq,final?"true":"false");
          if(sz1!=sz2)
          {
            throw Exception("Corrupted store file %s, record size mismatch:%u!=%u",f.getFileName().c_str(),sz1,sz2);
          }

          if(ht.find(id,itemPtr))
          {
            if(itemPtr->final || itemPtr->seq>seq)
            {
              continue;
            }
            if(final)
            {
              totalCount--;
              itemPtr->Clear();
              itemPtr->final=true;
              itemPtr->storeFile->smsList.erase(itemPtr);
              itemPtr->storeFile=0;
              continue;
            }
            if(itemPtr->storeFile!=store)
            {
              itemPtr->storeFile->smsList.erase(itemPtr);
              store->smsList.push_back(itemPtr);
              itemPtr->storeFile=store;
            }
            itemPtr->seq=seq;
            itemPtr->CopyBuf(smsBuf);
          }else
          {
            if(final)
            {
              itemPtr=new Scheduler::StoreData(id,0,0,0,seq);
              itemPtr->final=true;
            }else
            {
              itemPtr=new Scheduler::StoreData(id,store,smsBuf,seq);
              store->smsList.push_back(itemPtr);
              totalCount++;
            }
            ht.insert(id,itemPtr);
          }
        };
        smsc_log_debug(log,"finished loading file=%s",f.getFileName().c_str());
      }catch(exception& e)
      {
        smsc_log_error(log,"Operative storage read failed %s:%s",it->fileName.c_str(),e.what());
        it->badFile=true;
      }
    }
  }catch(std::exception& e)
  {
    smsc_log_error(log,"Exception during storage init %s",e.what());
  }

  /* validate */

  if(delayInit)
  {
    sched.mon.Lock();
  }

  smsc_log_warn(log,"Local store loaded. %llu messages found.",totalCount);

  int cnt=0;
  SMS sms;
  int storeIdx=0;
  int killIdx=files.size()>=sched.getStoresCount()?(int)files.size()-(int)sched.getStoresCount()+1:-1;
  smsc_log_info(log,"files.count=%d, maxCount=%d, toKillIdx=%d",(int)files.size(),(int)sched.getStoresCount(),killIdx);
  for(LoadFileVector::iterator fit=files.begin(),fend=files.end();fit!=fend;++fit,++storeIdx)
  {
    MutexGuard mg(sched.storeMtx);
    smsc_log_info(log,"processing store file no %d - %s",storeIdx,fit->fileName.c_str());
    Scheduler::StoreFileData* store=fit->store;
    bool toKill=storeIdx<=killIdx;
    smsc_log_info(log,"Store file contains %d short messages.",(int)store->smsList.size());
    for(Scheduler::SMSList::iterator it=store->smsList.begin(),end=store->smsList.end();it!=end;)
    {
      if(!running)
      {
        return;
      }
      if(toKill)
      {
        try{
          Save(it->id,++it->seq,it->smsBuf,it->smsBufSize);
        }catch(std::exception& e)
        {
          smsc_log_error(log,"Failed to save sms with Id=%lld:%s",it->id,e.what());
          store->smsList.erase(it++);
          continue;
        }
      }
      BufOps::SmsBuffer buf(it->smsBuf,it->smsBufSize);

      try {
        Deserialize(buf,sms,fileVer);
      } catch (std::exception& e)
      {
        smsc_log_error(log,"Failed to deserialize sms with Id=%lld:%s",it->id,e.what());
        store->smsList.erase(it++);
        continue;
      }

      if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x02)//forward mode sms!
      {
        store->smsList.erase(it++);
        continue;
      }

      bool added=false;
      try{
        int smeIndex=smsc->getSmeIndex(sms.getDestinationSmeId());
        Scheduler::StoreData* sd=&*it;
        sched.store.Insert(sd->id,sd);
        ht.erase(sd->id);
        if(sms.getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG))
        {
          sd->rit=sched.replMap.insert(Scheduler::ReplaceIfPresentMap::value_type(sms,sd->id));
        }else
        {
          sd->rit=sched.replMap.end();
        }
        if(toKill)
        {
          store->smsList.erase(it--);
          sd->storeFile=sched.curFile;
          sched.curFile->smsList.push_back(sd);
        }
        added=true;

        if(delayInit)
        {
          ++it;
          sched.mon.Unlock();
          sched.storeMtx.Unlock();
        }
        try{
          if(sms.lastResult==0)
          {
            sched.AddScheduledSms(sd->id,sms,smeIndex);
          }else
          {
            sched.RescheduleSms(sd->id,sms,smeIndex);
          }
        }catch(std::exception& e)
        {
          smsc_log_error(log,"Exception in AddScheduledSms:'%s'",e.what());
        }

        cnt++;
        if((cnt%200)==0)
        {
          millisleep(10);
        }

        if(delayInit)
        {
          sched.storeMtx.Lock();
          sched.mon.Lock();
        }

      }catch(std::exception& e)
      {
        smsc_log_error(log,"Sms init error id=%lld:%s",it->id,e.what());
        if(!added)
        {
          store->smsList.erase(it++);
        }
      }
    }
    if(toKill || store->smsList.empty())
    {
      try{
        if(!fit->badFile)
        {
          smsc_log_info(log,"Removing file file %s",fit->fileName.c_str());
          File::Unlink(fit->fileName);
        }else
        {
          smsc_log_info(log,"Renaming bad file file %s",fit->fileName.c_str());
          File::Rename(fit->fileName,"bad-"+fit->fileName);
        }
      }catch(exception& e)
      {
        smsc_log_error(log,"Failed to rename/remove file '%s':%s",fit->fileName.c_str(),e.what());
        fit->badFile=true;
      }
      sched.storeFiles.erase(store);
      delete store;
    }
  }

  smsc_log_info(log,"store init completed.");

  for(StoreHT::iterator it=ht.begin(),end=ht.end();it!=end;++it)
  {
    delete it->value();
  }


  loadup=false;
  running=true;
  lastRollTime=time(NULL);

  Start();
}

void LocalFileStore::StartRoll()
{
  MutexGuard mg2(mon);
  info1(sched.log,"Notify rolling");
  rolling=true;
  mon.notify();
}


void LocalFileStore::Save(smsc::sms::SMSId id,uint32_t seq,const char* smsBufPtr,int smsBufSize,bool final)
{
  using namespace smsc::sms;
  using namespace smsc::sms::BufOps;
  SmsBuffer smsbuf(4+8+4+1+smsBufSize+4);
  uint32_t sz=8+4+1+smsBufSize;
  uint8_t fin=final?1:0;

  smsbuf<<sz<<id<<seq<<fin;
  smsbuf.Append(smsBufPtr,smsBufSize);
  smsbuf<<sz;

  MutexGuard mg(mtx);
  hrtime_t writeStart=gethrtime();
  primaryFile.Write(smsbuf.get(),smsbuf.GetPos());
  hrtime_t writeEnd=gethrtime();
  int writeTime=(int)((writeEnd-writeStart)/1000000);
  if(writeTime>100)
  {
    info2(wrLog,"disk write takes %d msec!",writeTime);
  }
  fileSize+=smsbuf.GetPos();
  if(!loadup && !rolling && fileSize>maxStoreSize)
  {
    StartRoll();
  }
}


int LocalFileStore::Execute()
{
  MutexGuard mg(mon);
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("store.roll");
  std::vector<std::string> indexes;
  indexes.reserve(sched.getStoresCount());
  while(running)
  {
    if(!rolling)
    {
      mon.wait();
    }
    if(!running)
    {
      break;
    }
    if(!rolling)
    {
      continue;
    }

    // last roll check
    time_t now=time(0);

    while(running && now-lastRollTime<minRollTime)
    {
      int tosleep=1000*((int)(minRollTime-(now-lastRollTime)));
      smsc_log_info(log,"Rolling notify too early, sleeping %d msec",tosleep);
      mon.wait(tosleep);
      now=time(0);
    }

    if(!running)
    {
      break;
    }

    string timeStamp=mkTimeStamp(now);
    string rfn=mkTSFileName(timeStamp);
    {
      MutexGuard smg(mtx);
      sched.rollCurrent(timeStamp);
      primaryFile.Close();
      primaryFile.Rename(rfn);
      InitPrimaryFile();
    }


    info1(log,"Rolling started");
    mon.Unlock();
    bool ok=true;
    bool haveRollFile=false;
    std::string rollFileTS;
    try{
      long i=0;
      uint64_t blockStart=getmillis();
      //Target: 2Mb/sec
      //Estimation: 512b/sms
      //4000 sms/sec =~ 2Mb/sec
      //interval 10ms -> 40/10ms

      rollFileTS=sched.selectRollFile();
      if(!rollFileTS.empty())
      {
        haveRollFile=true;
        while(sched.rollStoreNext() && running)
        {
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
        info2(log,"Rolled %ld messages",i);
      }else
      {
        info1(log,"Rolling is not necessary");
      }
    }catch(exception& e)
    {
      warn2(log,"Exception during rolling:%s",e.what());
      ok=false;
    }
    if(!running)
    {
      break;
    }
    if(haveRollFile && !ok)
    {
      rfn=mkTSFileName(rollFileTS);
      try{
        File::Rename(rfn,"bad-"+rfn);
      }catch(std::exception& e)
      {
        smsc_log_error(log,"Failed to rename rol file:%s",e.what());
      }
      warn1(log,"Rolling finished with error");
    }
    sched.clearStores(indexes);
    for(size_t i=0;i<indexes.size();i++)
    {
      std::string& ts=indexes[i];
      if(haveRollFile && !ok && ts==rollFileTS)
      {
        continue;
      }
      rfn=mkTSFileName(ts);
      try{
        File::Unlink(rfn);
      }catch(std::exception& e)
      {
        smsc_log_error(log,"Failed to unlink file '%s':%s",rfn.c_str(),e.what());
      }
    }
    mon.Lock();
    lastRollTime=time(NULL);
    rolling=false;
  }
  return 0;
}

void Scheduler::Init(smsc::util::config::Manager* cfgman)
{
  smsc=&Smsc::getInstance();
  localFileStore.Init(cfgman);
  archiveStorage.init(cfgman->getString("MessageStore.archiveDir"),cfgman->getInt("MessageStore.archiveInterval"));

}


void Scheduler::DelayInit(smsc::util::config::Manager* cfgman)
{
  archiveStorage.init(cfgman->getString("MessageStore.archiveDir"),cfgman->getInt("MessageStore.archiveInterval"));
  smsc=&Smsc::getInstance();
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
      Init(&smsc::util::config::Manager::getInstance());
    }catch(std::exception& e)
    {
      smsc_log_error(log,"Exception during delayed Scheduler init:%s",e.what());
    }
    delayInit=false;
  }

  time_t t=time(NULL);
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
  archiveStorage.roll();
  archiveStorage.close();
  return 0;
}


SMSId Scheduler::getNextId()
{
  MutexGuard mg(idMtx);
  SMSId rv=idSeq+=16;
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
  StoreData* sd=0;
  bool ripFound=false;
  {
    MutexGuard mg(storeMtx);

    if(flag==smsc::store::SMPP_OVERWRITE_IF_PRESENT)
    {
      ReplaceIfPresentMap::iterator it=replMap.find(sms);
      if(it!=replMap.end())
      {
        sd=store.Get(it->second);
        ripFound=true;
        sd->CopyBuf(buf);
        updateInSnap(sd);
        sd->seq++;
        CancelSms(id,sms.getDealiasedDestinationAddress());
      }
    }
    if(!sd)
    {
      sd=newStoreData(id,curFile,buf);
      store.Insert(id,sd);
    }
    lg.Lock(sd);
    if(flag==smsc::store::SMPP_OVERWRITE_IF_PRESENT)
    {
      if(!ripFound)
      {
        sd->rit=replMap.insert(ReplaceIfPresentMap::value_type(sms,id));
      }
    }else
    {
      sd->rit=replMap.end();
    }
    if(!ripFound)
    {
      smsc_log_debug(log,"insert into snap");
      curFile->smsList.push_back(sd);
    }
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

static void addHistoryItem(SMS& sms,const char* item)
{
  if(!sms.hasStrProperty(Tag::SMSC_HISTORY))
  {
    sms.setStrProperty(Tag::SMSC_HISTORY,item);
  }else
  {
    char buf[129];
    int itemLength=(int)strlen(item);
    const SmsPropertyBuf& hst=sms.getStrProperty(Tag::SMSC_HISTORY);
    if(hst.length()+itemLength<128)
    {
      memcpy(buf,hst.c_str(),hst.length());
      memcpy(buf+hst.length(),item,itemLength);
      buf[hst.length()+itemLength]=0;
      sms.setStrProperty(Tag::SMSC_HISTORY,buf);
    }
  }
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
    updateInSnap(sd);
  }
  //SMS& sms=(*ptr)->sms;
  SMS sms;
  sd->LoadSms(sms);
  char buf[32];
  struct tm tm;
  time_t now=time(0);
  localtime_r(&now,&tm);
  sprintf(buf,"T%02d%02d%02d%02d:%d\n",tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,failureCause);
  addHistoryItem(sms,buf);

  sms.state = ENROUTE;
  sms.destinationDescriptor=dst;
  sms.lastTime=time(NULL);
  sms.setLastResult(failureCause);
  sms.setNextTime(nextTryTime);
  sms.setAttemptsCount(attempts);
  sd->seq++;
  sd->SaveSms(sms);
  LocalFileStoreSave(id,sd);
}

void Scheduler::changeSmsStateToDelivered(SMSId id,
    const Descriptor& dst)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToDelivered: msgId=%lld",id);
  doFinalizeSms(id,smsc::sms::DELIVERED,smsc::Status::OK,dst);
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
  doFinalizeSms(id,smsc::sms::EXPIRED,smsc::Status::EXPIRED);
}

void Scheduler::changeSmsStateToDeleted(SMSId id)
        throw(StorageException, NoSuchMessageException)
{
  debug2(log,"changeSmsStateToDeleted: msgId=%lld",id);
  doFinalizeSms(id,smsc::sms::DELETED,smsc::Status::DELETED);
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
    updateInSnap(sd);
  }
  SMS sms;
  sd->LoadSms(sms);
  sms.setConcatSeqNum(sms.getConcatSeqNum()+inc);
  char buf[32];
  struct tm tm;
  time_t now=time(0);
  localtime_r(&now,&tm);
  sprintf(buf,"P%02d%02d%02d%02d\n",tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
  addHistoryItem(sms,buf);

  sd->seq++;
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
    sd=*ptr;
    store.Delete(id);
    removeFromSnap(sd);
    if(sd->rit!=replMap.end())
    {
      replMap.erase(sd->rit);
    }
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
    updateInSnap(sd);
  }
  sd->SaveSms(sms);
  sd->seq++;
  LocalFileStoreSave(id,sd);
}

}//system
}//smsc
