#ident "@(#)$Id$"

#include "smsc.hpp"
#include <memory>
#include <vector>
#ifndef linux
#include <thread.h>
#endif

#include <exception>

#include "util/udh.hpp"


namespace smsc{
namespace system{

#define WAIT_DATA_TIMEOUT 10 /* ms */
using smsc::smeman::CommandId;
using smsc::smeman::SmscCommand;
using std::exception;
using smsc::sms::SMS;
using smsc::smeman::SmeIterator;
using std::auto_ptr;

#define __CMD__(x) smsc::smeman::x

bool Smsc::routeSms(const Address& org,const Address& dst, int& dest_idx,SmeProxy*& proxy,
  smsc::router::RouteInfo* ri,SmeIndex idx)
{
  proxy = 0;
  bool ok ;
  if(idx==-1)
  {
    ok=getRouterInstance()->lookup(org,dst,proxy,&dest_idx,ri);
  }else
  {
    ok=getRouterInstance()->lookup(idx,org,dst,proxy,&dest_idx,ri);
  }
  return ok;
}

bool isUSSDSessionSms(SMS* sms)
{
  return sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) &&
         sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP)!=USSD_PSSR_IND &&
         !(
            sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP)==USSD_USSR_REQ &&
            !sms->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
          )
  ;
}

void Smsc::RejectSms(const SmscCommand& cmd,bool isLicenseLimit)
{
  SmeProxy* src_proxy=cmd.getProxy();
  try{
    SMS* sms=cmd->get_sms();
    sms->setSourceSmeId(src_proxy->getSystemId());
    sms->setLastResult(isLicenseLimit?Status::LICENSELIMITREJECT:Status::MSGQFUL);
    registerStatisticalEvent(StatEvents::etSubmitErr,sms);

    src_proxy->putCommand
    (
      SmscCommand::makeSubmitSmResp
      (
        "0",
        cmd->get_dialogId(),
        Status::THROTTLED,
        sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
      )
    );
  }catch(...)
  {
    __warning__("Failed to send reject response to sme");
  }
}

void Smsc::mainLoop(int idx)
{
  typedef std::vector<SmscCommand> CmdVector;
  CmdVector frame;
  SmeIndex smscSmeIdx=smeman.lookup("smscsme");
  Event e;
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("smsc.mainLoop");
#ifndef linux
  thr_setprio(thr_self(),127);
#endif
  time_t last_tm = time(NULL);
  time_t now = 0;

  EventQueue::EnqueueVector enqueueVector;
  FindTaskVector findTaskVector;
  typedef std::vector<int> IntVector;
  IntVector shuffle;

  smsc::logger::Logger *tslog = smsc::logger::Logger::getInstance("timestat");

  int sbmcnt=0;

  time_t lastTimeStatCheck=last_tm;

  int licenseExpTrapDay=-1;

  int licenseFileCheckHour;
  time_t lastLicenseCheckTime=0;
  {
    struct tm t;
    localtime_r(&last_tm,&t);
    licenseFileCheckHour=t.tm_hour;
  }

  while(!stopFlag)
  {
    if(enqueueVector.size())
    {
      hrtime_t eqStart=gethrtime();
      eventqueue.enqueueEx(enqueueVector);
      int sz=(int)enqueueVector.size();
      enqueueVector.clear();
      hrtime_t eqEnd=gethrtime();
      debug2(log,"eventQueue.enqueue time=%lld, size=%d",eqEnd-eqStart,sz);
    }

    int maxScaled=smsWeight*maxSmsPerSecond*shapeTimeFrame;
    //maxScaled+=maxScaled/4;
    int totalCnt=getTotalCounter();
    int schedCnt=getSchedCounter();
    int freeBandwidthScaled=maxScaled-(totalCnt-schedCnt);

    debug2(log,"totalCounter=%d, freeBandwidth=%d, schedCounter=%d, schedHasInput=%s",totalCnt,freeBandwidthScaled,schedCnt,scheduler->hasInput()?"Y":"N");

    int perSlot=smsWeight*maxSmsPerSecond/(1000/getTotalCounterRes());
    int fperSlot=(freeBandwidthScaled/shapeTimeFrame)/(2*1000/getTotalCounterRes());

    //perSlot+=perSlot/4;



    do
    {
      hrtime_t gfStart=gethrtime();
      smeman.getFrame(frame,WAIT_DATA_TIMEOUT,getSchedCounter()>=freeBandwidthScaled/2);
      hrtime_t gfEnd=gethrtime();
      if(frame.size()>0)debug2(log,"getFrame time:%lld, size=%d",gfEnd-gfStart,frame.size());
      now = time(NULL);

      if(idx==0)
      {
        if(now/60>=nextSpeedLogFlush)
        {
          info2(tslog,"maxTotalCounter=%d, maxStatCounter=%d",maxTotalCounter,maxStatCounter);
          maxTotalCounter=0;
          maxStatCounter=0;
          nextSpeedLogFlush+=speedLogFlushPeriod;
        }
      }


      if ( stopFlag ) return;
      Task task;
      if( now > last_tm )
      {
        hrtime_t expStart=gethrtime();
        while ( tasks.getExpired(&task) )
        {
          SMSId id = task.messageId;
          __trace2__("enqueue timeout Alert: dialogId=%d, proxyUniqueId=%d",
            task.sequenceNumber,task.proxy_id);
          //eventqueue.enqueue(id,SmscCommand::makeAlert(task.sms));
          generateAlert(id,task.sms,task.inDlgId);
        }
        {
          MutexGuard mg(mergeCacheMtx);
          while(mergeCacheTimeouts.Count()>0 && mergeCacheTimeouts.Front().first<=now)
          {
            SMSId id=mergeCacheTimeouts.Front().second;
            MergeCacheItem* pmci=reverseMergeCache.GetPtr(id);
            if(pmci)
            {
              debug2(log,"merging expired for msgId=%lld;oa=%s;da=%s;mr=%d",id,pmci->oa.toString().c_str(),pmci->da.toString().c_str(),(int)pmci->mr);
              mergeCache.Delete(*pmci);
              reverseMergeCache.Delete(id);
              eventqueue.enqueue(id,SmscCommand::makeCancel(id));
            }
            mergeCacheTimeouts.Pop();
          }
        }
        last_tm = now;
        hrtime_t expEnd=gethrtime();
        debug2(log,"expiration processing time:%lld",expEnd-expStart);
      }
    }while(!frame.size());


    ////
    //
    // here will be event queue limit check!
    //

    //
    //
    //
    //////

//    time_t now=time(NULL);

    if(license.expdate<now)
    {
      stopFlag=true;
      break;
    }

    if(idx==0 && now-lastLicenseCheckTime>10*60)
    {
      struct tm t;
      localtime_r(&now,&t);
      if(t.tm_hour!=licenseFileCheckHour)
      {
        time_t oldLicenseExp=license.expdate;
        bool licenseExpireSoon=license.expdate-now<20*24*60*60;
        InitLicense();
        maxSmsPerSecond=license.maxsms;
        licenseFileCheckHour=t.tm_hour;
        if(oldLicenseExp!=license.expdate && licenseExpireSoon && license.expdate-now>20*24*60*60)
        {
#ifdef SNMP
          char msg[256];
          struct tm lexp;
          localtime_r(&oldLicenseExp,&lexp);
          sprintf(msg,"CLEARED SYSTEM LICENSE will expire on %02d.%02d.%04d (AlarmID=LICENSE; severity=1)",lexp.tm_mday,lexp.tm_mon+1,lexp.tm_year+1900);
          smsc::snmp::SnmpAgent::trap("LICENSE","SYSTEM",smsc::snmp::SnmpAgent::NORMAL,msg);
#endif
        }
      }
      if(t.tm_mday!=licenseExpTrapDay)
      {
        if(license.expdate-now<20*24*60*60)
        {
#ifdef SNMP
          char msg[256];
          struct tm lexp;
          localtime_r(&license.expdate,&lexp);
          sprintf(msg,"ACTIVE SYSTEM LICENSE will expire on %02d.%02d.%04d (AlarmID=LICENSE; severity=5)",lexp.tm_mday,lexp.tm_mon+1,lexp.tm_year+1900);
          smsc::snmp::SnmpAgent::trap("LICENSE","SYSTEM",smsc::snmp::SnmpAgent::CRITICAL,msg);
#endif
        }
        licenseExpTrapDay=t.tm_mday;
      }
      lastLicenseCheckTime=now;
    }

    int submitCount=0;
    for(CmdVector::iterator i=frame.begin();i!=frame.end();i++)
    {
      try{
        __trace2__("mainLoop: %s.priority=%d",i->getProxy()->getSystemId(),i->getProxy()->getPriority());
        int prio=i->getProxy()->getPriority()/1000;
        if(prio<0)prio=0;
        if(prio>=32)prio=31;
        (*i)->set_priority(prio);
        (*i)->sourceId=i->getProxy()->getSystemId();
      }catch(exception& e)
      {
        __warning2__("Source proxy died after selection: %s",e.what());
        CmdVector::difference_type pos=std::distance(frame.begin(),i);
        frame.erase(i);
        if(frame.size()==0)break;
        i=frame.begin()+pos;
        i--;
        continue;
      }catch(...)
      {
        __warning__("Source proxy died after selection");
        CmdVector::difference_type pos=std::distance(frame.begin(),i);
        frame.erase(i);
        if(frame.size()==0)break;
        i=frame.begin()+pos;
        i--;
        continue;
      }
      if((*i)->get_commandId()==SUBMIT || (*i)->get_commandId()==FORWARD)
      {
        submitCount++;
      }else
      {
        try{
          hrtime_t cmdStart=gethrtime();
          processCommand((*i),enqueueVector,findTaskVector);
          hrtime_t cmdEnd=gethrtime();
          debug2(log,"command %d processing time:%lld",(*i)->get_commandId(),cmdEnd-cmdStart);
        }catch(...)
        {
          __warning2__("command processing failed:%d",(*i)->get_commandId());
        }
      }
    }

    if(findTaskVector.size())
    {
      int sz=(int)findTaskVector.size();
      hrtime_t frStart=gethrtime();
      tasks.findAndRemoveTaskEx(findTaskVector);
      SMSId id;
      for(FindTaskVector::iterator it=findTaskVector.begin();it!=findTaskVector.end();it++)
      {
        if(it->found)
        {
          enqueueVector.push_back(EventQueue::EnqueueVector::value_type(it->id,it->cmd));
        }
      }
      findTaskVector.clear();
      hrtime_t frEnd=gethrtime();
      debug2(log,"findAndRemoveTaskTime time:%lld, size=%d",frEnd-frStart,sz);
    }

    if(submitCount==0)
    {
      continue; //start cycle from start
    }

    if(enqueueVector.size())
    {
      hrtime_t eqStart=gethrtime();
      eventqueue.enqueueEx(enqueueVector);
      int sz=(int)enqueueVector.size();
      enqueueVector.clear();
      hrtime_t eqEnd=gethrtime();
      debug2(log,"eventQueue.enqueue time=%lld, size=%d",eqEnd-eqStart,sz);
    }

    shuffle.clear();
    {
      for(int i=0;i<frame.size();i++)
      {
        int x=((double)rand()/RAND_MAX)*i;
        if(x<shuffle.size())
        {
          shuffle.push_back(shuffle[x]);
          shuffle[x]=i;
        }else
        {
          shuffle.push_back(i);
        }
      }
    }


    // main "delay/reject" cycle

    int eqsize,equnsize;
    //for(CmdVector::iterator i=frame.begin();i!=frame.end();i++)
    for(int j=0;j<frame.size();j++)
    {
      SmscCommand* i=&frame[shuffle[j]];
      eventqueue.getStats(eqsize,equnsize);
      /*
      while(equnsize+1>eventQueueLimit)
      {
        hrtime_t nslStart=gethrtime();
        {
          Task task;
          while ( tasks.getExpired(&task) )
          {
            SMSId id = task.messageId;
            __trace2__("enqueue timeout Alert: dialogId=%d, proxyUniqueId=%d",
              task.sequenceNumber,task.proxy_id);
            generateAlert(id,task.sms,task.inDlgId);
            //eventqueue.enqueue(id,SmscCommand::makeAlert(task.sms));
          }
        }
        timestruc_t tv={0,1000000};
        nanosleep(&tv,0);
        eventqueue.getStats(eqsize,equnsize);
        hrtime_t nslEnd=gethrtime();
        debug2(log,"eqlimit(%d/%d) nanosleep block time:%lld",equnsize+1,eventQueueLimit,nslEnd-nslStart);
      }
       */
      if((*i)->get_commandId()==SUBMIT || (*i)->get_commandId()==FORWARD)
      {
        incStatCounter();
        try{
          if((*i)->get_commandId()==FORWARD || !isUSSDSessionSms((*i)->get_sms()))
          {
            totalCnt=getTotalCounter();
            if(totalCnt>maxScaled || equnsize+1>eventQueueLimit)
            {
              if((*i)->get_commandId()==SUBMIT)
              {
                info2(log,"Sms %s->%s sbm rejected by license/eq limit: %d/%d (%d), %d/%d",
                  (*i)->get_sms()->getOriginatingAddress().toString().c_str(),
                  (*i)->get_sms()->getDestinationAddress().toString().c_str(),
                  totalCnt,maxScaled,perSlot,equnsize,eventQueueLimit);
                RejectSms(*i,totalCnt>maxScaled);
              }else
              {
                info2(log,"Sms id=%lld fwd rejected by license/eq limit: %d/%d (%d), %d/%d",(*i)->get_forwardMsgId(),
                  totalCnt,maxScaled,perSlot,equnsize,eventQueueLimit);
                scheduler->RejectForward((*i)->get_forwardMsgId());
              }
              continue;
            }
          }
          hrtime_t cmdStart=gethrtime();
          processCommand((*i),enqueueVector,findTaskVector);
          incTotalCounter(perSlot,(*i)->get_commandId()==FORWARD,fperSlot);
          sbmcnt++;
          hrtime_t cmdEnd=gethrtime();
          debug2(log,"command %d processing time:%lld",(*i)->get_commandId(),cmdEnd-cmdStart);
        }catch(...)
        {
          __warning2__("command processing failed:%d",(*i)->get_commandId());
        }
      }
      //__warning2__("count=%d, smooth_cnt=%d",cntInstant,cntSmooth);
    }
  } // end of main loop
  __warning__("end of mainloop");
}


void Smsc::generateAlert(SMSId id,SMS* sms,int inDlgId)
{
  eventqueue.enqueue(id,SmscCommand::makeAlert(sms,inDlgId));
}


void Smsc::processCommand(SmscCommand& cmd,EventQueue::EnqueueVector& ev,FindTaskVector& ftv)
{
  SMSId id=0;
  static smsc::logger::Logger *log = smsc::logger::Logger::getInstance("sms.trace");
  static smsc::logger::Logger *logML = smsc::logger::Logger::getInstance("smsc.mainLoop");
  switch(cmd->get_commandId())
  {
    case __CMD__(SUBMIT):
    {
      SMS &sms=*cmd->get_sms();
      if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40) && !strcmp(cmd.getProxy()->getSystemId(),"MAP_PROXY"))
      {
        unsigned int len;
        unsigned char *body;
        if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
        {
          body=(unsigned char*)sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
        }else
        {
          body=(unsigned char*)sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
        }
        uint16_t mr;
        uint8_t idx,num;
        bool havemoreudh;
        bool haveconcat=smsc::util::findConcatInfo(body,mr,idx,num,havemoreudh);
        if(haveconcat && num>1)
        {
          __trace2__("sms from %s have concat info:mr=%u, %u/%u",sms.getOriginatingAddress().toString().c_str(),(unsigned)mr,(unsigned)idx,(unsigned)num);
          MergeCacheItem mci;
          mci.mr=mr;
          mci.oa=sms.getOriginatingAddress();
          mci.da=sms.getDestinationAddress();
          sms.setConcatMsgRef(mr);
          sms.setConcatSeqNum(0);
          MutexGuard mg(mergeCacheMtx);
          SMSId *pid=mergeCache.GetPtr(mci);
          if(!pid)
          {
            __trace__("first piece");
            sms.setIntProperty(Tag::SMSC_MERGE_CONCAT,1);
            id=store->getNextId();
            info2(log,"create mrcache item msgId=%lld;oa=%s;da=%s;mr=%d",id,sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),(int)mr);
            mergeCache.Insert(mci,id);
            reverseMergeCache.Insert(id,mci);
            std::pair<time_t,SMSId> to(time(NULL)+mergeConcatTimeout,id);
            mergeCacheTimeouts.Push(to);
          }else
          {
            __trace__("next piece");
            sms.setIntProperty(Tag::SMSC_MERGE_CONCAT,2);
            id=*pid;
            info2(log,"assign from mrcache msgId=%lld;oa=%s;da=%s;mr=%d",id,sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),(int)mr);
          }
        }else
        {
          id=store->getNextId();
        }
      }else
      if(
          sms.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM) &&
          sms.hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS) &&
          sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM) &&
          sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS)>1
        )
      {
        uint16_t mr=sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);
        uint8_t idx=sms.getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM),
                num=sms.getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS);
        __trace2__("sms from %s have sar info:mr=%u, %u/%u",
          sms.getOriginatingAddress().toString().c_str(),(unsigned)mr,(unsigned)idx,(unsigned)num);
        MergeCacheItem mci;
        mci.mr=mr;
        mci.oa=sms.getOriginatingAddress();
        mci.da=sms.getDestinationAddress();
        sms.setConcatMsgRef(mr);
        sms.setConcatSeqNum(0);
        MutexGuard mg(mergeCacheMtx);
        SMSId *pid=mergeCache.GetPtr(mci);
        if(!pid)
        {
          __trace__("first piece");
          sms.setIntProperty(Tag::SMSC_MERGE_CONCAT,1);
          id=store->getNextId();
          info2(log,"create mrcache item msgId=%lld;oa=%s;da=%s;mr=%d",id,sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),(int)mr);
          mergeCache.Insert(mci,id);
          reverseMergeCache.Insert(id,mci);
          std::pair<time_t,SMSId> to(time(NULL)+mergeConcatTimeout,id);
          mergeCacheTimeouts.Push(to);
          //registerMsuStatEvent(StatEvents::etSubmitOk,&sms);
          eventqueue.enqueue(id,cmd);
          return;
        }else
        {
          __trace__("next piece");
          sms.setIntProperty(Tag::SMSC_MERGE_CONCAT,2);
          id=*pid;
          info2(log,"assign from mrcache msgId=%lld;oa=%s;da=%s;mr=%d",id,sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),(int)mr);
        }
      }else
      {
        id=store->getNextId();
      }
      __trace2__("main loop submit: seq=%d, id=%lld",cmd->get_dialogId(),id);
      //registerMsuStatEvent(StatEvents::etSubmitOk,&sms);
      break;
    }
    case __CMD__(DELIVERY_RESP):
    {
      //Task task;
      uint32_t dialogId = cmd->get_dialogId();

      /*
      hrtime_t tcStart=gethrtime();
      if (!tasks.findAndRemoveTask(cmd.getProxy()->getUniqueId(),dialogId,&task))
      {
        __warning2__("task not found for delivery response. Sid=%s, did=%d",cmd.getProxy()->getSystemId(),dialogId);
        return; //jump to begin of for
      }
      hrtime_t tcEnd=gethrtime();
      info2(logML,"findAndRemove time=%lld",tcEnd-tcStart);
      __trace2__("delivery response received. seqnum=%d,msgId=%lld,sms=%p",dialogId,task.messageId,task.sms);
      cmd->get_resp()->set_sms(task.sms);
      cmd->get_resp()->set_diverted(task.diverted);
      cmd->set_priority(31);
      id=task.messageId;
      */
      ftv.push_back(FindTaskVector::value_type(cmd.getProxy()->getUniqueId(),dialogId,cmd));
      return;
    }
    case __CMD__(REPLACE):
    {
      int pos;
      if(sscanf(cmd->get_replaceSm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
         cmd->get_replaceSm().messageId.get()[pos]!=0)
      {
        cmd.getProxy()->putCommand
        (
          SmscCommand::makeReplaceSmResp
          (
            cmd->get_dialogId(),
            Status::INVMSGID
          )
        );
        return;
      };
      break;
    }
    case __CMD__(QUERY):
    {
      int pos;
      if(sscanf(cmd->get_querySm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
         cmd->get_querySm().messageId.get()[pos]!=0)
      {
        cmd.getProxy()->putCommand
        (
          SmscCommand::makeQuerySmResp
          (
            cmd->get_dialogId(),
            Status::INVMSGID,
            0,0,0,0
          )
        );
        return;
      };
      break;
    }
    case __CMD__(CANCEL):
    {
      if((cmd->get_cancelSm().messageId.get() && cmd->get_cancelSm().serviceType.get()))
      {
          cmd.getProxy()->putCommand
          (
            SmscCommand::makeCancelSmResp
            (
              cmd->get_dialogId(),
              Status::CANCELFAIL
            )
          );
          return;
      }

      if(cmd->get_cancelSm().messageId.get())
      {
        int pos=0;
        if(sscanf(cmd->get_cancelSm().messageId.get(),"%lld%n",&id,&pos)!=1 ||
           cmd->get_cancelSm().messageId.get()[pos]!=0)
        {
          cmd.getProxy()->putCommand
          (
            SmscCommand::makeCancelSmResp
            (
              cmd->get_dialogId(),
              Status::INVMSGID
            )
          );
          return;
        };
      }else
      {
        if(!cmd->get_cancelSm().sourceAddr.get() || !cmd->get_cancelSm().sourceAddr.get()[0] ||
           !cmd->get_cancelSm().destAddr.get() || !cmd->get_cancelSm().destAddr.get()[0])
        {
          cmd.getProxy()->putCommand
          (
            SmscCommand::makeCancelSmResp
            (
              cmd->get_dialogId(),
              Status::CANCELFAIL
            )
          );
          return;
        }
        cancelAgent->putCommand(cmd);
        return;
      }
      break;
    }
    case __CMD__(QUERYABONENTSTATUS):
    {
      cmd->set_dialogId(mapProxy->getNextSequenceNumber());
      mapProxy->putCommand(cmd);
      return;
    }
    case __CMD__(QUERYABONENTSTATUS_RESP):
    {
      abonentInfoProxy->putCommand(cmd);
      return;
    }
    case __CMD__(SUBMIT_MULTI_SM):
    {
      distlstsme->putCommand(cmd);
      return;
    }
    case __CMD__(FORWARD):
    {
      id=cmd->get_forwardMsgId();
      break;
    }
    case __CMD__(HLRALERT):
    case __CMD__(SMEALERT):
    {
      scheduler->putCommand(cmd);
      return;
    }
    case __CMD__(INSMSCHARGERESPONSE):
    {
      id=cmd->get_chargeSmsResp()->id;
      break;
    }
    case __CMD__(INFWDSMSCHARGERESPONSE):
    {
      id=cmd->get_fwdChargeSmsResp()->id;
      break;
    }
    case __CMD__(KILLMRCACHEITEM):
    {
      KillMrCacheItemCmd& ki=cmd->get_KillMrCacheItem();
      MergeCacheItem mci;
      mci.mr=ki.mr;
      mci.oa=ki.org;
      mci.da=ki.dst;

      MutexGuard mg(mergeCacheMtx);
      SMSId* pid=mergeCache.GetPtr(mci);
      if(pid)
      {
        info2(log,"msgId=%lld: kill mr cache item for %s,%s,%d",*pid,ki.org.toString().c_str(),ki.dst.toString().c_str(),(int)ki.mr);
        SMSId killId=*pid;
        reverseMergeCache.Delete(killId);
        mergeCache.Delete(mci);
      }
      return;
    };
    default:
    {
      __warning2__("mainLoop: unprocessed command id:%d",cmd->get_commandId());
    };
  }
  //hrtime_t eqStart=gethrtime();
  //eventqueue.enqueue(id,cmd);
  //hrtime_t eqEnd=gethrtime();
  //info2(logML,"eventQueue.enqueue time=%lld",eqEnd-eqStart);
  ev.push_back(EventQueue::EnqueueVector::value_type(id,cmd));
}


}
}
