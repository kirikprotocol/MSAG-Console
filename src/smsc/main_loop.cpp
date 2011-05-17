#ident "@(#)$Id$"

#include <memory>
#include <vector>
#include <exception>
#ifndef linux
#include <thread.h>
#endif

#include "util/udh.hpp"

#include "smsc.hpp"
#include "interconnect/ClusterInterconnect.hpp"

namespace smsc{

#define WAIT_DATA_TIMEOUT 20 /* ms */
using smsc::smeman::CommandId;
using smsc::smeman::SmscCommand;
using std::exception;
using smsc::sms::SMS;
using smsc::smeman::SmeIterator;
using std::auto_ptr;

#define __CMD__(x) smsc::smeman::x

bool Smsc::routeSms(SmeIndex srcSme,const Address& org,const Address& dst,smsc::router::RoutingResult& rr)
{
  bool rv=smsc::router::Router::getInstance()->lookup(srcSme,org,dst,rr);
  if(rv)
  {
    rr.destSmeIdx=rr.info.destSmeIndex;
    rr.destProxy=getSmeProxy(rr.destSmeIdx);
  }
  return rv;
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

    SmscCommand resp=SmscCommand::makeSubmitSmResp
        (
          "0",
          cmd->get_dialogId(),
          Status::THROTTLED,
          sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
        );

    resp->dstNodeIdx=cmd->dstNodeIdx;
    resp->sourceId=cmd->sourceId;
    src_proxy->putCommand(resp);
  }catch(...)
  {
    __warning__("Failed to send reject response to sme");
  }
}

void Smsc::enqueueEx(EventQueue::EnqueueVector& ev)
{
  eventqueue.enqueueEx(ev);
  for(EventQueue::EnqueueVector::iterator it=ev.begin(),end=ev.end();it!=end;++it)
  {
    if(it->second.IsOk())
    {
      if(it->second->cmdid==SUBMIT)
      {
        SmscCommand resp=SmscCommand::makeSubmitSmResp
        (
          "",
          it->second->dialogId,
          Status::MSGQFUL,
          it->second->get_sms()->getIntProperty(Tag::SMPP_DATA_SM)
        );
        it->second.getProxy()->putCommand(resp);
      }
    }
  }
  ev.clear();
}

struct SpeedTimer{
  hrtime_t startTime,endTime;
  bool enabled;
  void start()
  {
    if(!enabled)
    {
      return;
    }
    startTime=gethrtime();
  }
  void end()
  {
    if(!enabled)
    {
      return;
    }
    endTime=gethrtime();
  }
  uint64_t getTime()
  {
    return (endTime-startTime)/1000;
  }
};

void Smsc::mainLoop(int idx)
{
  typedef std::vector<SmscCommand> CmdVector;
  CmdVector frame;
  SmeIndex smscSmeIdx=smeman.lookup("smscsme");
  Event e;
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("mainloop");
  smsc::logger::Logger *speedLog= smsc::logger::Logger::getInstance("ml.speeds");
  SpeedTimer st;
  st.enabled=speedLog->isDebugEnabled();
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
    if(!enqueueVector.empty())
    {
      int sz=(int)enqueueVector.size();
      st.start();
      enqueueEx(enqueueVector);
      st.end();
      debug2(speedLog,"enqueue time=%lld, size=%d",st.getTime(),sz);
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
      st.start();
      smeman.getFrame(frame,WAIT_DATA_TIMEOUT,getSchedCounter()>=freeBandwidthScaled/2);
      st.end();
      if(frame.size()>0)debug2(speedLog,"getFrame time:%lld, size=%d",st.getTime(),frame.size());
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
        st.start();
        while ( tasks.getExpired(&task) )
        {
          SMSId id = task.messageId;
          debug2(log,"enqueue timeout Alert: dialogId=%d, proxyUniqueId=%d",
            task.sequenceNumber,task.proxy_id);
          //eventqueue.enqueue(id,SmscCommand::makeAlert(task.sms));
          generateAlert(id,task.sms,task.inDlgId,task.diverted);
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
        st.end();
        debug2(speedLog,"expiration processing time:%lld",st.getTime());
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
        debug2(log,"mainLoop: %s.priority=%d",i->getProxy()->getSystemId(),i->getProxy()->getPriority());
        int prio=i->getProxy()->getPriority()/1000;
        if(prio<0)prio=0;
        if(prio>=32)prio=31;
        (*i)->set_priority(prio);
        if((*i)->sourceId.empty())
        {
          (*i)->sourceId=i->getProxy()->getSystemId();
        }
      }catch(exception& e)
      {
        warn2(log,"Source proxy died after selection: %s",e.what());
        CmdVector::difference_type pos=std::distance(frame.begin(),i);
        frame.erase(i);
        if(frame.size()==0)break;
        i=frame.begin()+pos;
        i--;
        continue;
      }catch(...)
      {
        warn1(log,"Source proxy died after selection");
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
          st.start();
          processCommand((*i),enqueueVector,findTaskVector);
          st.end();
          debug2(speedLog,"command %d processing time:%lld",(*i)->get_commandId(),st.getTime());
        }catch(std::exception& e)
        {
          __warning2__("command processing failed(%d):%s",(*i)->get_commandId(),e.what());
          if((*i)->get_commandId()==SUBMIT)
          {
            SmscCommand& cmd=*i;
            SMS* sms=cmd->get_sms();
            SmeProxy* src_proxy=cmd.getProxy();
            SmscCommand resp=SmscCommand::makeSubmitSmResp
                (
                  "0",
                  cmd->get_dialogId(),
                  Status::SYSERR,
                  sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                );

            resp->dstNodeIdx=cmd->dstNodeIdx;
            resp->sourceId=cmd->sourceId;
            src_proxy->putCommand(resp);
          }
        }
      }
    }

    if(findTaskVector.size())
    {
      int sz=(int)findTaskVector.size();
      st.start();
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
      st.end();
      debug2(log,"findAndRemoveTaskTime time:%lld, size=%d",st.getTime(),sz);
    }

    if(submitCount==0)
    {
      continue; //start cycle from start
    }

    if(!enqueueVector.empty())
    {
      int sz=(int)enqueueVector.size();
      st.start();
      enqueueEx(enqueueVector);
      st.end();
      debug2(speedLog,"enqueue time=%lld, size=%d",st.getTime(),sz);
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


    int eqsize,equnsize;
    for(int j=0;j<frame.size();j++)
    {
      SmscCommand* i=&frame[shuffle[j]];
      eventqueue.getStats(eqsize,equnsize);
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
          st.start();
          processCommand((*i),enqueueVector,findTaskVector);
          incTotalCounter(perSlot,(*i)->get_commandId()==FORWARD,fperSlot);
          sbmcnt++;
          st.end();
          debug2(speedLog,"command %d processing time:%lld",(*i)->get_commandId(),st.getTime());
        }catch(...)
        {
          __warning2__("command processing failed:%d",(*i)->get_commandId());
        }
      }
      //__warning2__("count=%d, smooth_cnt=%d",cntInstant,cntSmooth);
    }
  } // end of main loop
  info1(log,"end of mainloop");
}


void Smsc::generateAlert(SMSId id,SMS* sms,int inDlgId,bool diverted)
{
  //eventqueue.enqueue(id,SmscCommand::makeAlert(sms,inDlgId));
  SmscCommand resp=SmscCommand::makeDeliverySmResp(0,0,MAKE_COMMAND_STATUS(CMD_ERR_TEMP,Status::DELIVERYTIMEDOUT));
  resp->get_resp()->set_inDlgId(inDlgId);
  resp->get_resp()->set_sms(sms);
  resp->get_resp()->set_diverted(diverted);
  eventqueue.enqueue(id,resp);
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
      bool isFromIcon=!strcmp(cmd.getProxy()->getSystemId(),"CLSTRICON");
      if(sms.getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG) && !isFromIcon)
      {
        uint64_t addr;
        sscanf(sms.getDestinationAddress().value,"%lld",&addr);
        int dstNode=(int)(addr%nodesCount);
        cmd->dstNodeIdx=dstNode;
        if(dstNode!=nodeIndex-1)
        {
          interconnect::ClusterInterconnect::getInstance()->putCommand(cmd);
          return;
        }
      }
      if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40) &&
          (!strcmp(cmd.getProxy()->getSystemId(),"MAP_PROXY") || isFromIcon))
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
          debug2(log,"sms from %s have concat info:mr=%u, %u/%u",sms.getOriginatingAddress().toString().c_str(),(unsigned)mr,(unsigned)idx,(unsigned)num);

          if(!isFromIcon)
          {
            uint64_t addr;
            sscanf(sms.getOriginatingAddress().value,"%lld",&addr);
            int dstNode=(int)(addr%nodesCount);
            cmd->dstNodeIdx=dstNode;
            if(dstNode!=nodeIndex)
            {
              smsc_log_debug(logML,"redirecting multipart submit to node %d",dstNode);
              interconnect::ClusterInterconnect::getInstance()->putCommand(cmd);
              return;
            }
          }

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
            sms.setIntProperty(Tag::SMSC_MERGE_CONCAT,1);
            id=store->getNextId();
            info2(log,"create mrcache item msgId=%lld;oa=%s;da=%s;mr=%d",id,sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),(int)mr);
            mergeCache.Insert(mci,id);
            reverseMergeCache.Insert(id,mci);
            std::pair<time_t,SMSId> to(time(NULL)+mergeConcatTimeout,id);
            mergeCacheTimeouts.Push(to);
          }else
          {
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
        debug2(log,"sms from %s have sar info:mr=%u, %u/%u",
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
          sms.setIntProperty(Tag::SMSC_MERGE_CONCAT,2);
          id=*pid;
          info2(log,"assign from mrcache msgId=%lld;oa=%s;da=%s;mr=%d",id,sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),(int)mr);
        }
      }else
      {
        if(sms.getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG))
        {
          id=scheduler->getReplaceIfPresentId(sms);
        }
        if(id==0)
        {
          id=store->getNextId();
        }
      }
      //registerMsuStatEvent(StatEvents::etSubmitOk,&sms);
      break;
    }
    case __CMD__(SUBMIT_RESP):
    {
      SmeProxy* dstPrx=getSmeProxy(cmd->sourceId);
      smsc_log_debug(logML,"redirect submit_resp to %s",cmd->sourceId.c_str());
      try{
        dstPrx->putCommand(cmd);
      }catch(std::exception& e)
      {
        smsc_log_warn(logML,"putCommand exception:%s",e.what());
      }
      return;
    }break;
    case __CMD__(DELIVERY_RESP):
    {
      //Task task;
      uint32_t dialogId = cmd->get_dialogId();

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
      //distlstsme->putCommand(cmd);
      cmd.getProxy()->putCommand(smsc::smeman::SmscCommand::makeSubmitMultiResp("",cmd->get_dialogId(),Status::INVCMDID));
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
  ev.push_back(EventQueue::EnqueueVector::value_type(id,cmd));
}


}
