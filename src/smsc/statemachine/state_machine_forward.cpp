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
    smsc_log_warn(smsLog, "FWD: failed to retriveSms %lld",t.msgId);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return UNKNOWN_STATE;
  }

  bool firstPart=true;
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    if(sms.getConcatSeqNum()!=0)
    {
      firstPart=false;
    }
  }

  info2(smsLog,"FWD: id=%lld,oa=%s,da=%s,mp=%c,fp=%c",t.msgId,sms.originatingAddress.toString().c_str(),sms.destinationAddress.toString().c_str(),sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT)?'Y':'N',firstPart?'Y':'N');
#ifdef SNMP
  SnmpCounter::getInstance().incCounter(SnmpCounter::cntRetried,sms.getDestinationSmeId());
#endif

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
      warn2(smsLog,"Failed to replace sms with msgId=%lld in store:%s",t.msgId,e.what());
    }
  }

  if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnDelivery  && sms.billingRequired() && firstPart)
  {
    try{
      smsc->ChargeSms(t.msgId,sms,ctx);
    }catch(std::exception& e)
    {
      warn2(smsLog,"FWD: ChargeSms for id=%lld failed:%s",t.msgId,e.what());
      sms.setLastResult(Status::NOCONNECTIONTOINMAN);
      smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
      smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
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
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return ERROR_STATE;
  }

  if(sms.getState()==EXPIRED_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms in expired state msgId=%lld",t.msgId);
    sms.setLastResult(Status::EXPIRED);
    smsc->getScheduler()->InvalidSms(t.msgId);
    return EXPIRED_STATE;
  }


  if(sms.hasIntProperty(Tag::SMSC_MERGE_CONCAT) && sms.getIntProperty(Tag::SMSC_MERGE_CONCAT)!=3)
  {
    smsc_log_warn(smsLog, "Attempt to forward incomplete concatenated message %lld",t.msgId);
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
      __warning__("failed to change sms state to undeliverable");
    }
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return UNDELIVERABLE_STATE;
  }

  if(sms.getState()!=ENROUTE_STATE)
  {
    smsc_log_warn(smsLog, "FWD: sms msgId=%lld is not in enroute (%d)",t.msgId,sms.getState());
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::SYSERR);
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return sms.getState();
  }
  time_t now=time(NULL);
  if( sms.getNextTime()>now && sms.getAttemptsCount()==0 && (!isReschedulingForward || sms.getLastResult()==0) )
  {
    debug2(smsLog, "FWD: nextTime>now (%d>%d)",sms.getNextTime(),now);
    SmeIndex idx=smsc->getSmeIndex(sms.dstSmeId);
    smsc->getScheduler()->AddScheduledSms(t.msgId,sms,idx);
    sms.setLastResult(Status::SYSERR);
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return sms.getState();
  }


  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: Failed to change state of USSD request to undeliverable. msgId=%lld",t.msgId);
    }
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::EXPIRED);
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return UNDELIVERABLE_STATE;
  }

  //
  //sms in forward mode forwarded. this mean, that sms with set_dpf was sent,
  //but request timed out. need to send alert notification with status unavialable.
  //
  // set_dpf reworked
/*
  if((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3)==2)
  {
    SmeProxy* proxy=smsc->getSmeProxy(sms.srcSmeId);
    debug2(smsLog,"Sending AlertNotification to '%s'",sms.srcSmeId);
    if(proxy!=0)
    {
      try{
        proxy->putCommand(
          SmscCommand::makeAlertNotificationCommand
          (
            proxy->getNextSequenceNumber(),
            sms.getDestinationAddress(),
            sms.getOriginatingAddress(),
            2
          )
        );
      }catch(std::exception& e)
      {
        warn2(smsLog,"Failed to put Alert Notification Command:%s",e.what());
      }
    }else
    {
      warn2(smsLog,"Sme %s requested dpf, but not connected at the moment",sms.srcSmeId);
    }
    try{
      store->changeSmsStateToDeleted(t.msgId);
    }catch(...)
    {
      smsc_log_warn(smsLog,"FORWARD: Failed to change state of fwd/dgm sms to undeliverable. msgId=%lld",t.msgId);
    }
    smsc->getScheduler()->InvalidSms(t.msgId);
    sms.setLastResult(Status::EXPIRED);
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return UNDELIVERABLE_STATE;
  }
  */

  if(sms.getAttemptsCount()==0 && sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID))
  {
    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
    smsc->registerMsuStatEvent(StatEvents::etSubmitOk,&sms);
  }else
  {
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
  }

  if(sms.getLastTime()>sms.getValidTime() || sms.getNextTime()==1)
  {
    sms.setLastResult(Status::EXPIRED);
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
    try{
      smsc->getScheduler()->InvalidSms(t.msgId);
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      __warning__("FWD: failed to change state to expired");
    }
    info2(smsLog, "FWD: %lld expired lastTry(%u)>valid(%u) or max attempts reached(%d:%d)",
          t.msgId,sms.getLastTime(),sms.getValidTime(),
          sms.oldResult,sms.getAttemptsCount());
    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");
    try{
      smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return EXPIRED_STATE;
  }

  if(!t.command->get_fwdChargeSmsResp()->result)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: msgId=%lld denied by inman(%s->%s)",t.msgId,bufsrc,bufdst);
    sms.setLastResult(Status::DENIEDBYINMAN);
//    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
    try{
      sendNotifyReport(sms,t.msgId,"destination unavailable");
    }catch(...)
    {
      __warning__("FORWARD: failed to send intermediate notification");
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      changeSmsStateToEnroute(sms,t.msgId,d,Status::DENIEDBYINMAN,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }

  ////
  //
  // Traffic Control
  //


  /*
  if(!smsc->allowCommandProcessing(t.command))
  {
    sms.setLastResult(Status::THROTTLED);
    Descriptor d;
    //__trace__("FORWARD: traffic control denied forward");
    try{
      changeSmsStateToEnroute(sms,t.msgId,d,Status::THROTTLED,rescheduleSms(sms));
    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
    return ENROUTE_STATE;
  };
  */

  //
  // End of traffic Control
  //
  ////


  SmeProxy *dest_proxy=0;
  int dest_proxy_index;

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
      if(smsc->AliasToAddress(dst,newdst))dst=newdst;
    }
  }

  smsc::router::RouteInfo ri;
  bool has_route = false;
  try{
    has_route=smsc->routeSms
                    (
                      sms.getOriginatingAddress(),
                      dst,
                      dest_proxy_index,
                      dest_proxy,
                      &ri,
                      smsc->getSmeIndex(sms.getSourceSmeId())
                    );
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
    smsc_log_warn(smsLog, "FWD: msgId=%lld, No route (%s->%s)",t.msgId,from,to);
    try{
      sms.setLastResult(Status::NOROUTE);
      sendNotifyReport(sms,t.msgId,"destination unavailable");
    }catch(...)
    {
      __warning__("FORWARD: failed to send intermediate notification");
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      changeSmsStateToEnroute(sms,t.msgId,d,Status::NOROUTE,rescheduleSms(sms));

    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return ERROR_STATE;
  }

  if(sms.hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) &&
     sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
  {
    smsc_log_debug(smsLog,"FWD: diverted receipt for %s",sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    dest_proxy_index=smsc->getSmeIndex(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
    dest_proxy=smsc->getSmeProxy(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
  }

  if(!dest_proxy)
  {
    char bufsrc[64],bufdst[64];
    sms.getOriginatingAddress().toString(bufsrc,sizeof(bufsrc));
    sms.getDestinationAddress().toString(bufdst,sizeof(bufdst));
    smsc_log_warn(smsLog, "FWD: msgId=%lld sme is not connected(%s->%s(%s))",t.msgId,bufsrc,bufdst,ri.smeSystemId.c_str());
    sms.setLastResult(Status::SMENOTCONNECTED);
#ifdef SNMP
    incSnmpCounterForError(Status::SMENOTCONNECTED,ri.smeSystemId.c_str());
#endif
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
    try{
      sendNotifyReport(sms,t.msgId,"destination unavailable");
    }catch(...)
    {
      __trace__("FORWARD: failed to send intermediate notification");
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return ENROUTE_STATE;
  }
  // create task


  if(doRepartition && ri.smeSystemId=="MAP_PROXY")
  {
    debug2(smsLog,"FWD: sms repartition %lld",t.msgId);
    int pres=partitionSms(&sms);
    if(pres!=psSingle && pres!=psMultiple)
    {
      debug2(smsLog,"FWD: divert failed - cannot concat, msgId=%lld",t.msgId);
      try{
        sms.setLastResult(Status::SYSERR);
        smsc->ReportDelivery(inDlgId,sms,true,Smsc::chargeOnDelivery);
      }catch(std::exception& e)
      {
        smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
      }
      Descriptor d;
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SYSERR,rescheduleSms(sms));
      return UNKNOWN_STATE;
    }
    debug2(smsLog,"%lld after repartition: %s",t.msgId,pres==psSingle?"single":"multiple");
  }

  SmeInfo dstSmeInfo=smsc->getSmeInfo(dest_proxy_index);

  if(dstSmeInfo.systemId!=sms.getDestinationSmeId() || ri.routeId!=sms.getRouteId())
  {
    sms.setDestinationSmeId(dstSmeInfo.systemId.c_str());
    sms.setRouteId(ri.routeId.c_str());
    try{
      store->replaceSms(t.msgId,sms);
    }catch(...)
    {
      smsc_log_warn(smsLog,"Failed to replace smsId=%lld",t.msgId);
    }
  }

  if(dstSmeInfo.hasFlag(sfForceReceiptToSme) && sms.hasStrProperty(Tag::SMSC_RECIPIENTADDRESS))
  {
    sms.setOriginatingAddress(sms.getStrProperty(Tag::SMSC_RECIPIENTADDRESS).c_str());
  }

  TaskGuard tg;
  tg.smsc=smsc;


  uint32_t dialogId2;
  uint32_t uniqueId=dest_proxy->getUniqueId();
  try{
    dialogId2 = dest_proxy->getNextSequenceNumber();
    tg.dialogId=dialogId2;
    tg.uniqueId=uniqueId;
    debug2(smsLog, "FWDDLV: msgId=%lld, seq number:%d",t.msgId,dialogId2);
    //Task task((uint32_t)dest_proxy_index,dialogId2);
    Task task(uniqueId,dialogId2);
    task.diverted=diverted;
    task.messageId=t.msgId;
    task.inDlgId=inDlgId;
    if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
    {
      smsc_log_warn(smsLog, "FWDDLV: failed to create task msgId=%lld, seq number:%d",t.msgId,dialogId2);
      try{
        sms.setLastResult(Status::SYSERR);
        sendNotifyReport(sms,t.msgId,"destination unavailable");
      }catch(...)
      {
        __warning__("FORWARD: failed to send intermediate notification");
      }
      smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
      try{
        Descriptor d;
        __trace__("FORWARD: change state to enroute");
        sms.setLastResult(Status::SYSERR);
        changeSmsStateToEnroute(sms,t.msgId,d,Status::SYSERR,rescheduleSms(sms));

      }catch(...)
      {
        __warning__("FORWARD: failed to change state to enroute");
      }
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
      return ENROUTE_STATE;
    }
    tg.active=true;
  }catch(...)
  {
    smsc_log_warn(smsLog, "FWDDLV: failed to get seqnum msgId=%lld",t.msgId);
    try{
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      sms.setLastResult(Status::SMENOTCONNECTED);
#ifdef SNMP
    incSnmpCounterForError(Status::SMENOTCONNECTED,ri.smeSystemId.c_str());
#endif
      smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
      changeSmsStateToEnroute(sms,t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    try{
      smsc->ReportDelivery(inDlgId,sms,false,Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
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
    if(
        smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias &&
        sms.getIntProperty(Tag::SMSC_HIDE)==HideOption::hoEnabled &&
        ri.hide &&
        smsc->AddressToAlias(sms.getOriginatingAddress(),src)
      )
    {
      sms.setOriginatingAddress(src);
    }
    //Address dst=sms.getDealiasedDestinationAddress();
    sms.setDestinationAddress(dst);
    if(!sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      if(sms.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 && !ri.transit)
      {
        using namespace smsc::profiler::ProfileCharsetOptions;
        if(
           (
             (sms.getIntProperty(Tag::SMSC_DSTCODEPAGE)==Default ||
              sms.getIntProperty(Tag::SMSC_DSTCODEPAGE)==Latin1
             )
             && sms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2
           ) ||
           (
             (sms.getIntProperty(Tag::SMSC_DSTCODEPAGE)&Latin1)!=Latin1 &&
             sms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::LATIN1
           )
          )
        {
          try{
            transLiterateSms(&sms,sms.getIntProperty(Tag::SMSC_DSTCODEPAGE));
            if(sms.hasIntProperty(Tag::SMSC_ORIGINAL_DC))
            {
              int dc=sms.getIntProperty(Tag::SMSC_ORIGINAL_DC);
              int olddc=dc;
              if((dc&0xc0)==0 || (dc&0xf0)==0xf0) //groups 00xx and 1111
              {
                dc&=0xf3; //11110011 - clear 2-3 bits (set alphabet to default).

              }else if((dc&0xf0)==0xe0)
              {
                dc=0xd0 | (dc&0x0f);
              }
              sms.setIntProperty(Tag::SMSC_ORIGINAL_DC,dc);
              __trace2__("FORWARD: transliterate olddc(%x)->dc(%x)",olddc,dc);
            }
          }catch(exception& e)
          {
            __warning2__("SUBMIT:Failed to transliterate: %s",e.what());
          }
        }
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
      }else
      {
        __warning__("attempt to forward concatenated message but all parts are delivered!!!");
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
          __warning2__("failed to change state of sms %lld to final ... again!!!",t.msgId);
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
    if(ri.replyPath==smsc::router::ReplyPathForce)
    {
      sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)|0x80);
    }else if(ri.replyPath==smsc::router::ReplyPathSuppress)
    {
      sms.setIntProperty(Tag::SMPP_ESM_CLASS,sms.getIntProperty(Tag::SMPP_ESM_CLASS)&(~0x80));
    }
    smsc_log_debug(smsLog,"FWD: msgId=%lld, esm_class=%x",t.msgId,sms.getIntProperty(Tag::SMPP_ESM_CLASS));
    SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
    dest_proxy->putCommand(delivery);
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
    smsc_log_warn(smsLog, "FWDDLV: failed create deliver(%s) srcSme=%s msgId=%lld, seq number:%d",errtext,sms.getSourceSmeId(),t.msgId,dialogId2);

    sms.setOriginatingAddress(srcOriginal);
    sms.setDestinationAddress(dstOriginal);
    sms.setLastResult(errstatus);
    smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
//    sendNotifyReport(sms,t.msgId,errtext);
#ifdef SNMP
    if(Status::isErrorPermanent(errstatus))
    {
      SnmpCounter::getInstance().incCounter(SnmpCounter::cntFailed,sms.getDestinationSmeId());
    }else
    {
      incSnmpCounterForError(errstatus,sms.getDestinationSmeId());
    }
#endif
    try{
      if(Status::isErrorPermanent(errstatus))
        sendFailureReport(sms,t.msgId,errstatus,"system failure");
      else
        sendNotifyReport(sms,t.msgId,"system failure");
    }catch(std::exception& e)
    {
      __warning2__("failed to submit receipt:%s",e.what());
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
        if(Status::isErrorPermanent(errstatus))
        {
          smsc->getScheduler()->InvalidSms(t.msgId);
          store->changeSmsStateToUndeliverable(t.msgId,d,errstatus);
        }
        else
          changeSmsStateToEnroute(sms,t.msgId,d,errstatus,rescheduleSms(sms));

    }catch(...)
    {
      __warning__("FORWARD: failed to change state to enroute");
    }
    Task tsk;
    smsc->tasks.findAndRemoveTask(uniqueId,dialogId2,&tsk);
    try{
      smsc->ReportDelivery(inDlgId,sms,Status::isErrorPermanent(errstatus),Smsc::chargeOnDelivery);
    }catch(std::exception& e)
    {
      smsc_log_warn(smsLog,"ReportDelivery for %lld failed:'%s'",t.msgId,e.what());
    }
    return Status::isErrorPermanent(errstatus)?UNDELIVERABLE_STATE:ENROUTE_STATE;
  }
  info2(smsLog, "FWDDLV: deliver ok msgId=%lld, seq number:%d",t.msgId,dialogId2);

  return DELIVERING_STATE;
}

}
}
