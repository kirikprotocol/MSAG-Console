#include "util/debug.h"
#include "system/smsc.hpp"
#include "system/state_machine.hpp"
#include <exception>
#include "system/rescheduler.hpp"
#include "profiler/profiler.hpp"
#include "util/recoder/recode_dll.h"
#include "core/buffers/Hash.hpp"
#include "util/smstext.h"

namespace smsc{
namespace system{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace StateTypeValue;
using namespace smsc::smpp;
using namespace util;
using std::exception;

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


int StateMachine::Execute()
{
  Tuple t;
  StateType st;
  for(;;)
  {
    eq.selectAndDequeue(t,&isStopping);
    if(isStopping)break;
    switch(t.command->cmdid)
    {
      case SUBMIT:st=submit(t);break;
      case DELIVERY_RESP:st=deliveryResp(t);break;
      case FORWARD:st=forward(t);break;
      case ALERT:st=alert(t);break;
      default:
        __warning__("UNKNOWN COMMAND");
        st=ERROR_STATE;
        break;
    }
    __trace2__("change state for %lld to %d",t.msgId,st);
    eq.changeState(t.msgId,st);
  }
  __trace__("exit state machine");
  return 0;
}

StateType StateMachine::submit(Tuple& t)
{
  __require__(t.state==UNKNOWN_STATE);

  SmeProxy *src_proxy,*dest_proxy=0;

  src_proxy=t.command.getProxy();

  __trace2__("StateMachine::submit:%lld",t.msgId);

  SMS* sms = t.command->get_sms();
  uint32_t dialogId =  t.command->get_dialogId();

  if(sms->getNextTime()==-1)
  {
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::INVALIDSCHEDULE);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    __warning__("SUBMIT_SM: invalid schedule");
    return ERROR_STATE;
  }
  if(sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)!=DataCoding::DEFAULT &&
     sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)!=DataCoding::UCS2 &&
     sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)!=DataCoding::BINARY &&
     sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)!=DataCoding::SMSC7BIT)
  {
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::INVALIDDATACODING);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    __warning__("SUBMIT_SM: invalid datacoding");
    return ERROR_STATE;
  }
  if(sms->getValidTime()==-1)
  {
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::INVALIDVALIDTIME);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    __warning__("SUBMIT_SM: invalid valid time");
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

  bool has_route = smsc->routeSms(sms->getOriginatingAddress(),
                          dst,
                          dest_proxy_index,dest_proxy);
  //smsc->routeSms(sms,dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    //send_no_route;
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::NOROUTE);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    __warning__("SUBMIT_SM: no route");
    return ERROR_STATE;
  }

  if(sms->getValidTime()==0 || sms->getValidTime()>now+maxValidTime)
  {
    sms->setValidTime(now+maxValidTime);
  }

  __trace2__("Valid time for sms %lld=%u\n",t.msgId,(unsigned int)sms->getValidTime());

  if(sms->getNextTime()>now+maxValidTime || sms->getNextTime()>sms->getValidTime())
  {
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::INVALIDSCHEDULE);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    return ERROR_STATE;
  }

  __trace2__("Replace if present for message %lld=%d",t.msgId,sms->getIntProperty("SMPP_REPLACE_IF_PRESENT_FLAG"));

  time_t stime=sms->getNextTime();

  try{
    if(sms->getNextTime()<now)
    {
      sms->setNextTime(now);
    }
    store->createSms(*sms,t.msgId,
      sms->getIntProperty("SMPP_REPLACE_IF_PRESENT_FLAG")?smsc::store::SMPP_OVERWRITE_IF_PRESENT:smsc::store::CREATE_NEW);
  }catch(...)
  {
    __trace2__("failed to create sms with id %lld",t.msgId);
    SmscCommand resp = SmscCommand::makeSubmitSmResp(/*messageId*/"0", dialogId, SmscCommand::Status::DBERROR);
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
      __trace__("SUBMIT: failed to put response command");
    }
    return ERROR_STATE;
  }

  {
    // sms сохранена в базе, с выставленным Next Time, таким образом
    // даже если дальше что-то обломится, потом будет еще попытка послать её
    // то бишь мы приняли sms в обработку, можно слать ok.
    char buf[64];
    sprintf(buf,"%lld",t.msgId);
    SmscCommand resp = SmscCommand::makeSubmitSmResp(buf, dialogId, SmscCommand::Status::OK);
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
      store->changeSmsStateToEnroute(t.msgId,d,0,
        RescheduleCalculator::calcNextTryTime(sms->getNextTime(),sms->getAttemptsCount()));
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
      store->changeSmsStateToEnroute(t.msgId,d,0,
        RescheduleCalculator::calcNextTryTime(sms->getNextTime(),sms->getAttemptsCount()));
    }catch(...)
    {
      __trace__("SUBMIT_SM: failed to change state to enroute");
    }
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
      store->changeSmsStateToEnroute(t.msgId,d,0,
        RescheduleCalculator::calcNextTryTime(sms->getNextTime(),sms->getAttemptsCount()));
    }catch(...)
    {
      __trace__("SUBMIT_SM: failed to change state to enroute");
    }
    __warning__("SUBMIT_SM: can't create task");
    return ENROUTE_STATE;
  }
  }catch(...)
  {
    __trace__("SUBMIT: failed to create task");
    return ENROUTE_STATE;
  }
  try{
    // send delivery
    Address src;
    __trace2__("SUBMIT: wantAlias=%s",smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias?"true":"false");
    if(smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias && smsc->AddressToAlias(sms->getOriginatingAddress(),src))
    {
      sms->setOriginatingAddress(src);
    }
    sms->setDestinationAddress(dst);

    profile=smsc->getProfiler()->lookup(dst);
    if(profile.codepage==smsc::profiler::ProfileCharsetOptions::Default &&
       sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
    {
      __trace__("SUBMIT: converting ucs2->text");
      char buf[260];
      char buf8[260];
      unsigned len;
      const short*msg=(const short*)sms->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,&len);
      len=ConvertUCS2ToMultibyte(msg,len,buf,sizeof(buf),CONV_ENCODING_CP1251);
      int newlen=Transliterate(buf,len,CONV_ENCODING_CP1251,buf8,sizeof(buf8));
      sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf8,newlen);
      sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,newlen);
      sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
    }
    SmscCommand delivery = SmscCommand::makeDeliverySm(*sms,dialogId2);
    dest_proxy->putCommand(delivery);
  }catch(...)
  {
    __trace__("SUBMIT: failed to put delivery command");
    return ENROUTE_STATE;
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
    return UNKNOWN_STATE;
  }
  time_t now=time(NULL);
  time_t nextTryTime=RescheduleCalculator::calcNextTryTime(
    sms.getNextTime(),sms.getAttemptsCount());
  if(nextTryTime>sms.getValidTime())nextTryTime=sms.getValidTime();
  if(sms.getValidTime()<=now)
  {
    try{
      store->changeSmsStateToExpired(t.msgId);
    }catch(...)
    {
    }
    __trace2__("FORWARD: %lld expired (valid:%u - now:%u)",t.msgId,sms.getValidTime(),now);
    sendFailureReport(sms,t.msgId,"expired");
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
  SmeProxy *dest_proxy=0;
  int dest_proxy_index;

  bool has_route = smsc->routeSms(sms.getOriginatingAddress(),sms.getDealiasedDestinationAddress(),dest_proxy_index,dest_proxy);
  if ( !has_route )
  {
    __warning__("FORWARD: No route");
    try{
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,0,nextTryTime);
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
      //time_t now=time(NULL);
      Descriptor d;
      __trace__("FORWARD: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,0,nextTryTime);
    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    return ENROUTE_STATE;
  }
  // create task

  uint32_t dialogId2;
  try{
    try{
      //time_t now=time(NULL);
      /*Descriptor d;
      __trace__("FORWARD: change state to enroute");
      store->changeSmsStateToEnroute(t.msgId,d,0,
        RescheduleCalculator::calcNextTryTime(sms.getNextTime(),sms.getAttemptsCount()));*/
    }catch(...)
    {
      __trace__("FORWARD: failed to change state to enroute");
    }
    dialogId2 = dest_proxy->getNextSequenceNumber();
    __trace2__("FORWARD: seq number:%d",dialogId2);
    //Task task((uint32_t)dest_proxy_index,dialogId2);
    Task task(dest_proxy->getUniqueId(),dialogId2);
    task.messageId=t.msgId;
    if ( !smsc->tasks.createTask(task,dest_proxy->getPreferredTimeout()) )
    {
      __warning__("FORWARD: can't create task");
      return ENROUTE_STATE;
    }
  }catch(...)
  {
    return ENROUTE_STATE;
  }
  try{
    // send delivery
    Address src;
    if(smsc->getSmeInfo(dest_proxy->getIndex()).wantAlias && smsc->AddressToAlias(sms.getOriginatingAddress(),src))
    {
      sms.setOriginatingAddress(src);
    }
    Address dst=sms.getDealiasedDestinationAddress();
    sms.setDestinationAddress(dst);
    smsc::profiler::Profile p=smsc->getProfiler()->lookup(dst);
    if(p.codepage==smsc::profiler::ProfileCharsetOptions::Default &&
       sms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)==DataCoding::UCS2)
    {
      char buf[260];
      char buf8[260];
      unsigned len;
      const short*msg=(const short*)sms.getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,&len);
      len=ConvertUCS2ToMultibyte(msg,len,buf,sizeof(buf),CONV_ENCODING_CP1251);
      int newlen=Transliterate(buf,len,CONV_ENCODING_CP1251,buf8,sizeof(buf8));
      sms.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE,buf8,newlen);
      sms.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH,newlen);
      sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,DataCoding::DEFAULT);
    }
    SmscCommand delivery = SmscCommand::makeDeliverySm(sms,dialogId2);
    dest_proxy->putCommand(delivery);
  }catch(...)
  {
    //TODO!!!: remove task and reschedule
    __trace__("Failed to put delivery command");
    return ENROUTE_STATE;
  }
  return DELIVERING_STATE;
}

StateType StateMachine::deliveryResp(Tuple& t)
{
  __trace2__("delivering resp for :%lld",t.msgId);
  __require__(t.state==DELIVERING_STATE);
  smsc::sms::Descriptor d;
  SMS sms;
  try{
    store->retriveSms((SMSId)t.msgId,sms);
  }catch(exception& e)
  {
    __trace2__("failed to retrieve sms:%s",e.what());
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
          store->changeSmsStateToEnroute(t.msgId,d,0,time(NULL)+2);
        }catch(...)
        {
          __trace__("DELIVERYRESP: failed to change state to enroute");
        }
        smsc->notifyScheduler();
        return UNKNOWN_STATE;
      }break;
      case CMD_ERR_TEMP:
      {
        try{
          Descriptor d;
          __trace__("DELIVERYRESP: change state to enroute");
          store->changeSmsStateToEnroute(t.msgId,d,0,
            RescheduleCalculator::calcNextTryTime(sms.getNextTime(),sms.getAttemptsCount()));
        }catch(...)
        {
          __trace__("DELIVERYRESP: failed to change state to enroute");
        }
        smsc->notifyScheduler();
        return UNKNOWN_STATE;
      }break;
      default:
      {
        try{
          Descriptor d;
          __trace__("DELIVERYRESP: change state to undeliverable");
          store->changeSmsStateToUndeliverable(t.msgId,d,0);
        }catch(...)
        {
          __trace__("DELIVERYRESP: failed to change state to undeliverable");
        }
        if(//p.reportoptions==smsc::profiler::ProfileReportOptions::ReportFull ||
           sms.getDeliveryReport() ||
           (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==1 ||
           (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&0x3)==2 ||
           sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
        {
          sendFailureReport(sms,t.msgId,"permanent error");
        }
        return ERROR_STATE;
      }
    }
  }
  try{
    __trace__("change state to delivered");
    store->changeSmsStateToDelivered(t.msgId,d);
    __trace__("change state to delivered: ok");
  }catch(std::exception& e)
  {
    __trace2__("change state to delivered exception:%s",e.what());
    return DELIVERED_STATE;
  }
  try{
    //smsc::profiler::Profile p=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
    if(//p.reportoptions==smsc::profiler::ProfileReportOptions::ReportFull ||
       sms.getDeliveryReport() ||
       (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&1)==1  ||
       sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST))
    {
      SMS rpt;
      rpt.setOriginatingAddress(scAddress);
      char msc[]="123";
      char imsi[]="123";
      rpt.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
      rpt.setValidTime(0);
      rpt.setDeliveryReport(0);
      rpt.setArchivationRequested(false);
      rpt.setIntProperty(Tag::SMPP_ESM_CLASS,
        sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST) ||
        (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&1)==1?4:0);
      rpt.setDestinationAddress(sms.getOriginatingAddress());
      rpt.setMessageReference(sms.getMessageReference());
      rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
        sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
      rpt.setIntProperty(Tag::SMPP_MSG_STATE,sms.getState());
      char addr[64];
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
      rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));
      char msgid[60];
      sprintf(msgid,"%lld",t.msgId);
      rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
      Array<SMS*> arr;
      string out;
      sms.getDestinationAddress().getText(addr,sizeof(addr));
      formatDeliver(addr,time(NULL),out);
      rpt.getDestinationAddress().getText(addr,sizeof(addr));
      __trace2__("RECEIPT: sending receipt to %s:%s",addr,out.c_str());
      smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
      splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
      for(int i=0;i<arr.Count();i++)
      {
        smsc->submitSms(arr[i]);
      };

    }
  }catch(std::exception& e)
  {
    __trace2__("DELIVERY_RESP:failed to submit receipt");
  }
  return DELIVERED_STATE;
}

StateType StateMachine::alert(Tuple& t)
{
  __trace__("ALERT!!!");
  //time_t now=time(NULL);
  try{
    Descriptor d;
    SMS sms;
    store->retriveSms((SMSId)t.msgId,sms);
    store->changeSmsStateToEnroute(t.msgId,d,0,RescheduleCalculator::calcNextTryTime(sms.getNextTime(),sms.getAttemptsCount()));
    smsc->notifyScheduler();
  }catch(...)
  {
  };
  return UNKNOWN_STATE;
}

void StateMachine::sendFailureReport(SMS& sms,MsgIdType msgId,const char* reason)
{
  SMS rpt;
  rpt.setOriginatingAddress(scAddress);
  char msc[]="123";
  char imsi[]="123";
  rpt.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
  rpt.setValidTime(0);
  rpt.setDeliveryReport(0);
  rpt.setArchivationRequested(false);
  rpt.setIntProperty(Tag::SMPP_ESM_CLASS,
    sms.getIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST) ||
    (sms.getIntProperty(Tag::SMPP_REGISTRED_DELIVERY)&3)
    ?4:0);
  rpt.setDestinationAddress(sms.getOriginatingAddress());
  rpt.setMessageReference(sms.getMessageReference());
  rpt.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
    sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
  rpt.setIntProperty(Tag::SMPP_MSG_STATE,sms.getState());
  char addr[64];
  sms.getDestinationAddress().getText(addr,sizeof(addr));
  rpt.setStrProperty(Tag::SMSC_RECIPIENTADDRESS,addr);
  rpt.setIntProperty(Tag::SMSC_DISCHARGE_TIME,time(NULL));

  char msgid[60];
  sprintf(msgid,"%lld",msgId);
  rpt.setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,msgid);
  Array<SMS*> arr;
  string out;
  sms.getDestinationAddress().getText(addr,sizeof(addr));
  formatFailed(addr,reason,sms.getSubmitTime(),out);
  smsc::profiler::Profile profile=smsc->getProfiler()->lookup(sms.getOriginatingAddress());
  splitSms(&rpt,out.c_str(),out.length(),CONV_ENCODING_CP1251,profile.codepage,arr);
  for(int i=0;i<arr.Count();i++)
  {
    smsc->submitSms(arr[i]);
  };
}


};//system
};//smsc
