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

StateType StateMachine::forward(Tuple& t)
{
  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    smsc_log_warn(smsLog, "FWD: failed to retriveSms Id=%lld",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return UNKNOWN_STATE;
  }

  time_t now=time(NULL);
  if( sms.getNextTime()>now && sms.getAttemptsCount()==0 && (!t.command->is_reschedulingForward() || sms.getLastResult()==0) )
  {
    debug2(smsLog, "FWD: nextTime>now (%d>%d)",sms.getNextTime(),now);
    SmeIndex idx=smsc->getSmeIndex(sms.dstSmeId);
    smsc->getScheduler()->AddScheduledSms(t.msgId,sms,idx);
    sms.setLastResult(Status::INVSCHED);
    return sms.getState();
  }


  bool firstPart=true;
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    if(sms.getConcatSeqNum()!=0)
    {
      firstPart=false;
    }
  }

  info2(smsLog,"FWD: Id=%lld;oa=%s;da=%s;srcSme=%s",t.msgId,sms.originatingAddress.toString().c_str(),sms.destinationAddress.toString().c_str(),sms.getSourceSmeId());

  if(sms.getAttemptsCount()==0 && sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
  {
    onSubmitOk(t.msgId,sms);
  }else
  {
    onForwardOk(t.msgId,sms);
  }

  smsc_log_debug(smsLog,"orgMSC=%s, orgIMSI=%s",sms.originatingDescriptor.msc,sms.originatingDescriptor.imsi);
  INFwdSmsChargeResponse::ForwardContext ctx;
  ctx.allowDivert=t.command->get_forwardAllowDivert();
  ctx.reschedulingForward=t.command->is_reschedulingForward();
  if(sms.billingRequired() && !sms.hasIntProperty(Tag::SMSC_CHARGINGPOLICY))
  {
    if(strcmp(sms.getSourceSmeId(),"MAP_PROXY")==0 && strcmp(sms.getDestinationSmeId(),"MAP_PROXY")==0)
    {
      sms.setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->p2pChargePolicy);
    }else
    {
      sms.setIntProperty(Tag::SMSC_CHARGINGPOLICY,smsc->otherChargePolicy);
    }
    try{
      store->replaceSms(t.msgId,sms);
    }catch(std::exception& e)
    {
      warn2(smsLog,"Failed to replace sms with Id=%lld in store:%s",t.msgId,e.what());
    }
  }

  if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery  && sms.billingRequired() && firstPart)
  {
    try{
      smsc->ChargeSms(t.msgId,sms,ctx);
    }catch(std::exception& e)
    {
      warn2(smsLog,"FWD: ChargeSms for Id=%lld failed:%s",t.msgId,e.what());
      sms.setLastResult(Status::NOCONNECTIONTOINMAN);
      onDeliveryFail(t.msgId,sms);
      Descriptor d;
      changeSmsStateToEnroute(sms,t.msgId,d,Status::NOCONNECTIONTOINMAN,rescheduleSms(sms));
      return ENROUTE_STATE;
    }
    return CHARGINGFWD_STATE;
  }else
  {
    t.command=SmscCommand::makeINFwdSmsChargeResponse(t.msgId,sms,ctx,1);
    return forwardChargeResp(t);
  }
}


StateType StateMachine::forwardChargeResp(Tuple& t)
{
  SMS& sms=t.command->get_fwdChargeSmsResp()->sms;
  int  inDlgId=t.command->get_fwdChargeSmsResp()->cntx.inDlgId;
  bool allowDivert=t.command->get_fwdChargeSmsResp()->cntx.allowDivert;
  bool isReschedulingForward=t.command->get_fwdChargeSmsResp()->cntx.reschedulingForward;

  if(!sms.Invalidate(__FILE__,__LINE__))
  {
    smsc_log_warn(smsLog, "Invalidate of %lld failed",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::INVOPTPARAMVAL);
    smsc->ReportDelivery(t.msgId,inDlgId,sms,true,Smsc::chargeOnDelivery);
    onDeliveryFail(t.msgId,sms);
    return ERROR_STATE;
  }

  if(sms.getState()==EXPIRED_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms in expired state Id=%lld",t.msgId);
    sms.setLastResult(Status::EXPIRED);
    smsc->getScheduler()->InvalidSms(t.msgId);
    onDeliveryFail(t.msgId,sms);
    return EXPIRED_STATE;
  }


  if(sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT) && sms.getIntProperty(Tag::SMSC_MERGE_CONCAT)!=3)
  {
    smsc_log_warn(smsLog, "Attempt to forward incomplete concatenated message Id=%lld",t.msgId);
    try{
      Descriptor d;
      sms.setLastResult(Status::SYSERR);
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToUndeliverable
      (
        t.msgId,
        d,
        Status::SYSERR
      );
    }catch(...)
    {
      smsc_log_warn(smsLog,"failed to change sms state to undeliverable");
    }
    smsc->ReportDelivery(t.msgId,inDlgId,sms,true,Smsc::chargeOnDelivery);
    onDeliveryFail(t.msgId,sms);
    return UNDELIVERABLE_STATE;
  }

  if(sms.getState()!=ENROUTE_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms Id=%lld is not in enroute (%d)",t.msgId,sms.getState());
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::SYSERR);
    smsc->ReportDelivery(t.msgId,inDlgId,sms,true,Smsc::chargeOnDelivery);
    onDeliveryFail(t.msgId,sms);
    return sms.getState();
  }

  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: Failed to change state of USSD request to undeliverable. Id=%lld",t.msgId);
    }
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::EXPIRED);
    smsc->ReportDelivery(t.msgId,inDlgId,sms,true,Smsc::chargeOnDelivery);
    onUndeliverable(t.msgId,sms);
    return UNDELIVERABLE_STATE;
  }


  if(sms.getLastTime()>sms.getValidTime() || sms.getNextTime()==1)
  {
    sms.setLastResult(Status::EXPIRED);
    try{
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FWD: failed to change state to expired");
    }
    info2(smsLog, "FWD: Id=%lld expired lastTry(%u)>valid(%u) or max attempts reached(%d:%d)",
          t.msgId,sms.getLastTime(),sms.getValidTime(),
          sms.oldResult,sms.getAttemptsCount());
    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
    smsc->ReportDelivery(t.msgId,inDlgId,sms,true,Smsc::chargeOnDelivery);
    onUndeliverable(t.msgId,sms);
    return EXPIRED_STATE;
  }

  if(!t.command->get_fwdChargeSmsResp()->result)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: denied by inman Id=%lld;oa=%s;da=%s;err='%s'",t.msgId,bufsrc,bufdst,t.command->get_fwdChargeSmsResp()->inmanError.c_str());
    sms.setLastResult(Status::DENIEDBYINMAN);
    onDeliveryFail(t.msgId,sms);
    sendNotifyReport(sms,t.msgId,"destination unavailable");
    Descriptor d;
    changeSmsStateToEnroute(sms,t.msgId,d,Status::DENIEDBYINMAN,rescheduleSms(sms));
    return ENROUTE_STATE;
  }

  Address dst=sms.getDealiasedDestinationAddress();

  bool diverted=false;
  bool doRepartition=false;
  if(allowDivert &&
     (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&DF_COND) &&
     !(sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&DF_UNCOND)
    )
  {
    try{
      dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
      Address newdst;
      if(smsc->AliasToAddress(dst,newdst))dst=newdst;
      debug2(smsLog,"FWD: cond divert from %s to %s",
        sms.getDealiasedDestinationAddress().toString().c_str(),dst.toString().c_str());
      diverted=true;
      int df=sms.getIntProperty(Tag::SMSC_DIVERTFLAGS);
      sms.setIntProperty(Tag::SMSC_UDH_CONCAT,df&DF_UDHCONCAT);
      int olddc=sms.getIntProperty(Tag::SMSC_DSTCODEPAGE);
      int newdc=(df>>DF_DCSHIFT)&0xFF;
      sms.setIntProperty(Tag::SMSC_DSTCODEPAGE,newdc&(~smsc::profiler::ProfileCharsetOptions::UssdIn7Bit));
      debug2(smsLog,"FWD: set dstdc to %x for diverted msg (was %x)",newdc,olddc);
      if(olddc!=newdc && sms.hasBinProperty(Tag::SMSC_CONCATINFO) && !sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT))
      {
        doRepartition=true;
        sms.getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
      }
    }catch(...)
    {
      smsc_log_warn(smsLog,"FWD: failed to construct address for cond divert %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    }
  }

  //for interfaceVersion==0x50
  if(!sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
  {
    if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO) && (sms.getIntProperty(Tag::SMSC_DIVERTFLAGS)&DF_UNCOND))
    {
      dst=sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str();
      Address newdst;
      diverted=true;
      if(smsc->AliasToAddress(dst,newdst))
      {
        dst=newdst;
      }
    }
  }

  smsc::router::RoutingResult rr;
  bool has_route = false;
  try{
    has_route=smsc->routeSms(smsc->getSmeIndex(sms.getSourceSmeId()),sms.getOriginatingAddress(),dst,rr);
  }catch(std::exception& e)
  {
    warn2(smsLog,"Routing %s->%s failed:%s",sms.getOriginatingAddress().toString().c_str(),
     dst.toString().c_str(),e.what());
  }
  if ( !has_route )
  {
    char from[32],to[32];
    sms.getOriginatingAddress().toString(from,sizeof(from));
    sms.getDestinationAddress().toString(to,sizeof(to));
    smsc_log_warn(smsLog, "FWD: No route Id=%lld;oa=%s;da=%s",t.msgId,from,to);
    sms.setLastResult(Status::NOROUTE);
    sendNotifyReport(sms,t.msgId,"destination unavailable");
    time_t ntt=rescheduleSms(sms);
    if(ntt==sms.getValidTime())
    {
      try{
        store->changeSmsStateToExpired(t.msgId);
      }catch(...)
      {
        smsc_log_warn(smsLog,"FORWARD: Failed to change state of USSD request to undeliverable. Id=%lld",t.msgId);
      }
      smsc->getScheduler()->InvalidSms(t.msgId);
      sms.setLastResult(Status::EXPIRED);
      onUndeliverable(t.msgId,sms);
    }else
    {
      Descriptor d;
      changeSmsStateToEnroute(sms,t.msgId,d,Status::NOROUTE,ntt);
      onDeliveryFail(t.msgId,sms);
    }
    smsc->ReportDelivery(t.msgId,inDlgId,sms,false,Smsc::chargeOnDelivery);
    return ERROR_STATE;
  }

  smsc_log_debug(smsLog,"FWD: last result=%d",sms.getLastResult());
  if(sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) &&
     sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
  {
    smsc_log_debug(smsLog,"FWD: diverted receipt for %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    rr.destSmeIdx=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    rr.destProxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
  }else if(sms.getLastResult()==Status::BACKUPSMERESCHEDULE && sms.hasStrProperty(Tag::SMSC_BACKUP_SME))
  {
    smsc_log_debug(smsLog,"FWD: reroute to backup sme %s",sms.getStrProperty(Tag::SMSC_BACKUP_SME).c_str());
    rr.destSmeIdx=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_BACKUP_SME).c_str());
    rr.destProxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_BACKUP_SME).c_str());
  }


  if(!rr.destProxy)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: sme is not connected Id=%lld;oa=%s;da=%s;dstSme=%s",t.msgId,bufsrc,bufdst,rr.info.srcSmeSystemId.c_str());
    sms.setLastResult(Status::SMENOTCONNECTED);
    onDeliveryFail(t.msgId,sms);
    sendNotifyReport(sms,t.msgId,"destination unavailable");
    Descriptor d;
    changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));
    smsc->ReportDelivery(t.msgId,inDlgId,sms,false,Smsc::chargeOnDelivery);
    return ENROUTE_STATE;
  }
  // create task


  if(doRepartition && rr.info.destSmeSystemId=="MAP_PROXY")
  {
    debug2(smsLog,"FWD: sms repartition Id=%lld",t.msgId);
    int pres=partitionSms(&sms);
    if(pres!=psSingle && pres!=psMultiple)
    {
      debug2(smsLog,"FWD: divert failed - cannot concat, Id=%lld",t.msgId);
      sms.setLastResult(Status::SYSERR);
      smsc->ReportDelivery(t.msgId,inDlgId,sms,true,Smsc::chargeOnDelivery);
      Descriptor d;
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SYSERR,rescheduleSms(sms));
      return UNKNOWN_STATE;
    }
    debug2(smsLog,"Id=%lld after repartition: %s",t.msgId,pres==psSingle?"single":"multiple");
  }

  SmeInfo dstSmeInfo=smsc->getSmeInfo(rr.destSmeIdx);

  if(dstSmeInfo.systemId!=sms.getDestinationSmeId() || rr.info.routeId!=sms.getRouteId())
  {
    sms.setDestinationSmeId(dstSmeInfo.systemId.c_str());
    sms.setRouteId(rr.info.routeId.c_str());
    try{
      store->replaceSms(t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog,"Failed to replace Id=%lld",t.msgId);
    }
  }

  if(dstSmeInfo.hasFlag(sfForceReceiptToSme) && sms.hasStrProperty(Tag::SMSC_RECIPIENTADDRESS))
  {
    sms.setOriginatingAddress(sms.getStrProperty(Tag::SMSC_RECIPIENTADDRESS).c_str());
  }

  TaskGuard tg;
  tg.smsc=smsc;


  uint32_t dialogId2;
  uint32_t uniqueId=rr.destProxy->getUniqueId();
  bool taskCreated=false;
  try{
    dialogId2 = rr.destProxy->getNextSequenceNumber();
    tg.dialogId=dialogId2;
    tg.uniqueId=uniqueId;
    debug2(smsLog, "FWDDLV: Id=%lld;seq=%d",t.msgId,dialogId2);
    //Task task((uint32_t)dest_proxy_index,dialogId2);
    Task task(uniqueId,dialogId2);
    task.diverted=diverted;
    task.messageId=t.msgId;
    task.inDlgId=inDlgId;
    if ( smsc->tasks.createTask(task,rr.destProxy->getPreferredTimeout()) )
    {
      taskCreated=true;
      tg.active=true;
    }
  }catch(...)
  {
  }
  if(!taskCreated)
  {
    smsc_log_info(smsLog,"Failed to create task for Id=%lld",t.msgId);
    Descriptor d;
    sms.setLastResult(Status::SMENOTCONNECTED);
    changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));
    onDeliveryFail(t.msgId,sms);
    smsc->ReportDelivery(t.msgId,inDlgId,sms,false,Smsc::chargeOnDelivery);
    return ENROUTE_STATE;
  }
  Address srcOriginal=sms.getOriginatingAddress();
  Address dstOriginal=sms.getDestinationAddress();
  int errstatus=0;
  const char* errtext;
  try{
    // send delivery
    Address src;
    if(
        smsc->getSmeInfo(rr.destProxy->getIndex()).wantAlias &&
        sms.getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
        rr.info.hide &&
        smsc->AddressToAlias(sms.getOriginatingAddress(),src)
      )
    {
      sms.setOriginatingAddress(src);
    }
    //Address dst=sms.getDealiasedDestinationAddress();
    sms.setDestinationAddress(dst);
    if(!sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 && !rr.info.transit)
      {
        prepareSmsDc(sms,dstSmeInfo.hasFlag(sfDefaultDcLatin1));
      }
    }else
    {
      unsigned int len;
      ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
      if(sms.getConcatSeqNum()<ci->num)
      {
        if(!extractSmsPart(&sms,sms.getConcatSeqNum()))
        {
          throw ExtractPartFailedException();
        }
        sms.setIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND,1);
      }else
      {
        smsc_log_warn(smsLog,"attempt to forward concatenated message but all parts are delivered!!!");
        try{
          Descriptor d;
          smsc->getScheduler()->InvalidSms(t.msgId);
          store->changeSmsStateToUndeliverable
          (
            t.msgId,
            d,
            Status::SYSERR
          );
        }catch(...)
        {
          smsc_log_warn(smsLog,"failed to change state of sms %lld to final ... again!!!",t.msgId);
        }
        return ERROR_STATE;
      }

    }
    if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      uint32_t len;
      ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
      if(sms.getConcatSeqNum()<ci->num-1)
      {
        sms.setIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND,1);
      }
    }
    if(rr.info.replyPath==smsc::router::ReplyPathForce)
    {
      sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
    }else if(rr.info.replyPath==smsc::router::ReplyPathSuppress)
    {
      sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
    }
    smsc_log_debug(smsLog,"FWD: Id=%lld, esm_class=%x",t.msgId,sms.getIntProperty(Tag::SMPP_ESM_CLASS));
    SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
    rr.destProxy->putCommand(delivery);
    tg.active=false;
  }
  catch(ExtractPartFailedException& e)
  {
    errstatus=Status::INVPARLEN;
    smsc_log_error(smsLog,"FWDDLV: failed to extract sms part for %lld",t.msgId);
    errtext="failed to extract sms part";
  }
  catch(InvalidProxyCommandException& e)
  {
    errstatus=Status::INVBNDSTS;
    errtext="service rejected";
  }
  catch(exception& e)
  {
    smsc_log_warn(smsLog, "FWDDLV: failed create deliver, exception:%s",e.what());
    errstatus=Status::THROTTLED;
    errtext="SME busy";
  }
  catch(...)
  {
    errstatus=Status::THROTTLED;
    errtext="SME busy";
  }
  if(errstatus)
  {
    smsc_log_warn(smsLog, "FWDDLV: failed create deliver(%s) srcSme=%s;Id=%lld;seqNum=%d;oa=%s;da=%s",errtext,
        sms.getSourceSmeId(),t.msgId,dialogId2,
        sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str());

    sms.setOriginatingAddress(srcOriginal);
    sms.setDestinationAddress(dstOriginal);
    sms.setLastResult(errstatus);
    onDeliveryFail(t.msgId,sms);
    if(Status::isErrorPermanent(errstatus))
      sendFailureReport(sms,t.msgId,errstatus,"system failure");
    else
      sendNotifyReport(sms,t.msgId,"system failure");
    try{
      Descriptor d;
      if(Status::isErrorPermanent(errstatus))
      {
        smsc->getScheduler()->InvalidSms(t.msgId);
        store->changeSmsStateToUndeliverable(t.msgId,d,errstatus);
      }
      else
        changeSmsStateToEnroute(sms,t.msgId,d,errstatus,rescheduleSms(sms));
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: failed to change state to enroute/undeliverable");
    }
    smsc->ReportDelivery(t.msgId,inDlgId,sms,Status::isErrorPermanent(errstatus),Smsc::chargeOnDelivery);
    return Status::isErrorPermanent(errstatus)?UNDELIVERABLE_STATE:ENROUTE_STATE;
  }
  info2(smsLog, "FWDDLV: deliver ok Id=%lld;seqNum=%d;oa=%s;da=%s;srcSme=%s;dstSme=%s;routeId=%s",t.msgId,dialogId2,
      sms.getOriginatingAddress().toString().c_str(),
      sms.getDealiasedDestinationAddress().toString().c_str(),
      sms.getSourceSmeId(),
      sms.getDestinationSmeId(),
      rr.info.routeId.c_str());

  return DELIVERING_STATE;
}


}
}
