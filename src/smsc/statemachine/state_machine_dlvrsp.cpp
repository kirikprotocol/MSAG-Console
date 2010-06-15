#include <exception>

#include "logger/Logger.h"
#include "util/debug.h"
#include "util/udh.hpp"
#include "util/recoder/recode_dll.h"
#include "util/smstext.h"
#include "util/regexp/RegExp.hpp"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/FixedLengthString.hpp"
#include "inman/storage/cdrutil.hpp"
#include "smsc/closedgroups/ClosedGroupsInterface.hpp"
#include "smsc/status.h"
#include "smsc/resourcemanager/ResourceManager.hpp"
#include "smsc/smsc.hpp"
#include "smsc/common/rescheduler.hpp"
#include "smsc/profiler/profiler.hpp"
#include "smsc/common/TimeZoneMan.hpp"
#ifdef SMSEXTRA
#include "smsc/extra/Extra.hpp"
#include "smsc/extra/ExtraBits.hpp"
#endif

#include "constants.hpp"
#include "util.hpp"
#include "state_machine.hpp"


#ident "@(#)$Id$"


namespace smsc{
namespace statemachine{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace StateTypeValue;
using namespace smsc::smpp;
using namespace util;
using namespace smsc::snmp;
using namespace smsc::profiler;
using namespace smsc::common;
using namespace smsc::resourcemanager;
using std::exception;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using smsc::core::synchronization::Mutex;


StateType StateMachine::deliveryResp(Tuple& t)
{
  //__require__(t.state==DELIVERING_STATE);
  if(t.state!=DELIVERING_STATE)
  {
    debug2(smsLog, "DLVRSP: state of SMS isn't DELIVERING!!! Id=%lld;st=%d",t.msgId,t.command->get_resp()->get_status());
    smsc->getScheduler()->InvalidSms(t.msgId);
    return t.state;
  }

  {
    Descriptor d=t.command->get_resp()->getDescriptor();
    smsc_log_debug(smsLog,"resp dest descriptor:%s(%d)/%s(%d), Id=%lld",d.imsi,d.imsiLength,d.msc,d.mscLength,t.msgId);
  }

  SMS sms;
  bool dgortr=t.command->get_resp()->get_sms()!=0; //datagram or transaction
  bool finalized=false;
  if(dgortr)
  {
    sms=*t.command->get_resp()->get_sms();
    sms.destinationDescriptor=t.command->get_resp()->getDescriptor();
  }
  else
  {
    try{
      store->retriveSms((SMSId)t.msgId,sms);
    }catch(exception& e)
    {
      smsc_log_warn(smsLog, "DLVRSP: failed to retrieve sms:%s! Id=%lld;st=%d",e.what(),t.msgId,t.command->get_resp()->get_status());
      smsc->getScheduler()->InvalidSms(t.msgId);
      onDeliveryFail(t.msgId,sms);
      return UNKNOWN_STATE;
    }
    sms.destinationDescriptor=t.command->get_resp()->getDescriptor();
  }

  {
    Descriptor d=sms.destinationDescriptor;
    smsc_log_debug(smsLog,"sms dest descriptor:%s(%d)/%s(%d), Id=%lld",d.imsi,d.imsiLength,d.msc,d.mscLength,t.msgId);
  }

  {
    char buf[MAX_ADDRESS_VALUE_LENGTH*4+12]="";
    if(sms.originatingDescriptor.mscLength && sms.originatingDescriptor.imsiLength &&
       sms.destinationDescriptor.mscLength && sms.destinationDescriptor.imsiLength)
    {
      sprintf(buf,"Org: %s/%s, Dst:%s/%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi,
                         sms.destinationDescriptor.msc,sms.destinationDescriptor.imsi
                        );
    }else if(sms.originatingDescriptor.mscLength && sms.originatingDescriptor.imsiLength)
    {
      sprintf(buf,"Org: %s/%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi);
    }else if(sms.destinationDescriptor.mscLength && sms.destinationDescriptor.imsiLength)
    {
      sprintf(buf,"Dst:%s/%s",sms.destinationDescriptor.msc,sms.destinationDescriptor.imsi);
    }


    sms.setStrProperty(Tag::SMSC_DESCRIPTORS,buf);
  }

  bool firstPart=false;
  bool multiPart=sms.hasBinProperty(Tag::SMSC_CONCATINFO);

  if(GET_STATUS_TYPE(t.command->get_resp()->get_status())!=CMD_OK && !dgortr)
  {
    time_t now=time(NULL);
    bool softLimit=smsc->checkSchedulerSoftLimit();
    if((sms.getValidTime()<=now) || //expired or
       RescheduleCalculator::calcNextTryTime(now,sms.getLastResult(),sms.getAttemptsCount())==-1 || //max attempts count reached or
       softLimit  //soft limit reached
       )
    {
      sms.setLastResult(softLimit?Status::SCHEDULERLIMIT:Status::EXPIRED);
      onUndeliverable(t.msgId,sms);
      try{
        smsc->getScheduler()->InvalidSms(t.msgId);
        store->changeSmsStateToExpired(t.msgId);
      }catch(...)
      {
        smsc_log_warn(smsLog,"DLVRSP: failed to change state to expired");
      }
      info2(smsLog, "DLVRSP: Id=%lld;oa=%s;da=%s %s (valid:%u - now:%u), attempts=%d",t.msgId,
          sms.getOriginatingAddress().toString().c_str(),
          sms.getDestinationAddress().toString().c_str(),
          softLimit?"denied by soft sched limit":"expired",sms.getValidTime(),now,sms.getAttemptsCount());
      sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
      smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,true,Smsc::chargeOnDelivery);
      return EXPIRED_STATE;
    }
  }

  if(sms.billingRequired() && sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery)
  {
    int statusType=GET_STATUS_TYPE(t.command->get_resp()->get_status());
    bool final=
      statusType==CMD_OK ||
      statusType==CMD_ERR_PERM ||
      (
       statusType==CMD_ERR_TEMP &&
       (
        sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)||
        (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2
       )
      );
    if(multiPart)
    {
      unsigned len;
      if(sms.getConcatSeqNum()==0)
      {
        firstPart=true;
      }
    }
    smsc_log_debug(smsLog,"multiPart=%s, firstPart=%s, final=%s",multiPart?"true":"false",firstPart?"true":"false",final?"true":"false");
    if(!multiPart || firstPart)
    {
      int savedLastResult=sms.getLastResult();
      try{
        sms.setLastResult(GET_STATUS_CODE(t.command->get_resp()->get_status()));
        if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
        {
          std::string savedDivert=sms.getStrProperty(Tag::SMSC_DIVERTED_TO);
          sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
          smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,final,Smsc::chargeOnDelivery);
          sms.setStrProperty(Tag::SMSC_DIVERTED_TO,savedDivert.c_str());
        }else
        {
          smsc->ReportDelivery(t.command->get_resp()->get_inDlgId(),sms,final,Smsc::chargeOnDelivery);
        }
      }catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"ReportDelivery for Id=%lld failed:'%s'",t.msgId,e.what());
      }
      sms.setLastResult(savedLastResult);
      /*
      if(multiPart && statusType==CMD_OK && (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x02) && firstPart)
      {
        smsc_log_info(smsLog,"Remove billing flag for multipart sms msgId=%lld",t.msgId);
        sms.setBillingRecord(0);
      }
      */
    }
  }

  int sttype=GET_STATUS_TYPE(t.command->get_resp()->get_status());
  info2(smsLog, "DLVRSP: Id=%lld;class=%s;st=%d;oa=%s;%s;srcprx=%s;dstprx=%s;route=%s;%s%s",t.msgId,
      sttype==CMD_OK?"OK":
      sttype==CMD_ERR_RESCHEDULENOW?"RESCHEDULEDNOW":
      sttype==CMD_ERR_TEMP?"TEMP ERROR":"PERM ERROR",
      GET_STATUS_CODE(t.command->get_resp()->get_status()),
      sms.getOriginatingAddress().toString().c_str(),
      AddrPair("da",sms.getDestinationAddress(),"dda",sms.getDealiasedDestinationAddress()).c_str(),
      sms.getSourceSmeId(),
      sms.getDestinationSmeId(),
      sms.getRouteId(),
      t.command->get_resp()->get_diverted()?";diverted_to=":"",
      t.command->get_resp()->get_diverted()?sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str():""
    );



  if(GET_STATUS_TYPE(t.command->get_resp()->get_status())==CMD_OK)
  {
    sms.setLastResult(Status::OK);
    onDeliveryOk(t.msgId,sms);
  }else
  {
    sms.setLastResult(GET_STATUS_CODE(t.command->get_resp()->get_status()));
    if(dgortr)
    {
      onUndeliverable(t.msgId,sms);
    }else
    {
      switch(GET_STATUS_TYPE(t.command->get_resp()->get_status()))
      {
        case CMD_ERR_RESCHEDULENOW:
        case CMD_ERR_TEMP:
        {
          onDeliveryFail(t.msgId,sms);
        }break;
        default:
        {
          onUndeliverable(t.msgId,sms);
        }break;
      }
    }
  }



  if(GET_STATUS_TYPE(t.command->get_resp()->get_status())!=CMD_OK)
  {
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2 &&
       sms.getIntProperty(Tag::SMPP_SET_DPF))//forward/transaction mode
    {
      int status=GET_STATUS_CODE(t.command->get_resp()->get_status());
      if(status==1179 || status==1044)
      {
        try{
          if(!smsc->getScheduler()->registerSetDpf(
              sms.getDealiasedDestinationAddress(),
              sms.getOriginatingAddress(),
              status,
              sms.getValidTime(),
              sms.getSourceSmeId()))
          {
            sms.setIntProperty(Tag::SMPP_SET_DPF,0);
          }
        }catch(std::exception& e)
        {
          warn2(smsLog,"Failed to create dpf sms:%s",e.what());
          sms.setIntProperty(Tag::SMPP_SET_DPF,0);
        }
      }else
      {
        sms.setIntProperty(Tag::SMPP_SET_DPF,0);
      }
    }

    switch(GET_STATUS_TYPE(t.command->get_resp()->get_status()))
    {
      case CMD_ERR_RESCHEDULENOW:
      {
        time_t rt=time(NULL)+2;
        if(t.command->get_resp()->get_delay()>2)
        {
          rt+=t.command->get_resp()->get_delay()-2;
        }
        smsc_log_debug(smsLog,"DELIVERYRESP: change state to enroute (reschedule now=%d)",rt);
        changeSmsStateToEnroute
        (
            sms,
            t.msgId,
            sms.getDestinationDescriptor(),
            Status::RESCHEDULEDNOW,
            rt,
            true
        );

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        return UNKNOWN_STATE;
      }break;
      case CMD_ERR_TEMP:
      {
        if(!dgortr)
        {
          StateType st=DivertProcessing(t,sms);
          if(st!=UNKNOWN_STATE)return st;
        }
        time_t rt;
        if(t.command->get_resp()->get_delay()>2)
        {
          rt=time(NULL)+t.command->get_resp()->get_delay();
        }else
        {
          rt=rescheduleSms(sms);
        }
        changeSmsStateToEnroute
        (
            sms,
            t.msgId,
            sms.getDestinationDescriptor(),
            GET_STATUS_CODE(t.command->get_resp()->get_status()),
            rt
        );

        sendNotifyReport(sms,t.msgId,"subscriber busy");
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        return UNKNOWN_STATE;
      }break;
      default:
      {
        if(!dgortr)
        {
          try{
            smsc_log_debug(smsLog,"DELIVERYRESP: change state to undeliverable");

            if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
            {
              sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
              store->replaceSms(t.msgId,sms);
            }

            store->changeSmsStateToUndeliverable
            (
              t.msgId,
              sms.getDestinationDescriptor(),
              GET_STATUS_CODE(t.command->get_resp()->get_status())
            );
          }catch(std::exception& e)
          {
            smsc_log_warn(smsLog,"DELIVERYRESP: failed to change state to undeliverable:%s",e.what());
          }
        }

        sendFailureReport(sms,t.msgId,UNDELIVERABLE_STATE,"permanent error");

        if(!dgortr)
        {
          smsc->getScheduler()->InvalidSms(t.msgId);
        }

#ifdef SMSEXTRA
        if((sms.billingRecord!=BILLING_NONE && sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit) || sms.billingRecord==BILLING_FINALREP)
        {
          smsc->FullReportDelivery(t.msgId,sms);
        }
#else
        if(sms.billingRecord==BILLING_FINALREP || sms.billingRecord==BILLING_ONSUBMIT || sms.billingRecord==BILLING_CDR)
        {
          smsc->FullReportDelivery(t.msgId,sms);
        }
#endif

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
        }
        return UNDELIVERABLE_STATE;
      }
    }
  }


#ifdef SMSEXTRA
  if(!sms.hasBinProperty(Tag::SMSC_CONCATINFO) ||
     (sms.hasBinProperty(Tag::SMSC_CONCATINFO) && sms.getConcatSeqNum()==0))
  {
    if(createCopyOnNickUsage && (sms.getIntProperty(Tag::SMSC_EXTRAFLAGS)&EXTRA_NICK))
    {
      SMS newsms=sms;
      newsms.setIntProperty(Tag::SMSC_EXTRAFLAGS,EXTRA_FAKE);
      try{
        SMSId msgId=store->getNextId();
        newsms.setSourceSmeId(smscSmeId.c_str());
        store->createSms(newsms,msgId,smsc::store::CREATE_NEW);
        smsc->getScheduler()->AddFirstTimeForward(msgId,newsms);
      }catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"Failed to create fake sms for Id=%lld;oa=%s;da=%s;err='%s'",t.msgId,sms.getOriginatingAddress().toString().c_str(),
                      sms.getDealiasedDestinationAddress().toString().c_str(),e.what());
      }
    }
  }
#endif

  // concatenated message with conditional divert.
  // first part delivered ok.
  // other parts MUST be delivered to the same address.
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO) && sms.getConcatSeqNum()==0 &&
     (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&(DF_COND|DF_UNCOND)))
  {
    debug2(smsLog,"DLVRESP: Id=%lld - delivered first part of multipart sms with conditional divert.",t.msgId);
    // first part was delivered to diverted address!
    if(t.command->get_resp()->get_diverted())
    {
      // switch to unconditional divert.
      debug1(smsLog,"deliver to divert address");
      int df=sms.getIntProperty(Tag::SMSC_DIVERTFLAGS);
      sms.setIntProperty(Tag::SMSC_UDH_CONCAT,df&DF_UDHCONCAT);
      int dc=(df>>DF_DCSHIFT)&0xFF;
      sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,dc);
      sms.setIntProperty(Tag::SMSC_DIVERTFLAGS,df|DF_UNCOND);

      if(!sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        sms.getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
        partitionSms(&sms);
      }

    }else // first part was delivered to original address
    {
      // turn off divert
      debug1(smsLog,"deliver to original address");
      sms.getMessageBody().dropIntProperty(Tag::SMSC_DIVERTFLAGS);
    }
    try{
      //patch sms in store
      store->replaceSms(t.msgId,sms);
    }catch(...)
    {
       //shit happens
      smsc_log_warn(smsLog,"failed to replace sms in store (divert fix) Id=%lld",t.msgId);
    }
  }

  //bool skipFinalizing=false;

  vector<unsigned char> umrList; //umrs of parts of merged message
  //int umrIndex=-1;//index of current umr
  //bool umrLast=true;//need to generate receipts for the rest of umrs
  //int savedCsn=sms.getConcatSeqNum();

  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    if(smsc->getSmartMultipartForward() && (sms.getIntProperty(Tag::SMPP_ESM_CLASS)&3)==2)
    {
      info2(smsLog,"enabling smartMultipartForward  for Id=%lld",t.msgId);
      try{
        sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&~3);
        store->createSms(sms,t.msgId,smsc::store::CREATE_NEW_NO_CLEAR);
      }catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"DELIVERYRESP: failed to create sms for SmartMultipartForward:'%s'",e.what());
        finalizeSms(t.msgId,sms);
        onUndeliverable(t.msgId,sms);
        return UNDELIVERABLE_STATE;
      }
      dgortr=false;
      SmeProxy *src_proxy=smsc->getSmeProxy(sms.srcSmeId);
      if(src_proxy)
      {
        char msgId[64];
        sprintf(msgId,"%lld",t.msgId);
        SmscCommand resp=SmscCommand::makeSubmitSmResp
                         (
                           msgId,
                           sms.dialogId,
                           sms.lastResult,
                           sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
        try{
          src_proxy->putCommand(resp);
        }catch(...)
        {
          smsc_log_warn(smsLog,"DELIVERYRESP: failed to put transaction response command");
        }
      }
    }

    unsigned int len;
    ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
    info2(smsLog, "DLVRSP: sms has concatinfo, csn=%d/%d;Id=%lld",sms.getConcatSeqNum(),ci->num,t.msgId);
    if(sms.getConcatSeqNum()<ci->num-1)
    {
      {
        sms.setConcatSeqNum(sms.getConcatSeqNum()+1);
        if(!dgortr)
        try
        {
          store->changeSmsConcatSequenceNumber(t.msgId);
        }catch(std::exception& e)
        {
          smsc_log_warn(smsLog,"DELIVERYRESP: failed to change sms concat seq num Id=%lld;err='%s'",t.msgId,e.what());
          try{
            sms.setLastResult(sms.getLastResult());
            changeSmsStateToEnroute
            (
              sms,
              t.msgId,
              sms.getDestinationDescriptor(),
              Status::SYSERR,
              rescheduleSms(sms)
            );
          }catch(...)
          {
            smsc_log_warn(smsLog,"DELIVERYRESP: failed to cahnge sms state to enroute:Id=%lld",t.msgId);
          }
          if(dgortr)
          {
            sms.state=UNDELIVERABLE;
            finalizeSms(t.msgId,sms);
            onUndeliverable(t.msgId,sms);
            return UNDELIVERABLE_STATE;
          }else
          {
            onDeliveryFail(t.msgId,sms);
          }
          return UNKNOWN_STATE;
        }
        smsc_log_debug(smsLog,"CONCAT: concatseqnum=%d for Id=%lld",sms.getConcatSeqNum(),t.msgId);
      }

      ////
      //
      //  send concatenated
      //

      SmeProxy *dest_proxy=0;
      int dest_proxy_index;

      Address dst=sms.getDealiasedDestinationAddress();

      // for interfaceVersion==0x50
      if(!sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
      {
        if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&(DF_COND|DF_UNCOND)))
        {
          dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
        }
      }

      smsc::router::RouteInfo ri;
      bool has_route = false;
      try{
        has_route=smsc->routeSms(sms.getOriginatingAddress(),dst,dest_proxy_index,dest_proxy,&ri,smsc->getSmeIndex(sms.getSourceSmeId()));
      }catch(std::exception& e)
      {
        warn2(smsLog,"Routing %s->%s failed:%s",sms.getOriginatingAddress().toString().c_str(),
          dst.toString().c_str(),e.what());
      }
      if ( !has_route )
      {
        smsc_log_warn(smsLog,"CONCAT: no route Id=%lld;oa=%s;da=%s",t.msgId,sms.getOriginatingAddress().toString().c_str(),dst.toString().c_str());
        sms.setLastResult(Status::NOROUTE);
        sendNotifyReport(sms,t.msgId,"destination unavailable");
        //time_t now=time(NULL);
        Descriptor d;
        changeSmsStateToEnroute(sms,t.msgId,d,Status::NOROUTE,rescheduleSms(sms));

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          onUndeliverable(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }else
        {
          onDeliveryFail(t.msgId,sms);
        }
        return ERROR_STATE;
      }

      if(sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) &&
         sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
      {
        smsc_log_debug(smsLog,"CONCAT: diverted receipt for %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
        dest_proxy_index=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
        dest_proxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
      }

      if(!dest_proxy)
      {
        smsc_log_info(smsLog,"CONCAT: dest sme %s not connected msgId=%lld;oa=%s;da=%s",sms.getDestinationSmeId(),t.msgId,
            sms.getOriginatingAddress().toString().c_str(),
            sms.getDestinationAddress().toString().c_str());
        sms.setLastResult(Status::SMENOTCONNECTED);
        sendNotifyReport(sms,t.msgId,"destination unavailable");
        Descriptor d;
        changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          return UNDELIVERABLE_STATE;
          onUndeliverable(t.msgId,sms);
        }else
        {
          onDeliveryFail(t.msgId,sms);
        }
        return ENROUTE_STATE;
      }
      // create task

      uint32_t dialogId2;
      uint32_t uniqueId=dest_proxy->getUniqueId();

      TaskGuard tg;
      tg.smsc=smsc;
      tg.uniqueId=uniqueId;

      try{
        dialogId2 = dest_proxy->getNextSequenceNumber();
        tg.dialogId=dialogId2;

        Task task(uniqueId,dialogId2,dgortr?new SMS(sms):0);
        task.messageId=t.msgId;
        task.inDlgId=t.command->get_resp()->get_inDlgId();
        task.diverted=t.command->get_resp()->get_diverted();
        if ( smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
        {
          tg.active=true;
        }
      }catch(...)
      {
        Descriptor d;
        changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          onUndeliverable(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }else
        {
          onDeliveryFail(t.msgId,sms);
        }
        return ENROUTE_STATE;
      }
      Address srcOriginal=sms.getOriginatingAddress();
      Address dstOriginal=sms.getDestinationAddress();
      int errstatus=0;
      const char* errtext;
      try{
        // send delivery
        Address src;
        if(smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias &&
           sms.getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
           ri.hide &&
           smsc->AddressToAlias(sms.getOriginatingAddress(),src))
        {
          sms.setOriginatingAddress(src);
        }
        //Address dst=sms.getDealiasedDestinationAddress();
        sms.setDestinationAddress(dst);

        //
        //

        if(!extractSmsPart(&sms,sms.getConcatSeqNum()))
        {
          throw ExtractPartFailedException();
        }

        if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
        {
          uint32_t clen;
          ConcatInfo *nci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&clen);
          if(sms.getConcatSeqNum()<nci->num-1)
          {
            sms.setIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND,1);
          }else
          {
            sms.getMessageBody().dropIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND);
          }
        }
        if(ri.replyPath==smsc::router::ReplyPathForce)
        {
          sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
        }else if(ri.replyPath==smsc::router::ReplyPathSuppress)
        {
          sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
        }

        smsc_log_debug(smsLog,"CONCAT: Id=%lld;esm_class=%x",t.msgId,sms.getIntProperty(Tag::SMPP_ESM_CLASS));
        SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
        dest_proxy->putCommand(delivery);
        tg.active=false;
      }
      catch(ExtractPartFailedException& e)
      {
        errstatus=Status::INVPARLEN;
        smsc_log_error(smsLog,"CONCAT: failed to extract sms part for Id=%lld",t.msgId);
        errtext="failed to extract sms part";
      }
      catch(InvalidProxyCommandException& e)
      {
        errstatus=Status::INVBNDSTS;
        errtext="service rejected";
      }
      catch(...)
      {
        errstatus=Status::THROTTLED;
        errtext="SME busy";
      }
      if(errstatus)
      {
        smsc_log_warn(smsLog,"CONCAT::Err %s",errtext);
        try{
          Descriptor d;
          sms.setLastResult(errstatus);
          if(Status::isErrorPermanent(errstatus))
          {
            smsc->getScheduler()->InvalidSms(t.msgId);
            sendFailureReport(sms,t.msgId,UNDELIVERABLE_STATE,"permanent error");
            store->changeSmsStateToUndeliverable(t.msgId,d,errstatus);
          }
          else
            changeSmsStateToEnroute(sms,t.msgId,d,errstatus,rescheduleSms(sms));
        }catch(...)
        {
          smsc_log_warn(smsLog,"CONCAT: failed to change state to enroute");
        }
        if(dgortr)
        {
          sms.state=UNDELIVERABLE;
          finalizeSms(t.msgId,sms);
          onUndeliverable(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }
        if(Status::isErrorPermanent(errstatus))
        {
          onUndeliverable(t.msgId,sms);
          return UNDELIVERABLE_STATE;
        }else
        {
          onDeliveryFail(t.msgId,sms);
          return ENROUTE_STATE;
        }
      }

      return DELIVERING_STATE;
    }
  }

  if(sms.hasBinProperty(Tag::SMSC_UMR_LIST))
  {
    unsigned len;
    unsigned char* lst=(unsigned char*)sms.getBinProperty(Tag::SMSC_UMR_LIST,&len);
    if(!sms.hasBinProperty(Tag::SMSC_UMR_LIST_MASK))
    {
      umrList.insert(umrList.end(),lst,lst+len);
      //umrIndex=sms.hasBinProperty(Tag::SMSC_CONCATINFO)?savedCsn:0;
    }else
    {
      unsigned mlen;
      const char* mask=sms.getBinProperty(Tag::SMSC_UMR_LIST_MASK,&mlen);
      if(mlen<len)mlen=len;
      for(int i=0;i<mlen;i++)
      {
        if(mask[i])
        {
          umrList.push_back(lst[i]);
        }
      }
    }
  }

  if(dgortr)
  {
    sms.state=DELIVERED;
    if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==0x2)
    {
      SmeProxy *src_proxy=smsc->getSmeProxy(sms.srcSmeId);
      if(src_proxy)
      {
        char msgId[64];
        sprintf(msgId,"%lld",t.msgId);
        SmscCommand resp=SmscCommand::makeSubmitSmResp
            (
                msgId,
                sms.dialogId,
                sms.lastResult,
                sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
            );
        try{
          src_proxy->putCommand(resp);
        }catch(...)
        {

        }
      }
    }
    if(sms.billingRecord==BILLING_FINALREP  || sms.billingRecord==BILLING_ONSUBMIT || sms.billingRecord==BILLING_CDR)
    {
      smsc->FullReportDelivery(t.msgId,sms);
    }
    try{
      store->createFinalizedSms(t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog,"DELRESP: failed to finalize sms with Id=%lld",t.msgId);
    }
    return DELIVERED_STATE;
  }else if(!finalized)
  {
    try{

      if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && !t.command->get_resp()->get_diverted())
      {
        sms.getMessageBody().dropProperty(Tag::SMSC_DIVERTED_TO);
        store->replaceSms(t.msgId,sms);
      }

      store->changeSmsStateToDelivered(t.msgId,t.command->get_resp()->getDescriptor());

      smsc->getScheduler()->DeliveryOk(t.msgId);

    }catch(std::exception& e)
    {
      warn2(smsLog,"change state to delivered exception:%s",e.what());
      //return UNKNOWN_STATE;
    }
  }
  debug2(smsLog, "DLVRSP: DELIVERED, Id=%lld",t.msgId);

#ifdef SMSEXTRA
  if((sms.billingRecord && sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit) || sms.billingRecord==BILLING_FINALREP)
  {
    smsc->FullReportDelivery(t.msgId,sms);
  }
#else
  if(sms.billingRecord==BILLING_FINALREP  || sms.billingRecord==BILLING_ONSUBMIT || sms.billingRecord==BILLING_CDR)
  {
    smsc->FullReportDelivery(t.msgId,sms);
  }
#endif



  try{
    smsc_log_debug(smsLog,"DELIVERYRESP: suppdelrep=%d, delrep=%d, regdel=%d, srr=%d",
      sms.getIntProperty(Tag::SMSC_SUPPRESS_REPORTS),
      (int)sms.getDeliveryReport(),
      sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY),
      sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST));

    bool regdel=(sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==1 ||
                sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST);

    if(
       (
         sms.getIntProperty(Tag::SMSC_SUPPRESS_REPORTS)
       ) ||
       (
         sms.getDeliveryReport()==REPORT_NOACK
       ) ||
       (
         sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP)
       )
      )
    {
      return DELIVERED_STATE;
    }
    if(
        sms.getDeliveryReport() ||
        regdel
      )
    {
      SMS rpt;
      rpt.setOriginatingAddress(scAddress);
      char msc[]="";
      char imsi[]="";
      rpt.setOriginatingDescriptor((uint8_t)strlen(msc),msc,(uint8_t)strlen(imsi),imsi,1);
      rpt.setValidTime(0);
      rpt.setDeliveryReport(0);
      rpt.setArchivationRequested(false);
      rpt.setIntProperty(Tag::SMPP_ESM_CLASS,
        sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST) ||
        (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==1?4:0);
      rpt.setDestinationAddress(sms.getOriginatingAddress());
      rpt.setMessageReference(sms.getMessageReference());
      rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
        sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
      if(umrList.size())
      {
        rpt.setMessageReference(umrList[0]);
        rpt.setIntProperty(Tag::SMSC_RECEIPT_MR,umrList[0]);
      }
      smsc_log_debug(smsLog,"RECEIPT: set mr[0]=%d",rpt.getMessageReference());
      rpt.setIntProperty(Tag::SMPP_MSG_STATE,SmppMessageState::DELIVERED);
      char addr[64];
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
      rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,(unsigned)time(NULL));
      rpt.setIntProperty(Tag::SMSC_RECEIPTED_MSG_SUBMIT_TIME,(unsigned)sms.getSubmitTime());
      SmeInfo si=smsc->getSmeInfo(sms.getSourceSmeId());
      if(si.hasFlag(sfForceReceiptToSme))
      {
        rpt.setStrProperty(Tag::SMSC_DIVERTED_TO,sms.getSourceSmeId());
      }
      char msgid[60];
      sprintf(msgid,"%lld",t.msgId);
      rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
      string out;
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      const Descriptor& d=sms.getDestinationDescriptor();
      smsc_log_debug(smsLog,"RECEIPT: msc=%s, imsi=%s",d.msc,d.imsi);
      char ddest[64];
      Address ddestaddr=sms.getDealiasedDestinationAddress();
      Address tmp;
      if(!smsc->AddressToAlias(ddestaddr,tmp))
      {
        sms.getDealiasedDestinationAddress().getText(ddest,sizeof(ddest));
      }else
      {
        sms.getDestinationAddress().getText(ddest,sizeof(ddest));
      }
      FormatData fd;
      fd.ddest=ddest;
      fd.addr=addr;
      time_t tz=common::TimeZoneManager::getInstance().getTimeZone(rpt.getDestinationAddress())+timezone;
      fd.submitDate=sms.getSubmitTime()+tz;
      fd.date=time(NULL)+tz;
      fd.msgId=msgid;
      fd.err="";
      fd.lastResult=0;
      fd.lastResultGsm=0;
      fd.msc=d.msc;
      smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
      fd.locale=profile.locale.c_str();
      fd.scheme=si.receiptSchemeName.c_str();

      formatDeliver(fd,out);
      rpt.getDestinationAddress().getText(addr,sizeof(addr));
      smsc_log_debug(smsLog,"RECEIPT: sending receipt to %s:%s",addr,out.c_str());
      if(sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
      {
        fillSms(&rpt,"",0,CONV_ENCODING_CP1251,profile.codepage,0);
      }else
      {
        fillSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,0);
      }

      //smsc->submitSms(prpt);

      submitReceipt(rpt,0x4);

      {
        for(int i=1;i<umrList.size();i++)
        {
          rpt.setMessageReference(umrList[i]);
          rpt.setIntProperty(Tag::SMSC_RECEIPT_MR,umrList[i]);
          smsc_log_debug(smsLog,"RECEIPT: set mr[i]=%d",i,rpt.getMessageReference());
          submitReceipt(rpt,0x4);
        }
      }
    }
  }catch(std::exception& e)
  {
    smsc_log_warn(smsLog,"DELIVERY_RESP:failed to submit receipt");
  }
  //return skipFinalizing?DELIVERING_STATE:DELIVERED_STATE;
  return DELIVERED_STATE;
}

}
}
