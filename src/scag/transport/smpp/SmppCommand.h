#if !defined __SCAG_SMPP_SMPPCOMMAND_HPP__
#define __SCAG_SMPP_SMPPCOMMAND_HPP__

#include "sms/sms.h"
#include "smpp/smpp_structures.h"
#include "smpp/smpp_sms.h"
#include "core/synchronization/Mutex.hpp"
#include <string.h>
#include <memory>
#include "util/int.h"
#include "smpp/smpp_time.h"
#include "util/Exception.hpp"
#include <string>
#include <vector>
#include "system/status.h"
#include "scag/transport/SCAGCommand.h"
#include "util/64bitcompat.h"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "router/route_types.h"

namespace scag{
namespace transport{
namespace smpp{

using std::auto_ptr;
using namespace smsc::smpp;
using smsc::sms::SMS;
using smsc::sms::SMSId;
using smsc::sms::Address;
using smsc::sms::Descriptor;
using smsc::core::synchronization::Mutex;
using smsc::logger::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::util::Exception;
using std::string;

const int ScagCommandDefaultPriority=16;

enum CommandId
{
  UNKNOWN,                //0
  DELIVERY,               //1
  DELIVERY_RESP,          //2
  SUBMIT,                 //3
  SUBMIT_RESP,            //4
  FORWARD,                //5
  GENERIC_NACK,           //6
  QUERY,                  //7
  QUERY_RESP,             //8
  UNBIND,                 //9
  UNBIND_RESP,            //10
  REPLACE,                //11
  REPLACE_RESP,           //12
  CANCEL,                 //13
  CANCEL_RESP,            //14
  ENQUIRELINK,            //15
  ENQUIRELINK_RESP,       //16
  SUBMIT_MULTI_SM,        //17
  SUBMIT_MULTI_SM_RESP,   //18
  BIND_TRANSCEIVER,       //19
  BIND_RECIEVER_RESP,     //20
  BIND_TRANSMITTER_RESP,  //21
  BIND_TRANCIEVER_RESP,   //22
  DATASM,                 //23
  DATASM_RESP,            //24
  PROCESSEXPIREDRESP,     //25
  ALERT_NOTIFICATION      //26
};

/*
enum CommandStatus{
  CMD_OK=0,
  CMD_ERR_TEMP=1,
  CMD_ERR_PERM=2,
  CMD_ERR_FATAL=3,
  CMD_ERR_RESCHEDULENOW=4
};
*/

enum DataSmDirection{
  dsdUnknown,
  dsdSrv2Srv,
  dsdSrv2Sc,
  dsdSc2Srv,
  dsdSc2Sc
};

/*
#define MAKE_COMMAND_STATUS(type,code) ((type<<16)|code)
#define GET_STATUS_TYPE(status) ((status>>16)&7)
#define GET_STATUS_CODE(status) (status&0x0ffff)
*/
static inline void fillField(auto_ptr<char>& field,const char* text,int length=-1)
{
  if(length==0 || text==NULL)return;
  if(length==-1)length=(int)strlen(text);
  field=auto_ptr<char>(new char[length+1]);
  memcpy(field.get(),text,length);
  field.get()[length]=0;
}

static inline void fillSmppAddr(auto_ptr<char>& field,PduAddress& addr)
{
  char buf[64];
  if(!addr.get_value() || !addr.get_value()[0])
  {
    fillField(field,NULL);
  }else
  {
    int len=sprintf(buf,".%d.%d.%.20s",
      (int)addr.get_typeOfNumber(),
      (int)addr.get_numberingPlan(),
      addr.get_value());
    fillField(field,buf,len);
  }
}

struct ReplaceSm{
  auto_ptr<char> messageId;
  auto_ptr<char> sourceAddr;
  time_t scheduleDeliveryTime;
  time_t validityPeriod;
  int registeredDelivery;
  int smDefaultMsgId;
  int smLength;
  auto_ptr<char> shortMessage;

  SMSId getMessageId()const
  {
    SMSId id=0;
    if(messageId.get())sscanf(messageId.get(),"%lld",&id);
    return id;
  }

  ReplaceSm(PduReplaceSm* repl)
  {
    fillField(messageId,repl->get_messageId());
    if(!messageId.get() || !messageId.get()[0])
    {
      throw Exception("REPLACE: non empty messageId required");
    }
    fillSmppAddr(sourceAddr,repl->get_source());
    scheduleDeliveryTime=
      repl->scheduleDeliveryTime.size()?smppTime2CTime(repl->scheduleDeliveryTime):0;
    validityPeriod=
      repl->validityPeriod?smppTime2CTime(repl->validityPeriod):0;
    registeredDelivery=repl->get_registredDelivery();
    smDefaultMsgId=repl->get_smDefaultMsgId();
    smLength=repl->shortMessage.size();
    fillField(shortMessage,repl->shortMessage.cstr(),smLength);
  }
};

struct QuerySm{
  auto_ptr<char> messageId;
  auto_ptr<char> sourceAddr;
  QuerySm(PduQuerySm* q)
  {
    fillField(messageId,q->get_messageId());
    if(!messageId.get())
    {
      messageId=auto_ptr<char>(new char[1]);
      messageId.get()[0]=0;
    }
    fillSmppAddr(sourceAddr,q->get_source());
  }
  SMSId getMessageId()const
  {
    SMSId id=0;
    if(messageId.get())sscanf(messageId.get(),"%lld",&id);
    return id;
  }
};

struct QuerySmResp{
  auto_ptr<char> messageId;
  auto_ptr<char> finalDate;
  uint8_t        messageState;
  uint8_t        networkCode;
  int            commandStatus;
  QuerySmResp(int cmdStatus,SMSId id,time_t findate,int state,int netcode)
  {
    commandStatus=cmdStatus;
    messageId=auto_ptr<char>(new char[22]);
    sprintf(messageId.get(),"%lld",id);
    if(findate==0)
    {
      finalDate=auto_ptr<char>(new char[1]);
      finalDate.get()[0]=0;
    }else
    {
      finalDate=auto_ptr<char>(new char[18]);
      cTime2SmppTime(findate,finalDate.get());
    }
    messageState=(uint8_t)state;
    networkCode=(uint8_t)netcode;
  }
  void fillPdu(PduQuerySmResp* pdu)
  {
    pdu->set_messageId(messageId.get());
    pdu->set_finalDate(finalDate.get());
    pdu->set_messageState(messageState);
    pdu->set_errorCode(networkCode);
  }
};

struct CancelSm{
  auto_ptr<char> serviceType;
  auto_ptr<char> messageId;
  auto_ptr<char> sourceAddr;
  auto_ptr<char> destAddr;
  bool internall;
  bool force;

  CancelSm(PduCancelSm* pdu)
  {
    fillField(serviceType,pdu->get_serviceType());
    fillField(messageId,pdu->get_messageId());
    fillSmppAddr(sourceAddr,pdu->get_source());
    fillSmppAddr(destAddr,pdu->get_dest());
    internall=false;
    force=false;
  }
  CancelSm(SMSId id,const Address& oa,const Address& da)
  {
    char oabuf[32];
    char dabuf[32];
    int oalen=oa.toString(oabuf,sizeof(oabuf));
    int dalen=da.toString(dabuf,sizeof(dabuf));
    fillField(sourceAddr,oabuf,oalen);
    fillField(destAddr,dabuf,dalen);
    char idbuf[32];
    int idlen=sprintf(idbuf,"%lld",id);
    fillField(messageId,idbuf,idlen);
    internall=true;
    force=false;
  }

  CancelSm(SMSId id)
  {
    char idbuf[32];
    int idlen=sprintf(idbuf,"%lld",id);
    fillField(messageId,idbuf,idlen);
    internall=true;
    force=true;
  }

  SMSId getMessageId()const
  {
    SMSId id=0;
    if(messageId.get())sscanf(messageId.get(),"%lld",&id);
    return id;
  }
};

struct DlElement{
  unsigned dest_flag : 1;
  unsigned __  : 7;
  unsigned ton : 8;
  unsigned npi : 8;
  string value;
};

struct UnsuccessElement{
    Address addr;
    uint32_t errcode;
};

struct SubmitMultiSm{
  SMS msg;
  uint8_t number_of_dests;
  DlElement dests[255];
  //SubmitMultiSm(const SMS& sms){}
  //~SubmitMultiSm(){}
};

struct SubmitMultiResp{
  //SMS msg;
  char* messageId;
  uint32_t status;
  uint8_t no_unsuccess;
  UnsuccessElement unsuccess[255];
public:
  void set_messageId(const char* msgid)
  {
    if(!msgid)return;
    if ( messageId ) delete( messageId);
    messageId = new char[strlen(msgid)+1];
    strcpy(messageId,msgid);
  }
  void set_status(uint32_t st) { status = st; }
  const char* get_messageId() const {return messageId;}
  UnsuccessElement* get_unsuccess() { return unsuccess; }
  void set_unsuccessCount(unsigned c) { no_unsuccess = c; }
  uint32_t get_status() { return status; }
  SubmitMultiResp() : messageId(0), status(0) {};
  ~SubmitMultiResp() { if ( messageId ) delete messageId; }
};


struct BindCommand{
  std::string sysId;
  std::string pass;
  std::string addrRange;
  std::string systemType;
  BindCommand(const char* id,const char* pwd,const char* rng,const char* st):sysId(id),pass(pwd),addrRange(rng),systemType(st){}
};

struct AlertNotification{
  AlertNotification(PduAlertNotification* pdu)
  {
    src=PduAddress2Address(pdu->get_source());
    dst=PduAddress2Address(pdu->get_esme());
    status=pdu->optional.get_msAvailableStatus();
  }
  Address src;
  Address dst;
  int status;

};

struct SmsCommand{
  SmsCommand():dir(dsdUnknown),original_ussd_op(-1) {}
  SmsCommand(const SMS& sms):sms(sms),dir(dsdUnknown),original_ussd_op(-1) {}
  SMS sms;
  Address orgSrc,orgDst;
  DataSmDirection dir;
  int original_ussd_op;
};

namespace SmppCommandFlags{
    const uint8_t NOTIFICATION_RECEIPT = 1;
    const uint8_t SERVICE_INITIATED_USSD_DIALOG = 2;
    const uint8_t EXPIRED_COMMAND = 4;
    const uint8_t FAILED_COMMAND_RESP = 8;
};

struct SmppEntity;
struct SmsResp;

struct _SmppCommand
{
  mutable int ref_count;
  CommandId cmdid;
  uint32_t dialogId;
  uint32_t orgDialogId;
  void* dta;
  int status;
  Mutex mutex;
  SmppEntity *ent, *dst_ent;
  int serviceId;
  int priority;
  uint64_t opId;
  SessionPtr session;
  uint16_t usr;
  uint32_t flags;
  uint32_t sliceCount;
  uint8_t slicingRespPolicy;
  bool slicedRespSent;
  Mutex    slicedMutex;

  static Logger* logger;
  static Mutex loggerMutex;

  static uint32_t commandCounter; // for debugging

  static Mutex    cntMutex;
  static uint32_t stuid;
  uint32_t uid;

  ~_SmppCommand();

  _SmppCommand() : ref_count(0), dta(0), ent(0), dst_ent(0), status(0),priority(ScagCommandDefaultPriority), usr(0), flags(0), opId(-1), sliceCount(1), slicedRespSent(false)
  {
    if(!logger)
    {
        MutexGuard mt(loggerMutex);
        if(!logger) logger = Logger::getInstance("smppMan");
    }

    if(logger->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG))
    {
        uint32_t sc = 0;
        {
            MutexGuard mtxx(cntMutex);
            sc = ++commandCounter;
            uid = ++stuid;
        }
        smsc_log_debug(logger, "Command create: count=%d, addr=%s, usr=%d, uid=%d", sc, session.Get() ? session->getSessionKey().abonentAddr.toString().c_str() : "", session.Get() ?  session->getSessionKey().USR : 0, uid);
    }
  }

  uint32_t get_dialogId() const { return dialogId; }
  void set_dialogId(uint32_t dlgId) { dialogId=dlgId; }

  uint32_t get_orgDialogId() const { return orgDialogId; }
  void set_orgDialogId(uint32_t dlgId) { orgDialogId=dlgId; }

  void setSlicingParams(uint8_t srp, uint32_t cnt) { slicingRespPolicy = srp; sliceCount=cnt; };

  bool essentialSlicedResponse(bool failed)
  {
    MutexGuard mg(slicedMutex);

    sliceCount--;

    if(!slicedRespSent && (failed || slicingRespPolicy == router::SlicingRespPolicy::ANY || !sliceCount))
        return slicedRespSent = true;

    return false;
  }

  CommandId get_commandId() const { return cmdid; }

  SMS* get_sms()
  {
    __require__(cmdid==SUBMIT || cmdid==DELIVERY || cmdid==DATASM);
    return &((SmsCommand*)dta)->sms;
  }
  SmsCommand& get_smsCommand()
  {
    __require__(cmdid==SUBMIT || cmdid==DELIVERY || cmdid==DATASM);
    return *((SmsCommand*)dta);
  }
  const ReplaceSm& get_replaceSm(){return *(ReplaceSm*)dta;}
  const QuerySm& get_querySm(){return *(QuerySm*)dta;}
  const CancelSm& get_cancelSm(){return *(CancelSm*)dta;}
  SmsResp* get_resp() const
  {
    __require__(cmdid==DELIVERY_RESP || cmdid==SUBMIT_RESP || cmdid==DATASM_RESP);
    return (SmsResp*)dta;
  }
  uint32_t getCommandStatus() const;
  SubmitMultiResp* get_MultiResp() { return (SubmitMultiResp*)dta;}
  SubmitMultiSm* get_Multi() { return (SubmitMultiSm*)dta;}
  int get_priority(){return priority;};
  void set_priority(int newprio){priority=newprio;}

  const Address& get_address() { return *(Address*)dta; }
  void set_address(const Address& addr) { *(Address*)dta = addr; }

  //SmppHeader* get_smppPdu(){return (SmppHeader*)dta;}

  BindCommand& get_bindCommand(){return *((BindCommand*)dta);}

  AlertNotification& get_alertNotification()
  {
    return *((AlertNotification*)dta);
  }

  int get_smeIndex(){return VoidPtr2Int(dta);}

  void set_status(int st){status=st;}
  int get_status(){return status;} // for enquirelink and unbind

  int get_mode()
  {
    return VoidPtr2Int(dta);
  }

  void set_serviceId(int argServiceId)
  {
    serviceId=argServiceId;
  }
  int get_serviceId()
  {
    return serviceId;
  }
  void set_operationId(uint64_t id)
  {
    opId=id;
  }
  uint64_t get_operationId()
  {
    return opId;
  }

  void setFlag(uint32_t f) { flags |= f; }
  bool flagSet(uint32_t f) { return flags & f; }
};

class SmppCommand:public SCAGCommand
{
  _SmppCommand* cmd;
  static void unref(_SmppCommand*& cmd)
  {
    __require__ ( cmd != 0 );
    int count;
    {
      MutexGuard guard(cmd->mutex);
      __require__ ( cmd->ref_count > 0 );
      cmd->ref_count--;
      count=cmd->ref_count;
    }
    if ( count == 0 )
    {
      delete cmd;
      cmd = 0;
    }
  }

  void dispose()
    {
      if (cmd) unref(cmd);
    }

public:

  static _SmppCommand* ref(_SmppCommand* cmd)
  {
    if ( !cmd )
    {
      return 0;
    }
    MutexGuard guard(cmd->mutex);
    __require__ ( cmd->ref_count >= 0 );
    ++(cmd->ref_count);
    return cmd;
  }

  SmppEntity* getEntity()const{return cmd->ent;}
  void setEntity(SmppEntity* newent){cmd->ent=newent;}
  SmppEntity* getDstEntity()const{return cmd->dst_ent;}
  void setDstEntity(SmppEntity* newent){cmd->dst_ent=newent;}

  ~SmppCommand() {
     dispose();
  }

  SmppCommand() : cmd (0){}
  SmppCommand(SmppHeader* pdu,bool forceDC=false);

  uint32_t makeSmppStatus(uint32_t status)
  {
    return status;
  }

  SmppHeader* makePdu(bool forceDC=false);

  // specialized constructors (meta constructors)
  static SmppCommand makeCommandSm(CommandId command, const SMS& sms,uint32_t dialogId);
  static SmppCommand makeSubmitSm(const SMS& sms,uint32_t dialogId);
  static SmppCommand makeDeliverySm(const SMS& sms,uint32_t dialogId);
  static SmppCommand makeDataSm(const SMS& sms,uint32_t dialogId);
  static SmppCommand makeSubmitSmResp(const char* messageId, uint32_t dialogId, uint32_t status,bool dataSm=false);
  static SmppCommand makeDataSmResp(const char* messageId, uint32_t dialogId, uint32_t status);
  static SmppCommand makeSubmitMultiResp(const char* messageId, uint32_t dialogId, uint32_t status);
  static SmppCommand makeDeliverySmResp(const char* messageId, uint32_t dialogId, uint32_t status);
  static SmppCommand makeGenericNack(uint32_t dialogId,uint32_t status);
  static SmppCommand makeUnbind(int dialogId,int mode=0);
  static SmppCommand makeUnbindResp(uint32_t dialogId,uint32_t status,void* data);
  static SmppCommand makeReplaceSmResp(uint32_t dialogId,uint32_t status);
  static SmppCommand makeQuerySmResp(uint32_t dialogId,uint32_t status,
            SMSId id,time_t findate,uint8_t state,uint8_t netcode);
  static SmppCommand makeCancelSmResp(uint32_t dialogId,uint32_t status);
  static SmppCommand makeCancel(SMSId id,const Address& oa,const Address& da);
  static SmppCommand makeCancel(SMSId id);
  static SmppCommand makeBindCommand(const char* sysId,const char* pwd,const char* addrRange,const char* sysType);
  static SmppCommand makeCommand(CommandId cmdId,uint32_t dialogId,uint32_t status,void* data);
  static void makeSMSBody(SMS* sms,const SmppHeader* pdu,bool forceDC);

  SmppCommand(const SmppCommand& _cmd)
  {
    cmd = ref(_cmd.cmd);
  }

  const SmppCommand& operator = (const SmppCommand& _cmd)
  {
    if (cmd) unref(cmd);
    cmd = ref(_cmd.cmd);
    return _cmd;
  }

  _SmppCommand* operator->() const
  {
    __require__(cmd);
    return cmd;
  }

  TransportType getType()const
  {
    return SMPP;
  }

  int getServiceId()const
  {
    return cmd->get_serviceId();
  }

  void setServiceId(int serviceId)
  {
    cmd->set_serviceId(serviceId);
  }

  int64_t getOperationId() const
  {
    return cmd->get_operationId();
  }

  void setOperationId(int64_t op)
  {
    cmd->set_operationId(op);
  }

   uint8_t getCommandId() const
   {
       return cmd->cmdid;
   }

   uint16_t getUSR()
   {
       return cmd->usr;
   }

   void setUSR(uint16_t usr)
   {
       cmd->usr = usr;
   }

   SessionPtr getSession() { return cmd->session; };
   void setSession(SessionPtr& s) { cmd->session = s; };
   bool hasSession() { return cmd->session.Get(); }

};

struct SmsResp
{
private:
  char* messageId;
  uint32_t status;
  bool dataSm;
  SMS* sms;
  DataSmDirection dir;
  SmppCommand orgCmd;
  bool bHasOrgCmd;

  bool bHasDeliveryFailureReason;
  uint8_t deliveryFailureReason;
  bool bHasAdditionalStatusInfoText;
  string additionalStatusInfoText;
  bool bHasDpfResult;
  uint8_t dpfResult;
  bool bHasNetworkErrorCode;
  uint32_t networkErrorCode;
  Logger* logger;

public:

  SmsResp();
  ~SmsResp();

  void set_dir(DataSmDirection direction)
  {
      dir = direction;
  }

  DataSmDirection get_dir()
  {
      return dir;
  }

  void set_messageId(const char* msgid)
  {
    if(!msgid)return;
    if ( messageId ) delete( messageId);
    messageId = new char[strlen(msgid)+1];
    strcpy(messageId,msgid);
  }
  void set_status(uint32_t st)
  {
    status = st;
  }

  void setDeliveryFailureReason(uint8_t reason) {
    if (reason < 4) {
      bHasDeliveryFailureReason  = true;
      deliveryFailureReason = reason;
    } else {
      smsc_log_warn(logger, "error value for 'delivery_failure_reason' tag: %d. should be >=0 & <=3", reason);
    }
  }

  uint8_t getDeliveryFailureReason() const {
    return deliveryFailureReason;
  }

  void setAdditionalStatusInfoText(const char* info) {
    if (strlen(info) <= 255) {
      bHasAdditionalStatusInfoText = true;
      additionalStatusInfoText = info;
    } else {
      smsc_log_warn(logger, "to long string (should be <= 255) for 'additional_status_info_text' tag: '%s'", info);
    }
  }

  const char* getAdditionalStatusInfoText() const {
    return additionalStatusInfoText.c_str();
  }

  void setDpfResult(uint8_t result) {
    if (result < 2) {
      bHasDpfResult = true;
      dpfResult = result;
    } else {
      smsc_log_warn(logger, "error value for 'dpf_result' tag: %d. should be 0 or 1", result);
    }
  }

  uint8_t getDpfResult() const {
    return dpfResult;
  }

  bool hasAdditionalStatusInfoText() const {
    return bHasAdditionalStatusInfoText;
  }

  bool hasDeliveryFailureReason() const {
    return bHasDeliveryFailureReason;
  }

  bool hasDpfResult() const {
    return bHasDpfResult;
  }

  bool hasNetworkErrorCode() const {
    return bHasNetworkErrorCode;
  }

  void setNetworkErrorCode(uint32_t code) {
    bHasNetworkErrorCode = true;
    networkErrorCode = code;
  }

  uint32_t getNetworkErrorCode() const {
    return networkErrorCode;
  }

  const char* get_messageId()const
  {
    return messageId?messageId:"";
  }

  uint32_t get_status()
  {
    return status;
  }

  void set_dataSm()
  {
    dataSm=true;
  }
  bool get_dataSm() const
  {
    return dataSm;
  }

  void set_sms(SMS* s)
  {
    sms=s;
  }
  SMS* get_sms() const
  {
    return sms;
  }

  void setOrgCmd(SmppCommand& o);
  void getOrgCmd(SmppCommand& o);
  bool hasOrgCmd() { return bHasOrgCmd; };

  int expiredUid;
  bool expiredResp;

};

} //smpp
} //transport
} //scag

#endif
