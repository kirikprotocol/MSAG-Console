/*
  $Id$
*/

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

#define WAIT_DATA_TIMEOUT 100 /* ms */
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

void Smsc::mainLoop()
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
  for(;;)
  {
    do
    {
      smeman.getFrame(frame,WAIT_DATA_TIMEOUT);
      now = time(NULL);
      if ( stopFlag ) return;
      Task task;
      if( now > last_tm )
      {
        while ( tasks.getExpired(&task) )
        {
          SMSId id = task.messageId;
          __trace2__("enqueue timeout Alert: dialogId=%d, proxyUniqueId=%d",
            task.sequenceNumber,task.proxy_id);
          //eventqueue.enqueue(id,SmscCommand::makeAlert(task.sms));
          generateAlert(id,task.sms);
        }
        while(mergeCacheTimeouts.Count()>0 && mergeCacheTimeouts.Front().first<=now)
        {
          SMSId id=mergeCacheTimeouts.Front().second;
          MergeCacheItem* pmci=reverseMergeCache.GetPtr(id);
          if(pmci)
          {
            info2(log,"merging expired for msgId=%lld;oa=%s;da=%s;mr=%d",id,pmci->oa.toString().c_str(),pmci->da.toString().c_str(),(int)pmci->mr);
            mergeCache.Delete(*pmci);
            reverseMergeCache.Delete(id);
            eventqueue.enqueue(id,SmscCommand::makeCancel(id));
          }
          mergeCacheTimeouts.Pop();
        }
        last_tm = now;
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
          processCommand((*i));
        }catch(...)
        {
          __warning2__("command processing failed:%d",(*i)->get_commandId());
        }
      }
    }

    if(submitCount==0)
    {
      continue; //start cycle from start
    }

    int stf=tcontrol->getConfig().shapeTimeFrame;
    int smt=tcontrol->getConfig().smoothTimeFrame;
    int maxsms=tcontrol->getConfig().maxSmsPerSecond;

    // main "delay" cycle

    while(frame.size())
    {
      int cntInstant=tcontrol->getTotalCount();
//      int cntSmooth=tcontrol->getTotalCountLong();
      /*if(cntInstant+1>maxsms && cntSmooth+1000<=maxsms*smt*1000)
      {
        __info2__(log,"cnt=%d, smooth_cnt=%d, submitCnt=%d",cntInstant,cntSmooth,submitCount);
      }*/

      int eqsize,equnsize;
      eventqueue.getStats(eqsize,equnsize);
      /*
      if(
          (cntInstant+1<=maxsms*stf ||
            (
              cntSmooth+1000<=maxsms*smt*1000 &&
              cntInstant+1<=maxsms*stf+maxsms*stf*20/100
            )
          ) &&
          eqsize+1<=eventQueueLimit
        )
        */
      if(cntInstant+1<=maxsms*stf && equnsize+1<=eventQueueLimit)
      {
        SmscCommand cmd=frame.back();
        frame.pop_back();
        if(cmd->get_commandId()==SUBMIT || cmd->get_commandId()==FORWARD)
        {
          try{
            processCommand(cmd);
            tcontrol->incTotalCount(1);
          }catch(...)
          {
            __warning2__("command processing failed:%d",cmd->get_commandId());
          }
        }
        continue;
      }
      //__warning2__("count=%d, smooth_cnt=%d",cntInstant,cntSmooth);
      {
        Task task;
        while ( tasks.getExpired(&task) )
        {
          SMSId id = task.messageId;
          __trace2__("enqueue timeout Alert: dialogId=%d, proxyUniqueId=%d",
            task.sequenceNumber,task.proxy_id);
          generateAlert(id,task.sms);
          //eventqueue.enqueue(id,SmscCommand::makeAlert(task.sms));
        }
      }
      timestruc_t tv={0,1000000};
      nanosleep(&tv,0);
    }
  } // end of main loop
}


void Smsc::generateAlert(SMSId id,SMS* sms)
{
  eventqueue.enqueue(id,SmscCommand::makeAlert(sms));
}


void Smsc::processCommand(SmscCommand& cmd)
{
  SMSId id=0;
  static smsc::logger::Logger *log = smsc::logger::Logger::getInstance("sms.trace");
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
        if(haveconcat)
        {
          __trace2__("sms from %s have concat info:mr=%u, %u/%u",sms.getOriginatingAddress().toString().c_str(),(unsigned)mr,(unsigned)idx,(unsigned)num);
          MergeCacheItem mci;
          mci.mr=mr;
          mci.oa=sms.getOriginatingAddress();
          mci.da=sms.getDestinationAddress();
          sms.setConcatMsgRef(mr);
          sms.setConcatSeqNum(0);
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
          sms.hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM)
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
      __trace2__("main loop submit: seq=%d, id=%lld",cmd->get_dialogId(),id);
      break;
    }
    case __CMD__(DELIVERY_RESP):
    {
      Task task;
      uint32_t dialogId = cmd->get_dialogId();

      if (!tasks.findAndRemoveTask(cmd.getProxy()->getUniqueId(),dialogId,&task))
      {
        __warning2__("task not found for delivery response. Sid=%s, did=%d",cmd.getProxy()->getSystemId(),dialogId);
        return; //jump to begin of for
      }
      __trace2__("delivery response received. seqnum=%d,msgId=%lld,sms=%p",dialogId,task.messageId,task.sms);
      cmd->get_resp()->set_sms(task.sms);
      cmd->get_resp()->set_diverted(task.diverted);
      cmd->set_priority(31);
      id=task.messageId;
      break;
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
    case __CMD__(KILLMRCACHEITEM):
    {
      KillMrCacheItemCmd& ki=cmd->get_KillMrCacheItem();
      MergeCacheItem mci;
      mci.mr=ki.mr;
      mci.oa=ki.org;
      mci.da=ki.dst;

      SMSId* pid=mergeCache.GetPtr(mci);
      if(pid)
      {
        info2(log,"msgId=%lld: kill mr cache item for %s,%s,%d",*pid,ki.org.toString().c_str(),ki.dst.toString().c_str(),(int)ki.mr);
        SMSId id=*pid;
        reverseMergeCache.Delete(id);
        mergeCache.Delete(mci);
      }
      return;
    };
    default:
    {
      __warning2__("mainLoop: unprocessed command id:%d",cmd->get_commandId());
    };
  }
  eventqueue.enqueue(id,cmd);
}


}
}
