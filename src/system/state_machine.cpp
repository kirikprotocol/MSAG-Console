#include "util/debug.h"
#include "system/smsc.hpp"
#include "system/state_machine.hpp"
#include <exception>
#include "system/rescheduler.hpp"
#include "profiler/profiler.hpp"
#include "util/recoder/recode_dll.h"
#include "core/buffers/Hash.hpp"
#include "util/smstext.h"
#include "util/Logger.h"
#include "util/regexp/RegExp.hpp"
#include "core/synchronization/Mutex.hpp"
#include "system/status.h"

namespace smsc{
namespace system{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace StateTypeValue;
using namespace smsc::smpp;
using namespace util;
using namespace smsc::system;
using std::exception;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using smsc::core::synchronization::Mutex;

class ReceiptGetAdapter:public GetAdapter{
public:

  virtual bool isNull(const char* key)
      throw(AdapterException)
  {
    return false;
  }

  virtual const char* getString(const char* key)
      throw(AdapterException)
  {
    return "";
  }

  virtual int8_t getInt8(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int16_t getInt16(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int32_t getInt32(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual int64_t getInt64(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual uint8_t getUint8(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint16_t getUint16(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint32_t getUint32(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual uint64_t getUint64(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual float getFloat(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual double getDouble(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

  virtual long double getLongDouble(const char* key)
      throw(AdapterException)
  {
    return 0;
  }


  virtual time_t getDateTime(const char* key)
      throw(AdapterException)
  {
    return 0;
  }

};

void StateMachine::formatDeliver(const char* addr,time_t date,std::string& out)
{
  if(!ofDelivered)return;
  ReceiptGetAdapter ga;
  ContextEnvironment ce;
  ce.exportStr("dest",addr);
  ce.exportDat("date",date);
  try{
    ofDelivered->format(out,ga,ce);
  }catch(exception& e)
  {
    __trace2__("FORMATTER: %s",e.what());
  }
}
void StateMachine::formatFailed(const char* addr,const char* err,time_t date,std::string& out)
{
  if(!ofFailed)return;
  ReceiptGetAdapter ga;
  ContextEnvironment ce;
  ce.exportStr("dest",addr);
  ce.exportStr("error",err);
  ce.exportDat("date",date);

  try{
    ofFailed->format(out,ga,ce);
  }catch(exception& e)
  {
    __trace2__("FORMATTER: %s",e.what());
  }
}

void StateMachine::formatNotify(const char* addr,const char* reason,time_t date,std::string& out)
{
  if(!ofNotify)return;
  ReceiptGetAdapter ga;
  ContextEnvironment ce;
  ce.exportStr("dest",addr);
  ce.exportStr("reason",reason);
  ce.exportDat("date",date);

  try{
    ofNotify->format(out,ga,ce);
  }catch(exception& e)
  {
    __trace2__("FORMATTER: %s",e.what());
  }
}


int StateMachine::Execute()
{
  Tuple t;
  StateType st;
  for(;;)
  {
    eq.selectAndDequeue(t,&isStopping);
    if(isStopping)break;
    try{
      switch(t.command->cmdid)
      {
        case SUBMIT:st=submit(t);break;
        case DELIVERY_RESP:st=deliveryResp(t);break;
        case FORWARD:st=forward(t);break;
        case ALERT:st=alert(t);break;
        case REPLACE:st=replace(t);break;
        case QUERY:st=query(t);break;
        case CANCEL:st=cancel(t);break;
        default:
          __warning2__("UNKNOWN COMMAND:%d",t.command->cmdid);
          st=ERROR_STATE;
          break;
      }
      __trace2__("change state for %lld to %d",t.msgId,st);
      eq.changeState(t.msgId,st);
    }catch(exception& e)
    {
      __warning2__("StateMachine::exception %s",e.what());
    }
    catch(...)
    {
      __warning2__("StateMachine::unknown exception");
    }
  }
  __trace__("exit state machine");
  return 0;
}

Hash<RegExp*> reCache;
Mutex reCacheMtx;
static struct CacheKiller{
~CacheKiller()
{
  char* key;
  RegExp* re;
  reCache.First();
  while(reCache.Next(key,re))
  {
    delete re;
  }
  reCache.Empty();
}
}ck;

bool checkSourceAddress(const std::string& pattern,const Address& src)
{
  char buf[32];
  src.toString(buf,sizeof(buf));
  MutexGuard g(reCacheMtx);
  RegExp **reptr=reCache.GetPtr(pattern.c_str());
  RegExp *re;
  if(!reptr)
  {
    re=new RegExp();
    if(!re->Compile(pattern.c_str(),OP_OPTIMIZE|OP_STRICT))
    {
      smsc::util::Logger::getCategory("smsc.system.StateMachine").
        error("Failed to compile address range regexp");
    }
    reCache.Insert(pattern.c_str(),re);
  }else
  {
    re=*reptr;
  }
  SMatch m[10];
  int n=10;
  return re->Match(buf,m,n);
}



StateType StateMachine::submit(Tuple& t)
{
  __require__(t.state==UNKNOWN_STATE);

  SmeProxy *src_proxy,*dest_proxy=0;

  src_proxy=t.command.getProxy();

  __require__(src_proxy!=NULL);

  __trace2__("StateMachine::submit:%lld",t.msgId);

  SMS* sms = t.command->get_sms();
  uint32_t dialogId =  t.command->get_dialogId();

  if(sms->getNextTime()==-1)
  {
    sms->lastResult=Status::INVSCHED;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVSCHED,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    __warning__("SUBMIT_SM: invalid schedule");
    return ERROR_STATE;
  }

  if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) &&
     sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE)   &&
     sms->getIntProperty(Tag::SMPP_SM_LENGTH)!=0)
  {
    sms->lastResult=Status::SUBMITFAIL;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::SUBMITFAIL,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    __warning__("SUBMIT_SM: both short_message and payload present!!!");
    return ERROR_STATE;
  }

  if(sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::DEFAULT &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::UCS2 &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::BINARY &&
     sms->getIntProperty(Tag::SMPP_DATA_CODING)!=DataCoding::SMSC7BIT)
  {
    sms->lastResult=Status::INVDCS;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                       (
                         /*messageId*/"0",
                         dialogId,
                         Status::INVDCS,
                         sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                       );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    __warning2__("SUBMIT_SM: invalid datacoding %d",sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING));
    return ERROR_STATE;
  }

  if(sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
  {
    unsigned len;
    const unsigned char* msg;
    if(sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      msg=(const unsigned char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    }
    else
    {
      msg=(const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    }
    if(sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
    {
      len-=*msg+1;
    }
    ////
    // Unicode message with odd length
    if(len&1)
    {
      sms->lastResult=Status::INVMSGLEN;
      smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
      SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVMSGLEN,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
      try{
        src_proxy->putCommand(resp);
      }catch(...)
      {
      }
      __warning__("SUBMIT_SM: invalid message length");
      return ERROR_STATE;
    }
  }


  if(sms->getValidTime()==-1)
  {
    sms->lastResult=Status::INVEXPIRY;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVEXPIRY,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    __warning__("SUBMIT_SM: invalid valid time");
    return ERROR_STATE;
  }

  if(src_proxy->getSourceAddressRange().length() &&
     !checkSourceAddress(src_proxy->getSourceAddressRange(),sms->getOriginatingAddress()))
  {
    sms->lastResult=Status::INVSRCADR;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVSRCADR,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    char buf[256];
    sms->getOriginatingAddress().toString(buf,sizeof(buf));
    __warning2__("SUBMIT_SM: invalid source address:%s(%s)",buf,
      src_proxy->getSourceAddressRange().c_str());
    return ERROR_STATE;
  }

  time_t now=time(NULL);
  sms->setSubmitTime(now);

  __trace2__("SUBMIT: seq=%d",dialogId);
  int dest_proxy_index;
  // route sms
  //SmeProxy* dest_proxy = 0;
  Address dst;
  __trace2__("AliasToAddress: %d.%d.%.20s",
    sms->getDestinationAddress().type,
    sms->getDestinationAddress().plan,
    sms->getDestinationAddress().value);
  if(smsc->AliasToAddress(sms->getDestinationAddress(),dst))
  {
    __trace2__("ALIAS:%20s->%20s",sms->getDestinationAddress().value,dst.value);
  }
  else
  {
    dst=sms->getDestinationAddress();
  }
  sms->setDealiasedDestinationAddress(dst);
  smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms->getOriginatingAddress());
  __trace2__("SUBMIT: lookup .%d.%d.%20s, result: %d,%d",sms->getOriginatingAddress().type,
    sms->getOriginatingAddress().plan,sms->getOriginatingAddress().value,
    profile.reportoptions,profile.codepage);

  sms->setDeliveryReport(profile.reportoptions);

  smsc::router::RouteInfo ri;
  bool has_route = smsc->routeSms(sms->getOriginatingAddress(),
                          dst,
                          dest_proxy_index,dest_proxy,&ri);
  sms->setRouteId(ri.routeId.c_str());
  int prio=sms->getPriority()+ri.priority;
  if(prio>SmeProxyPriorityMax)prio=SmeProxyPriorityMax;
  sms->setPriority(ri.priority);

  //smsc->routeSms(sms,dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    sms->lastResult=Status::NOROUTE;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    //send_no_route;
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::NOROUTE,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    char buf1[32];
    char buf2[32];
    sms->getOriginatingAddress().toString(buf1,sizeof(buf2));
    dst.toString(buf2,sizeof(buf2));
    __warning2__("SUBMIT_SM: no route (%s->%s)",buf1,buf2);
    return ERROR_STATE;
  }

  sms->setSourceSmeId(t.command->get_sourceId());

  sms->setDestinationSmeId(ri.smeSystemId.c_str());
  sms->setServiceId(ri.serviceId);


  __trace2__("SUBMIT: archivation request for %lld/%d is %s",t.msgId,dialogId,ri.archived?"true":"false");
  sms->setArchivationRequested(ri.archived);
  sms->setBillingRecord(ri.billing);

  if(sms->getValidTime()==0 || sms->getValidTime()>now+maxValidTime)
  {
    sms->setValidTime(now+maxValidTime);
    __trace2__("maxValidTime=%d",maxValidTime);
  }

  __trace2__("Valid time for sms %lld=%u",t.msgId,(unsigned int)sms->getValidTime());

  if(sms->getNextTime()>now+maxValidTime || sms->getNextTime()>sms->getValidTime())
  {
    sms->lastResult=Status::INVSCHED;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::INVSCHED,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    return ERROR_STATE;
  }

  profile=smsc->getProfiler()->lookup(dst);
  sms->setIntProperty(Tag::SMSC_DSTCODEPAGE,profile.codepage);
  int pres=psSingle;
  if(ri.smeSystemId=="MAP_PROXY")
  {
    pres=partitionSms(sms,profile.codepage);
  }
  if(pres==psErrorLength)
  {
    sms->lastResult=Status::INVMSGLEN;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                       (
                         /*messageId*/"0",
                         dialogId,
                         Status::INVMSGLEN,
                         sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                       );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT:Failed to put command");
    }
    unsigned int len;
    const char *msg=sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    __warning2__("SUBMIT_SM: invalid message length:%d",len);
    return ERROR_STATE;
  }

  if(pres==psErrorUdhi)
  {
    sms->lastResult=Status::SUBMITFAIL;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::SUBMITFAIL,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __warning__("SUBMIT_SM: failed to put response command");
    }
    __warning__("SUBMIT_SM: udhi present in concatenated message!!!");
    return ERROR_STATE;
  }

  if(pres==psMultiple)
  {
    uint8_t msgref=smsc->getNextMR(dst);
    sms->setConcatMsgRef(msgref);
    sms->setConcatSeqNum(0);
  }

  __trace2__("Replace if present for message %lld=%d",t.msgId,sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG));

  time_t stime=sms->getNextTime();


  try{
    if(sms->getNextTime()<now)
    {
      sms->setNextTime(now);
    }
    store->createSms(*sms,t.msgId,
      sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG)?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
  }catch(...)
  {
    __trace2__("failed to create sms with id %lld",t.msgId);
    sms->lastResult=Status::SYSERR;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,sms);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           /*messageId*/"0",
                           dialogId,
                           Status::SYSERR,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    return ERROR_STATE;
  }

  smsc->registerStatisticalEvent(StatEvents::etSubmitOk,sms);

  {
    // sms сохранена в базе, с выставленным Next Time, таким образом
    // даже если дальше что-то обломится, потом будет еще попытка послать её
    // то бишь мы приняли sms в обработку, можно слать ok.
    char buf[64];
    sprintf(buf,"%lld",t.msgId);
    SmscCommand resp = SmscCommand::makeSubmitSmResp
                         (
                           buf,
                           dialogId,
                           Status::OK,
                           sms->getIntProperty(Tag::SMPP_DATA_SM)!=0
                         );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
  }

  __trace2__("Sms scheduled to %d, now %d",(int)sms->getNextTime(),(int)now);
  if(stime>now)
  {
    smsc->notifyScheduler();
    return ENROUTE_STATE;
  }


  if ( !dest_proxy )
  {
    __warning__("SUBMIT_SM: SME is not connected");
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");

      sms->lastResult=Status::SMENOTCONNECTED;
      sendNotifyReport(*sms,t.msgId,"destination unavailable");

      store->changeSmsStateToEnroute(t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(*sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("SUBMIT_SM: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  // create task
  uint32_t dialogId2;
  try{
     dialogId2=dest_proxy->getNextSequenceNumber();
  }catch(...)
  {
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(*sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("SUBMIT_SM: failed to change state to enroute");
    }
    sms->lastResult=Status::SMENOTCONNECTED;
    sendNotifyReport(*sms,t.msgId,"destination unavailable");
    return ENROUTE_STATE;
  }
  __trace2__("DELIVER: seq:%d",dialogId2);
  //Task task((uint32_t)dest_proxy_index,dialogId2);
  try{
  Task task(dest_proxy->getUniqueId(),dialogId2);
  task.messageId=t.msgId;
  if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
  {
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("SUBMIT_SM: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,Status::SYSERR,rescheduleSms(*sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("SUBMIT_SM: failed to change state to enroute");
    }
    __warning__("SUBMIT_SM: can't create task");

    sms->lastResult=Status::SYSERR;
    sendNotifyReport(*sms,t.msgId,"system failure");

    return ENROUTE_STATE;
  }
  }catch(...)
  {
    __trace__("SUBMIT: failed to create task");
    sms->lastResult=Status::SYSERR;
    sendNotifyReport(*sms,t.msgId,"system failure");
    return ENROUTE_STATE;
  }
  Address srcOriginal=sms->getOriginatingAddress();
  Address dstOriginal=sms->getDestinationAddress();
  try{
    // send delivery
    Address src;
    __trace2__("SUBMIT: wantAlias=%s",smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias?"true":"false");
    if(smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias && smsc->AddressToAlias(sms->getOriginatingAddress(),src))
    {
      sms->setOriginatingAddress(src);
    }
    sms->setDestinationAddress(dst);

    // profile lookup performed before partitioning
    //profile=smsc->getProfiler()->lookup(dst);
    //
    if(!sms->hasBinProperty(Tag::SMSC_CONCATINFO))
    {
      if(sms->getIntProperty(Tag::SMSC_DSTCODEPAGE)==smsc::profiler::ProfileCharsetOptions::Default &&
         sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
      {
        try{
          transLiterateSms(sms);
        }catch(exception& e)
        {
          __warning2__("SUBMIT:Failed to transliterate: %s",e.what());
        }
      }
    }else
    {
      extractSmsPart(sms,0);
    }

    SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
    unsigned bodyLen=0;
    delivery->get_sms()->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&bodyLen);
    __trace2__("SUBMIT: delivery.sms.sm_length=%d",bodyLen);
    int prio=ri.priority/1000;
    if(prio<0)prio=0;
    if(prio>=32)prio=31;
    delivery->set_priority(prio);
    try{
      dest_proxy->putCommand(delivery);
    }catch(InvalidProxyCommandException& e)
    {
      sms->setOriginatingAddress(srcOriginal);
      sms->setDestinationAddress(dstOriginal);
      sms->lastResult=Status::INVBNDSTS;
      sendNotifyReport(*sms,t.msgId,"service rejected");
      __trace__("SUBMIT: Attempt to putCommand for sme in invalid bind state");
      try{
        Descriptor d;
        store->changeSmsStateToEnroute(t.msgId,d,Status::INVBNDSTS,rescheduleSms(*sms));
        smsc->notifyScheduler();
      }catch(...)
      {
        __trace__("SUBMIT: failed to change state to enroute");
      }
      return DELIVERING_STATE;
    }
  }catch(exception& e)
  {
    __warning2__("SUBMIT: failed to put delivery command:%s",e.what());
    sms->setOriginatingAddress(srcOriginal);
    sms->setDestinationAddress(dstOriginal);
    sms->lastResult=Status::THROTTLED;
    sendNotifyReport(*sms,t.msgId,"system failure");
    try{
      Descriptor d;
      store->changeSmsStateToEnroute(t.msgId,d,Status::THROTTLED,rescheduleSms(*sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("SUBMIT: failed to change state to enroute");
    }
    return DELIVERING_STATE;
  }catch(...)
  {
    __trace__("SUBMIT: failed to put delivery command");
    sms->setOriginatingAddress(srcOriginal);
    sms->setDestinationAddress(dstOriginal);
    sms->lastResult=Status::THROTTLED;
    sendNotifyReport(*sms,t.msgId,"system failure");
    try{
      Descriptor d;
      store->changeSmsStateToEnroute(t.msgId,d,Status::THROTTLED,rescheduleSms(*sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("SUBMIT: failed to change state to enroute");
    }
    return DELIVERING_STATE;
  }
  __trace2__("SUBMIT_SM:OK:%lld",t.msgId);
  return DELIVERING_STATE;
}

StateType StateMachine::forward(Tuple& t)
{
  SMS sms;
  __trace2__("FORWARD:%lld",t.msgId);
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    __trace__("FORWARD: failed to retriveSms");
    return UNKNOWN_STATE;
  }
  if(sms.getState()==EXPIRED_STATE)
  {
    return EXPIRED_STATE;
  }
  time_t now=time(NULL);
  if(sms.getValidTime()<=now && sms.getAttemptsCount()!=0)
  {
    sms.lastResult=Status::EXPIRED;
    smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
      __trace__("FORWARD: failed to change state to expired");
    }
    __trace2__("FORWARD: %lld expired (valid:%u - now:%u)",t.msgId,sms.getValidTime(),now);

    sendFailureReport(sms,t.msgId,EXPIRED_STATE,"expired");

    return EXPIRED_STATE;
  }
  if(sms.getState()!=ENROUTE_STATE)
  {
    return sms.getState();
  }
  if(sms.getNextTime()>now)
  {
    return sms.getState();
  }
  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    Descriptor d;
    try{
      store->changeSmsStateToUndeliverable(t.msgId,d,0);
    }catch(...)
    {
      __trace__("FORWARD: Failed to change state of USSD request to undeliverable");
    }
    return UNDELIVERABLE_STATE;
  }
  smsc->registerStatisticalEvent(StatEvents::etRescheduled,&sms);

  SmeProxy *dest_proxy=0;
  int dest_proxy_index;

  bool has_route = smsc->routeSms(sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),dest_proxy_index,dest_proxy,NULL);
  if ( !has_route )
  {
    __warning__("FORWARD: No route");
    try{
      sms.lastResult=Status::NOROUTE;
      sendNotifyReport(sms,t.msgId,"destination unavailable");
    }catch(...)
    {
      __trace__("FORWARD: failed to send intermediate notification");
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,Status::NOROUTE,rescheduleSms(sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    return ERROR_STATE;
  }
  if(!dest_proxy)
  {
    __trace__("FORWARD: no proxy");
    try{
      sms.lastResult=Status::SMENOTCONNECTED;
      sendNotifyReport(sms,t.msgId,"destination unavailable");
    }catch(...)
    {
      __trace__("FORWARD: failed to send intermediate notification");
    }
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  // create task

  uint32_t dialogId2;
  try{
    dialogId2 = dest_proxy->getNextSequenceNumber();
    __trace2__("FORWARD: seq number:%d",dialogId2);
    //Task task((uint32_t)dest_proxy_index,dialogId2);
    Task task(dest_proxy->getUniqueId(),dialogId2);
    task.messageId=t.msgId;
    if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
    {
      __warning__("FORWARD: can't create task");
      try{
        sms.lastResult=Status::SYSERR;
        sendNotifyReport(sms,t.msgId,"destination unavailable");
      }catch(...)
      {
        __trace__("FORWARD: failed to send intermediate notification");
      }
      return ENROUTE_STATE;
    }
  }catch(...)
  {
    return ENROUTE_STATE;
  }
  Address srcOriginal=sms.getOriginatingAddress();
  Address dstOriginal=sms.getDestinationAddress();
  int errstatus=0;
  const char* errtext;
  try{
    // send delivery
    Address src;
    if(smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias && smsc->AddressToAlias(sms.getOriginatingAddress(),src))
    {
      sms.setOriginatingAddress(src);
    }
    Address dst=sms.getDealiasedDestinationAddress();
    sms.setDestinationAddress(dst);
    if(!sms.hasBinProperty(Tag::SMSC_CONCATINFO))
    {

      if(sms.getIntProperty(Tag::SMSC_DSTCODEPAGE)==smsc::profiler::ProfileCharsetOptions::Default &&
         sms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
      {
        transLiterateSms(&sms);
      }
    }else
    {
      extractSmsPart(&sms,sms.getConcatSeqNum());
    }
    if(t.command->is_reschedulingForward())
    {
      try{
        Descriptor d;
        store->changeSmsStateToEnroute(t.msgId,d,sms.getLastTime(),time(NULL));
      }catch(...)
      {
        __trace__("FORWARD: failed to reschedule sms to now");
      }
    }
    SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
    dest_proxy->putCommand(delivery);
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
    //TODO!!!: remove task and reschedule
    __trace2__("FORWARD::Err %s",errtext);
    sms.setOriginatingAddress(srcOriginal);
    sms.setDestinationAddress(dstOriginal);
    sms.lastResult=errstatus;
    sendNotifyReport(sms,t.msgId,errtext);
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,errstatus,rescheduleSms(sms));
      smsc->notifyScheduler();
    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  return DELIVERING_STATE;
}

StateType StateMachine::deliveryResp(Tuple& t)
{
  __trace2__("delivering resp for :%lld",t.msgId);
  //__require__(t.state==DELIVERING_STATE);
  if(t.state!=DELIVERING_STATE)
  {
    __warning__("DELIVERYRESP: state of SMS isn't DELIVERING!!!");
    return t.state;
  }
  //smsc::sms::Descriptor d;
  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(exception& e)
  {
    __trace2__("DELIVERYRESP: failed to retrieve sms:%s",e.what());
    return UNKNOWN_STATE;
  }
  if(GET_STATUS_TYPE(t.command->get_resp()->get_status())!=CMD_OK)
  {
    switch(GET_STATUS_TYPE(t.command->get_resp()->get_status()))
    {
      case CMD_ERR_RESCHEDULENOW:
      {
        try{
          Descriptor d;
          __trace__("DELIVERYRESP: change state to enroute");
          time_t rt=time(NULL)+2;
          if(t.command->get_resp()->get_delay()!=-1)
          {
            rt=t.command->get_resp()->get_delay()-2;
          }
          store->changeSmsStateToEnroute(t.msgId,d,GET_STATUS_CODE(t.command->get_resp()->get_status()),rt,true);
        }catch(...)
        {
          __trace__("DELIVERYRESP: failed to change state to enroute");
        }
        smsc->notifyScheduler();
        sms.lastResult=GET_STATUS_CODE(t.command->get_resp()->get_status());
        smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
        return UNKNOWN_STATE;
      }break;
      case CMD_ERR_TEMP:
      {
        try{
          Descriptor d;
          __trace__("DELIVERYRESP: change state to enroute");
          time_t rt;
          if(t.command->get_resp()->get_delay()!=-1)
          {
            rt=t.command->get_resp()->get_delay();
          }else
          {
            rt=rescheduleSms(sms);
          }
          store->changeSmsStateToEnroute(t.msgId,d,GET_STATUS_CODE(t.command->get_resp()->get_status()),rt);
        }catch(...)
        {
          __trace__("DELIVERYRESP: failed to change state to enroute");
        }
        smsc->notifyScheduler();
        sms.lastResult=GET_STATUS_CODE(t.command->get_resp()->get_status());
        sendNotifyReport(sms,t.msgId,"subscriber busy");
        smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
        return UNKNOWN_STATE;
      }break;
      default:
      {
        try{
          Descriptor d;
          __trace__("DELIVERYRESP: change state to undeliverable");
          store->changeSmsStateToUndeliverable(t.msgId,d,GET_STATUS_CODE(t.command->get_resp()->get_status()));
        }catch(...)
        {
          __trace__("DELIVERYRESP: failed to change state to undeliverable");
        }

        sms.lastResult=GET_STATUS_CODE(t.command->get_resp()->get_status());
        sendFailureReport(sms,t.msgId,UNDELIVERABLE_STATE,"permanent error");
        smsc->registerStatisticalEvent(StatEvents::etUndeliverable,&sms);
        return UNDELIVERABLE_STATE;
      }
    }
  }

  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    unsigned int len;
    ConcatInfo *ci=(ConcatInfo*)sms.getBinProperty(Tag::SMSC_CONCATINFO,&len);
    sms.setConcatSeqNum(sms.getConcatSeqNum()+1);
    store->changeSmsConcatSequenceNumber(t.msgId);
    __trace2__("CONCAT: concatseqnum=%d for msdgId=%lld",sms.getConcatSeqNum(),t.msgId);
    if(sms.getConcatSeqNum()<ci->num)
    {
      ////
      //
      //  send concatenated
      //

      SmeProxy *dest_proxy=0;
      int dest_proxy_index;

      bool has_route = smsc->routeSms(sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),dest_proxy_index,dest_proxy,NULL);
      if ( !has_route )
      {
        __warning__("CONCAT: No route");
        try{
          sms.lastResult=Status::NOROUTE;
          sendNotifyReport(sms,t.msgId,"destination unavailable");
        }catch(...)
        {
          __trace__("CONCAT: failed to send intermediate notification");
        }
        try{
          //time_t now=time(NULL);
          Descriptor d;
          __trace__("CONCAT: change state to enroute");
          store->changeSmsStateToEnroute(t.msgId,d,Status::NOROUTE,rescheduleSms(sms));
          smsc->notifyScheduler();
        }catch(...)
        {
          __trace__("CONCAT: failed to change state to enroute");
        }
        return ERROR_STATE;
      }
      if(!dest_proxy)
      {
        __trace__("CONCAT: no proxy");
        try{
          sms.lastResult=Status::SMENOTCONNECTED;
          sendNotifyReport(sms,t.msgId,"destination unavailable");
        }catch(...)
        {
          __trace__("CONCAT: failed to send intermediate notification");
        }
        try{
          //time_t now=time(NULL);
          Descriptor d;
          __trace__("CONCAT: change state to enroute");
          store->changeSmsStateToEnroute(t.msgId,d,Status::SMENOTCONNECTED,rescheduleSms(sms));
          smsc->notifyScheduler();
        }catch(...)
        {
          __trace__("CONCAT: failed to change state to enroute");
        }
        return ENROUTE_STATE;
      }
      // create task

      uint32_t dialogId2;
      try{
        dialogId2 = dest_proxy->getNextSequenceNumber();
        __trace2__("CONCAT: seq number:%d",dialogId2);
        //Task task((uint32_t)dest_proxy_index,dialogId2);
        Task task(dest_proxy->getUniqueId(),dialogId2);
        task.messageId=t.msgId;
        if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
        {
          __warning__("CONCAT: can't create task");
          return ENROUTE_STATE;
        }
      }catch(...)
      {
        return ENROUTE_STATE;
      }
      Address srcOriginal=sms.getOriginatingAddress();
      Address dstOriginal=sms.getDestinationAddress();
      int errstatus=0;
      const char* errtext;
      try{
        // send delivery
        Address src;
        if(smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias && smsc->AddressToAlias(sms.getOriginatingAddress(),src))
        {
          sms.setOriginatingAddress(src);
        }
        Address dst=sms.getDealiasedDestinationAddress();
        sms.setDestinationAddress(dst);

        //
        //
        extractSmsPart(&sms,sms.getConcatSeqNum());

        SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
        dest_proxy->putCommand(delivery);
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
        __trace2__("CONCAT::Err %s",errtext);
      }

      //
      //
      //
      ////


      return DELIVERING_STATE;
    }
  }

  try{
    __trace__("change state to delivered");

    store->changeSmsStateToDelivered(t.msgId,t.command->get_resp()->getDescriptor());

    __trace__("change state to delivered: ok");
  }catch(std::exception& e)
  {
    __trace2__("change state to delivered exception:%s",e.what());
    log4cpp::Category &log=smsc::util::Logger::getCategory("smsc.system.StateMachine");
    log.error("Failed to change state to delivered for sms %lld",t.msgId);
    return UNKNOWN_STATE;
  }
  __trace__("DELIVERY_RESP: registerStatisticalEvent");
  smsc->registerStatisticalEvent(StatEvents::etDeliveredOk,&sms);
  try{
    //smsc::profiler::Profile p=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
    if(!sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
    {
      if(//p.reportoptions==smsc::profiler::ProfileReportOptions::ReportFull ||
         sms.getDeliveryReport() ||
         (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==1  ||
         sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
      {
        SMS *prpt=new SMS;
        SMS &rpt=*prpt;
        rpt.setOriginatingAddress(scAddress);
        char msc[]="";
        char imsi[]="";
        rpt.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
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
        rpt.setIntProperty(Tag::SMPP_MSG_STATE,DELIVERED_STATE);
        char addr[64];
        sms.getDestinationAddress().getText(addr,sizeof(addr));
        rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
        rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));
        char msgid[60];
        sprintf(msgid,"%lld",t.msgId);
        rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
        string out;
        sms.getDestinationAddress().getText(addr,sizeof(addr));
        formatDeliver(addr,time(NULL),out);
        rpt.getDestinationAddress().getText(addr,sizeof(addr));
        __trace2__("RECEIPT: sending receipt to %s:%s",addr,out.c_str());
        smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
        trimSms(prpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage);
        smsc->submitSms(prpt);
        /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
        for(int i=0;i<arr.Count();i++)
        {
          smsc->submitSms(arr[i]);
        };*/
      }
    }
  }catch(std::exception& e)
  {
    __trace2__("DELIVERY_RESP:failed to submit receipt");
  }
  return DELIVERED_STATE;
}

StateType StateMachine::alert(Tuple& t)
{
  __trace2__("ALERT: %lld",t.msgId);
  //time_t now=time(NULL);
  Descriptor d;
  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    __trace2__("ALERT: Failed to retrieve sms:%lld",t.msgId);
    return UNKNOWN_STATE;
  }
  try{
    store->changeSmsStateToEnroute(t.msgId,d,Status::DELIVERYTIMEDOUT,rescheduleSms(sms));
  }catch(std::exception& e)
  {
    __trace2__("ALERT: failed to change state to enroute:%s",e.what());
  }catch(...)
  {
    __trace2__("ALERT: failed to change state to enroute");
  }
  smsc->notifyScheduler();
  sms.lastResult=Status::DELIVERYTIMEDOUT;
  sendNotifyReport(sms,t.msgId,"delivery attempt timed out");
  smsc->registerStatisticalEvent(StatEvents::etDeliverErr,&sms);
  return UNKNOWN_STATE;
}

StateType StateMachine::replace(Tuple& t)
{
  __trace2__("REPLACE:msgid=%lld",t.msgId);
  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(...)
  {
    __trace2__("REPLACE: Failed to retrieve sms:%lld",t.msgId);
    try{
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeReplaceSmResp
        (
          t.command->get_dialogId(),
          Status::REPLACEFAIL
        )
      );
    }catch(...)
    {
    }
    return UNKNOWN_STATE;
  }
  if(sms.hasBinProperty(Tag::SMSC_CONCATINFO))
  {
    if(sms.getConcatSeqNum()>0)
    {
      __trace__("REPLACE: replace of concatenated message");
      try{
        t.command.getProxy()->putCommand
        (
          SmscCommand::makeReplaceSmResp
          (
            t.command->get_dialogId(),
            Status::REPLACEFAIL
          )
        );
      }catch(...)
      {
      }
      return UNKNOWN_STATE;
    }
    sms.getMessageBody().dropProperty(Tag::SMSC_CONCATINFO);
  }
  if(sms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
  {
    sms.getMessageBody().dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
  }

  try{
    Address addr(t.command->get_replaceSm().sourceAddr.get());
    if(!(sms.getOriginatingAddress()==addr))
    {
      throw Exception("replace failed");
    }
  }catch(...)
  {
    try{
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeReplaceSmResp
        (
          t.command->get_dialogId(),
          Status::REPLACEFAIL
        )
      );
    }catch(...)
    {
    }
    return UNKNOWN_STATE;
  }


  if(sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
  {
    if(
        t.command->get_replaceSm().smLength==0 ||
        *((uint8_t*)t.command->get_replaceSm().shortMessage.get())+1 >
        t.command->get_replaceSm().smLength
      )
    {
      __trace__("REPLACE: invalid length of/for UDHI");
      try{
        t.command.getProxy()->putCommand
        (
          SmscCommand::makeReplaceSmResp
          (
            t.command->get_dialogId(),
            Status::REPLACEFAIL
          )
        );
      }catch(...)
      {
        __trace__("REPLACE: failed to put response command");
      }
      return UNKNOWN_STATE;
    }
  }

  sms.setBinProperty
  (
    Tag::SMPP_SHORT_MESSAGE,
    t.command->get_replaceSm().shortMessage.get(),
    t.command->get_replaceSm().smLength
  );
  sms.setIntProperty(Tag::SMPP_SM_LENGTH,t.command->get_replaceSm().smLength);

  if(!strcmp(sms.getDestinationSmeId(),"MAP_PROXY"))
  {
    int pres=partitionSms(&sms,sms.getIntProperty(Tag::SMSC_DSTCODEPAGE));
    if(pres==psErrorUdhi || pres==psErrorUdhi)
    {
      __trace2__("REPLACE: concatenation failed(%d)",pres);
      try{
        t.command.getProxy()->putCommand
        (
          SmscCommand::makeReplaceSmResp
          (
            t.command->get_dialogId(),
            Status::REPLACEFAIL
          )
        );
      }catch(...)
      {
        __trace__("REPLACE: failed to put response command");
      }
      return UNKNOWN_STATE;
    }
    if(pres==psMultiple)
    {
      Address dst=sms.getDealiasedDestinationAddress();
      uint8_t msgref=smsc->getNextMR(dst);
      sms.setConcatMsgRef(msgref);
      sms.setConcatSeqNum(0);
    }
  }


  if(t.command->get_replaceSm().validityPeriod==-1)
  {
    sms.lastResult=Status::INVEXPIRY;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    try{
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeReplaceSmResp
        (
          t.command->get_dialogId(),
          Status::INVEXPIRY
        )
      );
    }catch(...)
    {
      __trace__("REPLACE: failed to put response command");
    }
    return UNKNOWN_STATE;
  }
  if(t.command->get_replaceSm().scheduleDeliveryTime==-1)
  {
    sms.lastResult=Status::INVSCHED;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    try{
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeReplaceSmResp
        (
          t.command->get_dialogId(),
          Status::INVSCHED
        )
      );
    }catch(...)
    {
      __trace__("REPLACE: failed to put response command");
    }
    return UNKNOWN_STATE;
  }
  time_t newvalid=t.command->get_replaceSm().validityPeriod?
    t.command->get_replaceSm().validityPeriod:sms.getValidTime();
  time_t newsched=t.command->get_replaceSm().scheduleDeliveryTime?
    t.command->get_replaceSm().scheduleDeliveryTime:sms.getNextTime();
  time_t now=time(NULL);
  if(newsched<now)newsched=now;
  if(newvalid>now+maxValidTime)newvalid=now+maxValidTime;

  if(newsched>newvalid)
  {
    sms.lastResult=Status::INVSCHED;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    try{
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeReplaceSmResp
        (
          t.command->get_dialogId(),
          Status::INVSCHED
        )
      );
    }catch(...)
    {
      __trace__("REPLACE: failed to put response command");
    }
    return UNKNOWN_STATE;
  }

  sms.setValidTime(newvalid);
  sms.setNextTime(newsched);
  sms.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,t.command->get_replaceSm().registeredDelivery);

  try{
    //Address addr(t.command->get_replaceSm().sourceAddr.get());
    store->replaceSms(t.msgId,sms);
    smsc->registerStatisticalEvent(StatEvents::etSubmitOk,&sms);
//  }catch(NoSuchMessageException& e)
//  {
    //
  }catch(...)
  {
    __trace__("REPLACE: replacefailed");
    //
    sms.lastResult=Status::SYSERR;
    smsc->registerStatisticalEvent(StatEvents::etSubmitErr,&sms);
    try{
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeReplaceSmResp
        (
          t.command->get_dialogId(),
          Status::REPLACEFAIL
        )
      );
    }catch(...)
    {
      __trace__("REPLACE: failed to put response command");
    }
    return UNKNOWN_STATE;
  }
  try{
    t.command.getProxy()->putCommand
    (
      SmscCommand::makeReplaceSmResp
      (
        t.command->get_dialogId(),
        Status::OK
      )
    );
  }catch(...)
  {
    __trace__("REPLACE: failed to put response command");
  }
  smsc->notifyScheduler();
  return ENROUTE;
}

StateType StateMachine::query(Tuple& t)
{
  __trace2__("QUERY:msguid=%lld",t.msgId);
  SMS sms;
  try{
    Address addr(t.command->get_querySm().sourceAddr.get());
    store->retriveSms(t.msgId,sms);
    if(!(sms.getOriginatingAddress()==addr))
    {
      throw 0;
    }
  }catch(...)
  {
    t.command.getProxy()->putCommand
    (
      SmscCommand::makeQuerySmResp
      (
        t.command->get_dialogId(),
        Status::QUERYFAIL,
        t.msgId,
        0,
        0,
        0
      )
    );
    return t.state;
  }
  int state=7;
  switch(sms.getState())
  {
    case ENROUTE:      state=1;break;
    case DELIVERED:    state=2;break;
    case EXPIRED:      state=3;break;
    case DELETED:      state=4;break;
    case UNDELIVERABLE:state=5;break;
  }
  t.command.getProxy()->putCommand
  (
    SmscCommand::makeQuerySmResp
    (
      t.command->get_dialogId(),
      Status::OK,
      t.msgId,
      state==1?0:sms.getLastTime(),
      state,
      0
    )
  );
  return t.state;
}

StateType StateMachine::cancel(Tuple& t)
{
  __trace2__("CANCEL: msgid=%lld",t.msgId);
  SMS sms;
  try{
    Address addr(t.command->get_cancelSm().sourceAddr.get());
    store->retriveSms(t.msgId,sms);

    if(sms.hasBinProperty(Tag::SMSC_CONCATINFO) && sms.getConcatSeqNum()>0)
    {
      throw Exception("CANCEL: attempt to cancel concatenated message already in delivery");
    }

    if(!(sms.getOriginatingAddress()==addr))
    {
      throw Exception("CANCEL: source address doesn't match");
    }
    if(t.command->get_cancelSm().destAddr.get())
    {
      addr=Address(t.command->get_cancelSm().destAddr.get());
      if(!(sms.getDestinationAddress()==addr))
      {
        throw Exception("CANCEL: destination address doesn't match");
      }
    }
  }catch(...)
  {
    if(!t.command->get_cancelSm().internall)
    {
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeCancelSmResp
        (
          t.command->get_dialogId(),
          Status::CANCELFAIL
        )
      );
    }
    return t.state;
  }
  try{
    store->changeSmsStateToDeleted(t.msgId);
  }catch(...)
  {
    if(!t.command->get_cancelSm().internall)
    {
      t.command.getProxy()->putCommand
      (
        SmscCommand::makeCancelSmResp
        (
          t.command->get_dialogId(),
          Status::CANCELFAIL
        )
      );
    }
    return t.state;
  }
  if(!t.command->get_cancelSm().internall)
  {
    t.command.getProxy()->putCommand
    (
      SmscCommand::makeCancelSmResp
      (
        t.command->get_dialogId(),
        Status::OK
      )
    );
  }
  return t.state;
}


void StateMachine::sendFailureReport(SMS& sms,MsgIdType msgId,int state,const char* reason)
{
  if(!(
        sms.getDeliveryReport() ||
        (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==1 ||
        (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==2 ||
        sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST)
      )
    )return;
  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))return;
  SMS *prpt=new SMS;
  SMS &rpt=*prpt;
  rpt.setOriginatingAddress(scAddress);
  char msc[]="";
  char imsi[]="";
  rpt.lastResult=sms.lastResult;
  rpt.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  rpt.setValidTime(0);
  rpt.setDeliveryReport(0);
  rpt.setArchivationRequested(false);
  rpt.setIntProperty(Tag::SMPP_ESM_CLASS,
    sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST) ||
    (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==1 ||
    (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==2
    ?4:0);
  rpt.setDestinationAddress(sms.getOriginatingAddress());
  rpt.setMessageReference(sms.getMessageReference());
  rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
    sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
  rpt.setIntProperty(Tag::SMPP_MSG_STATE,state);
  char addr[64];
  sms.getDestinationAddress().getText(addr,sizeof(addr));
  rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
  rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));

  char msgid[60];
  sprintf(msgid,"%lld",msgId);
  rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
  //Array<SMS*> arr;
  string out;
  sms.getDestinationAddress().getText(addr,sizeof(addr));
  formatFailed(addr,reason,sms.getSubmitTime(),out);
  smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
  trimSms(prpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage);
  smsc->submitSms(prpt);
  /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
  for(int i=0;i<arr.Count();i++)
  {
    smsc->submitSms(arr[i]);
  };*/
}

void StateMachine::sendNotifyReport(SMS& sms,MsgIdType msgId,const char* reason)
{
  if(!(
        sms.getDeliveryReport() ||
        (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==1 ||
        (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==2 ||
        sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST)
      )
    )return;
  __trace2__("sendNotifyReport: attemptsCount=%d",sms.getAttemptsCount());
  if(sms.getAttemptsCount()!=0)return;
  if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))return;
  SMS *prpt=new SMS;
  SMS &rpt=*prpt;
  rpt.setOriginatingAddress(scAddress);
  char msc[]="";
  char imsi[]="";
  rpt.lastResult=sms.lastResult;
  rpt.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  rpt.setValidTime(0);
  rpt.setDeliveryReport(0);
  rpt.setArchivationRequested(false);
  rpt.setIntProperty(Tag::SMPP_ESM_CLASS,
    sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST) ||
    (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==1 ||
    (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)==2
    ?0x20:0);
  rpt.setDestinationAddress(sms.getOriginatingAddress());
  rpt.setMessageReference(sms.getMessageReference());
  rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
    sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
  rpt.setIntProperty(Tag::SMPP_MSG_STATE,ENROUTE);
  char addr[64];
  sms.getDestinationAddress().getText(addr,sizeof(addr));
  rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
  rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));

  char msgid[60];
  sprintf(msgid,"%lld",msgId);
  rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
  //Array<SMS*> arr;
  string out;
  sms.getDestinationAddress().getText(addr,sizeof(addr));
  formatNotify(addr,reason,sms.getSubmitTime(),out);
  smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
  trimSms(prpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage);
  smsc->submitSms(prpt);
  /*splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
  for(int i=0;i<arr.Count();i++)
  {
    smsc->submitSms(arr[i]);
  };*/
}

time_t StateMachine::rescheduleSms(SMS& sms)
{
  time_t nextTryTime=RescheduleCalculator::calcNextTryTime(sms.getNextTime(),sms.getAttemptsCount());
  if(nextTryTime>sms.getValidTime())nextTryTime=sms.getValidTime();
  return nextTryTime;
}


};//system
};//smsc
