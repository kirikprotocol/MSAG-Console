#include "dpfTracker.hpp"
#include "system/smsc.hpp"
#include "system/common/rescheduler.hpp"

namespace smsc{
namespace system{

static const char* sig="DPFSTORE";
static const char* jsig="DPFJOURNAL";
static uint32_t ver=0x0100;
static uint32_t jver=0x0100;

static void createJFile(buf::File& f,const std::string& fn)
{
  f.WOpen(fn.c_str());
  f.Write(jsig,strlen(jsig));
  f.WriteNetInt32(jver);
  f.Flush();
}

static void createDpfFile(buf::File& f,const std::string& fn)
{
  f.WOpen(fn.c_str());
  f.Write(sig,strlen(sig));
  f.WriteNetInt32(ver);
  f.Flush();
}

static void openDpfFile(buf::File& f,const std::string& fn)
{
  f.ROpen(fn.c_str());
  char fileSig[9]={0,};
  f.Read(fileSig,8);
  if(strcmp(fileSig,sig))
  {
    throw smsc::util::Exception("Invalid dpfstore file signature:'%s'",fileSig);
  }
  uint32_t fileVer=f.ReadNetInt32();
  if(fileVer>ver)
  {
    throw smsc::util::Exception("Unsupported dpfstore file version:%u",fileVer);
  }
}

static void openJFile(buf::File& f,const std::string& fn)
{
  f.ROpen(fn.c_str());
  char fileSig[11]={0,};
  f.Read(fileSig,10);
  if(strcmp(jsig,fileSig))
  {
    throw smsc::util::Exception("Invalid dpfjournal file signature:'%s'",fileSig);
  }
  uint32_t fileVer=f.ReadNetInt32();
  if(fileVer>ver)
  {
    throw smsc::util::Exception("Unsupported dpfjournal file version:%u",fileVer);
  }
}

void DpfTracker::Init(const char *argStoreLocation, int to1179, int to1044,int mxch,int ctm)
{
  storeLocation=argStoreLocation;
  if(storeLocation.length()>0 && storeLocation[storeLocation.length()-1]!='/')
  {
    storeLocation+='/';
  }
  timeOut1179=to1179;
  timeOut1044=to1044;
  maxChangesCount=mxch;
  compactTime=ctm;
  log=smsc::logger::Logger::getInstance("dpf.track");

  std::string binFile=storeLocation+"dpfstore.bin";
  std::string jFile=storeLocation+"dpfjournal.bin";
  std::string newbinFile=storeLocation+"dpfstore.new";
  std::string joldFile=storeLocation+"dpfjournal.old";
  if(buf::File::Exists(newbinFile.c_str()))
  {
    buf::File::Unlink(newbinFile.c_str());
  }
  if(buf::File::Exists(binFile.c_str()))
  {
    buf::File f;
    openDpfFile(f,binFile.c_str());
    buf::File::offset_type sz=f.Size();
    Record* rec;
    while(f.Pos()<sz)
    {
      rec=new Record();
      rec->Read(f);
      abonents.insert(rec);
      rec->expIter=expirations.insert(rec);
    }
  }
  if(buf::File::Exists(joldFile.c_str()))
  {
    ApplyChanges(joldFile);
  }
  if(buf::File::Exists(jFile.c_str()))
  {
    ApplyChanges(jFile);
    changesStore.Append(jFile.c_str());
  }else
  {
    createJFile(changesStore,jFile);
  }
  cthread.init(storeLocation);
  Start();
}

void DpfTracker::ApplyChanges(const std::string &fileName)
{
  buf::File f;
  openJFile(f,fileName);
  buf::File::offset_type sz=f.Size();
  Change c(0);
  time_t now=time(NULL);
  int cnt=0;
  while(f.Pos()<sz)
  {
    changesCount++;
    if((changesCount%50000)==0)
    {
      smsc_log_info(log,"reading changes:%d(%lld/%lld)",changesCount,f.Pos(),sz);
    }
    c.Read(f);
    if(!c.validate())
    {
      smsc_log_warn(log,"Invalid change record detected at %ld",f.Pos());
      continue;
    }
    if(c.ct==ctRegisterSme)
    {
      Record frec;
      frec.abonent=c.abonent;
      AbonentsSet::iterator it=abonents.find(&frec);
      ReqRecord req=c.asReqRecord();
      if(now-req.expiration<5*60)
      {
        req.expiration=now+5*60+cnt++;
      }
      if(it==abonents.end())
      {
        Record* rec=new Record;
        rec->abonent=c.abonent;
        rec->requests.insert(req);
        rec->expiration=req.expiration;
        abonents.insert(rec);
        rec->expIter=expirations.insert(rec);
      }else
      {
        Record *rec=*it;
        Record::ReqSet::iterator begin=rec->requests.begin();
        Record::ReqSet::iterator end=rec->requests.end();
        bool found=false;
        for(Record::ReqSet::iterator rit=begin;rit!=end;rit++)
        {
          if(rit->smeId==req.smeId)
          {
            smsc_log_debug(log,"replaced record for abonent=%lld, smsId=%s, expiration=%d",c.abonent,req.smeId.c_str(),req.expiration);
            expirations.erase(rec->expIter);
            rec->requests.erase(rit);
            rec->requests.insert(req);
            rec->expiration=req.expiration;
            rec->expIter=expirations.insert(rec);
            found=true;
            break;
          }
        }
        if(!found)
        {
          rec->requests.insert(req);
        }
      }
    }else if(c.ct==ctRemoveAbonent)
    {
      Record frec;
      frec.abonent=c.abonent;
      AbonentsSet::iterator it=abonents.find(&frec);
      if(it==abonents.end())
      {
        smsc_log_warn(log,"remove abonent %lld request found in journal, but abonent already removed",c.abonent);
      }else
      {
        Record* rec=*it;
        abonents.erase(it);
        expirations.erase(rec->expIter);
        delete rec;
      }
    }else if(c.ct==ctRemoveSme)
    {
      Record frec;
      frec.abonent=c.abonent;
      AbonentsSet::iterator it=abonents.find(&frec);
      if(it==abonents.end())
      {
        smsc_log_warn(log,"remove request of abonent %lld sme %s found in journal, but abonent already removed",c.abonent,c.smeId.c_str());
      }else
      {
        Record* rec=*it;
        Record::ReqSet::iterator end=rec->requests.end();
        for(Record::ReqSet::iterator rit=rec->requests.begin();rit!=end;rit++)
        {
          if(rit->smeId==c.smeId)
          {
            rec->requests.erase(rit);
            break;
          }
        }
        if(rec->requests.empty())
        {
          abonents.erase(it);
          expirations.erase(rec->expIter);
          delete rec;
        }else
        {
          expirations.erase(rec->expIter);
          rec->expiration=rec->requests.begin()->expiration;
          rec->expIter=expirations.insert(rec);
        }
      }
    }
  }
}


bool DpfTracker::registerSetDpf(const smsc::sms::Address &abonent,const smsc::sms::Address &smeAddr, int errCode,time_t validTime,const char* smeId,int attempt)
{
  smsc_log_info(log,"register abonent=%s, errCode=%d, sme=%s, valid=%d",abonent.toString().c_str(),errCode,smeId,validTime);
  sync::MutexGuard mg(mon);
  time_t expValue;
  if(validTime==0)
  {
    expValue=time(NULL)+(errCode==1179?timeOut1179:timeOut1044);
  }else
  {
    expValue=validTime;
    if(expValue-time(NULL)>(errCode==1179?timeOut1179:timeOut1044))
    {
      smsc_log_info(log,"registration denied for, too long valid time");
      return false;
    }
  }
  uint64_t abnId;
  sscanf(abonent.value,"%lld",&abnId);
  Record frec;
  frec.abonent=abnId;
  AbonentsSet::iterator it=abonents.find(&frec);
  Record* rec;
  ReqRecord req;
  req.expiration=expValue;
  req.smeId=smeId;
  req.addr=smeAddr;
  req.attempt=attempt;
  SaveChange(Change(abnId,req.expiration,smeId,smeAddr));
  if(it==abonents.end())
  {
    smsc_log_debug(log,"new record for abonent=%lld, expiration=%d",abnId,req.expiration);
    rec=new Record;
    rec->abonent=abnId;
    rec->requests.insert(req);
    abonents.insert(rec);
    rec->expiration=req.expiration;
    rec->expIter=expirations.insert(rec);
  }else
  {
    rec=*it;
    Record::ReqSet::iterator begin=rec->requests.begin();
    Record::ReqSet::iterator end=rec->requests.end();
    bool found=false;
    for(Record::ReqSet::iterator rit=begin;rit!=end;rit++)
    {
      if(rit->smeId==smeId)
      {
        smsc_log_debug(log,"replaced record for abonent=%lld, smsId=%s, expiration=%d",abnId,smeId,req.expiration);
        expirations.erase(rec->expIter);
        rec->requests.erase(rit);
        rec->requests.insert(req);
        rec->expiration=req.expiration;
        rec->expIter=expirations.insert(rec);
        found=true;
        break;
      }
    }
    if(!found)
    {
      rec->requests.insert(req);
    }
  }
  mon.notify();
  return true;
}

void DpfTracker::hlrAlert(const smsc::sms::Address &abonent)
{
  try
  {
    sync::MutexGuard mg(mon);
    uint64_t abnId;
    sscanf(abonent.value,"%lld",&abnId);
    Record frec;
    frec.abonent=abnId;
    AbonentsSet::iterator it=abonents.find(&frec);
    if(it==abonents.end())
    {
      smsc_log_debug(log,"record for abonent=%lld not found",abnId);
      return;
    }
    smsc_log_info(log,"hlralert for abonent=%s",abonent.toString().c_str());
    SaveChange(Change(abnId));
    Record* rec=*it;
    Record::ReqSet::iterator end=rec->requests.end();
    for(Record::ReqSet::iterator rit=rec->requests.begin();rit!=end;rit++)
    {
      smsc_log_debug(log,"sent alert notification for abonent=%lld smeId=%s",abnId,rit->smeId.c_str());
      sendAlertNotify(rec->abonent,rit->addr,rit->smeId,0);
    }
    abonents.erase(it);
    expirations.erase(rec->expIter);
    delete rec;
    smsc_log_debug(log,"record for abonent=%lld deleted",abnId);
  } catch(std::exception& e)
  {
    smsc_log_warn(log,"Exception during hlrAlert for abonent %s",abonent.toString().c_str());
  }
}


int DpfTracker::Execute()
{
  uint64_t abonent;
  SystemIdStr smeId;
  smsc::sms::Address smeAddr;
  bool needToSendAlert=false;
  int attempt;
  while(!isStopping)
  {
    try
    {
      sync::MutexGuard mg(mon);
      if(expirations.empty())
      {
        time_t now=time(NULL);
        int toSleep=(int)(compactTime-(now-lastCompact))*1000;
        if(toSleep>0)mon.wait(toSleep);
        now=time(NULL);
        if(now-lastCompact>compactTime)
        {
          if(changesCount>0)
          {
            smsc_log_info(log,"start compacting by time");
            if(StartCompacting())
            {
              changesCount=0;
            }
          }
          lastCompact=now;
        }
        continue;
      }
      Record* rec=(*expirations.begin());
      time_t nextExp=rec->expiration;
      time_t now=time(NULL);
      if(nextExp>now)
      {
        int toWait=(int)(nextExp-now)*1000;
        int tillCompact=(int)(compactTime-(now-lastCompact))*1000;
        mon.wait(std::min(toWait,tillCompact));
        if(isStopping)
        {
          break;
        }
        now=time(NULL);
      }
      if(now-lastCompact>compactTime && changesCount>0)
      {
        smsc_log_info(log,"start compacting by time");
        if(StartCompacting())
        {
          changesCount=0;
        }
        lastCompact=now;
      }
      if(expirations.empty())
      {
        continue;
      }
      rec=(*expirations.begin());
      if(rec->expiration<=now)
      {
        expirations.erase(rec->expIter);
        Record::ReqSet::iterator it=rec->requests.begin();
        smeId=it->smeId;
        SaveChange(Change(rec->abonent,smeId));
        smsc_log_info(log,"request for abonent=%lld smeIdx=%s expired",rec->abonent,smeId.c_str());
        abonent=rec->abonent;
        smeAddr=it->addr;
        attempt=it->attempt;
        needToSendAlert=true;
        rec->requests.erase(it);
        if(rec->requests.empty())
        {
          abonents.erase(rec);
          delete rec;
        }else
        {
          it=rec->requests.begin();
          rec->expiration=it->expiration;
          rec->expIter=expirations.insert(rec);
        }
      }
    } catch(std::exception& e)
    {
      smsc_log_warn(log,"Exception in dpfTracker::Execute:'%s'",e.what());
    }
    if(needToSendAlert)
    {
      if(!sendAlertNotify(abonent,smeAddr,smeId,2))
      {
        char buf[64];
        sprintf(buf,"%lld",abonent);
        time_t expTime=RescheduleCalculator::calcNextTryTime(time(NULL),Status::DPFSMENOTCONNECTED,attempt);
        registerSetDpf(buf,smeAddr,1179,expTime,smeId.c_str(),attempt+1);
      }
      needToSendAlert=false;
    }
  }
  return 0;
}

bool DpfTracker::sendAlertNotify(uint64_t abonent, const smsc::sms::Address &smeAddr,const SystemIdStr& smeId, int status)
{

  try{
    Smsc& smsc=Smsc::getInstance();
    SmeProxy* proxy=smsc.getSmeProxy(smeId.c_str());
    smsc_log_debug(log,"Sending AlertNotification for abonent=%lld to sme='%s' status=%d",abonent,smeId.c_str(),status);
    if(proxy!=0)
    {
      char buf[32];
      sprintf(buf,"+%lld",abonent);
      proxy->putCommand(
        SmscCommand::makeAlertNotificationCommand
        (
          proxy->getNextSequenceNumber(),
          buf,
          smeAddr,
          status
        )
      );
      return true;
    }else
    {
      smsc_log_warn(log,"Sme %s requested dpf, but not connected at the moment",smeId.c_str());
    }
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"Failed to send AlertNotification to sme %s:'%s'",smeId.c_str(),e.what());
  }
  return false;
}

void DpfTracker::SaveChange(const Change &c)
{
  c.Write(changesStore);
  changesStore.Flush();
  changesCount++;
  if(changesCount>maxChangesCount)
  {
    if(StartCompacting())
    {
      smsc_log_info(log,"Start compacting by count");
      changesCount=0;
      lastCompact=time(NULL);
    }
  }
}

bool DpfTracker::StartCompacting()
{
  if(cthread.isCompacting())
  {
    return false;
  }
  std::string fn=changesStore.getFileName();
  changesStore.RenameExt("old");
  changesStore.Close();
  createJFile(changesStore,fn);
  cthread.startCompacting();
  return true;
}




int DpfTracker::CompactingThread::Execute()
{
  std::string binFile=storeLocation+"dpfstore.bin";
  std::string jFile=storeLocation+"dpfjournal.bin";
  std::string newbinFile=storeLocation+"dpfstore.new";
  std::string joldFile=storeLocation+"dpfjournal.old";
  smsc::logger::Logger* log=smsc::logger::Logger::getInstance("dpf.cmpct");
  smsc_log_info(log,"compactification thread started");
  while(!isStopping)
  {
    try
    {
      {
        MutexGuard mg(mon);
        while(!needCompacting && !isStopping)
        {
          mon.wait();
        }
        if(isStopping)
        {
          return 0;
        }
        if(!needCompacting)
        {
          continue;
        }
        compacting=true;
        needCompacting=false;
      }
      smsc_log_info(log,"starting compactification");
      buf::File f;
      RecSet rs;
      SmallRecord sr;
      if(buf::File::Exists(binFile.c_str()))
      {
        openDpfFile(f,binFile);
        buf::File::offset_type sz=f.Size();
        while(f.Pos()<sz)
        {
          sr.Read(f);
          rs.insert(sr);
        }
      }
      openJFile(f,joldFile);
      Change c(0);
      buf::File::offset_type sz=f.Size();
      while(f.Pos()<sz)
      {
        c.Read(f);
        if(c.ct==ctRegisterSme)
        {
          sr.abonent=c.abonent;
          RecSet::iterator it=rs.find(sr);
          ReqRecord req=c.asReqRecord();
          if(it==rs.end())
          {
            SmallRecord rec;
            rec.abonent=c.abonent;
            rec.requests.insert(req);
            rs.insert(rec);
          }else
          {
            it->requests.insert(req);
          }
        }else if(c.ct==ctRemoveAbonent)
        {
          sr.abonent=c.abonent;
          RecSet::iterator it=rs.find(sr);
          if(it!=rs.end())
          {
            rs.erase(it);
          }
        }else if(c.ct==ctRemoveSme)
        {
          sr.abonent=c.abonent;
          RecSet::iterator it=rs.find(sr);
          if(it!=rs.end())
          {
            ReqRecord rr;
            rr.smeId=c.smeId;
            SmallRecord::ReqSet::iterator rit=it->requests.find(rr);
            if(rit!=it->requests.end())
            {
              it->requests.erase(rit);
            }
            if(it->requests.empty())
            {
              rs.erase(it);
            }
          }
        }
      }
      createDpfFile(f,newbinFile);
      RecSet::iterator end=rs.end();
      for(RecSet::iterator it=rs.begin();it!=end;it++)
      {
        it->Write(f);
      }
      f.Flush();
      f.RenameExt("bin");
      buf::File::Unlink(joldFile.c_str());
      smsc_log_info(log,"finished compactification");
      compacting=false;
    } catch(std::exception& e)
    {
      smsc_log_warn(log,"exception during compactification:'%s'",e.what());
      compacting=false;
    }
  }
  smsc_log_info(log,"compactification thread finished");
  return 0;
}

void DpfTracker::CompactingThread::init(const std::string &argStoreLocation)
{
  storeLocation=argStoreLocation;
  if(buf::File::Exists((storeLocation+"dpfjournal.old").c_str()))
  {
    needCompacting=true;
  }
  Start();
}


void DpfTracker::CompactingThread::startCompacting()
{
  sync::MutexGuard mg(mon);
  if(compacting)
  {
    return;
  }
  needCompacting=true;
  mon.notify();
}


void DpfTracker::CompactingThread::stop()
{
  sync::MutexGuard mg(mon);
  isStopping=true;
  mon.notify();
}

}
}

