#if !defined __Cpp_Header__smsccmd_h__
#define __Cpp_Header__smsccmd_h__

#ifndef __GNUC__
#ident "@(#)$Id$"
#endif

/*
��� ���������� ���� ��������, ��� ��,
����� ������������ � ����������� ��� ������ _SmscCommand
� ����������� ��������� ������� : ��� �������� ������ �� Smpp/Map PDU
� �������� PDU �� ������
*/

#include "../sms/sms.h"
#include "../smpp/smpp_structures.h"
#include "../smpp/smpp_sms.h"
#include "core/synchronization/Mutex.hpp"
#include <string.h>
#include <memory>
#include <inttypes.h>
#include "smeman/smetypes.h"
#include "smpp/smpp_time.h"
#include "util/Exception.hpp"
#include <string>
#include <vector>
#include "system/status.h"
#include "router/route_types.h"

namespace smsc{
namespace smeman{

using std::auto_ptr;
using std::vector;
using namespace smsc::smpp;
using smsc::sms::SMS;
using smsc::sms::SMSId;
using smsc::sms::Address;
using smsc::sms::Descriptor;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::util::Exception;
using std::string;

const int SmscCommandDefaultPriority=16;

enum CommandId
{
  UNKNOWN,                //0
  DELIVERY,               //1
  DELIVERY_RESP,          //2
  SUBMIT,                 //3
  SUBMIT_RESP,            //4
  FORWARD,                //5
  ALERT,                  //6
  GENERIC_NACK,           //7
  QUERY,                  //8
  QUERY_RESP,             //9
  UNBIND,                 //10
  UNBIND_RESP,            //11
  REPLACE,                //12
  REPLACE_RESP,           //13
  CANCEL,                 //14
  CANCEL_RESP,            //15
  HLRALERT,               //16
  ENQUIRELINK,            //17
  ENQUIRELINK_RESP,       //18
  QUERYABONENTSTATUS,     //19
  QUERYABONENTSTATUS_RESP,//20
  SMPP_PDU,               //21
  SUBMIT_MULTI_SM,        //22
  SUBMIT_MULTI_SM_RESP,   //23
  ALERT_NOTIFICATION,     //24
  SMEALERT,               //25
  KILLMRCACHEITEM,        //26
  KILLEXPIREDTRANSACTIONS,//27
  INSMSCHARGERESPONSE,    //28
  INFWDSMSCHARGERESPONSE  //29
};

enum CommandStatus{
  CMD_OK=0,
  CMD_ERR_TEMP=1,
  CMD_ERR_PERM=2,
  CMD_ERR_FATAL=2,
  CMD_ERR_RESCHEDULENOW=4
};

#define MAKE_COMMAND_STATUS(type,code) ((type<<16)|code)
#define GET_STATUS_TYPE(status) ((status>>16)&7)
#define GET_STATUS_CODE(status) (status&0x0ffff)

struct AbonentStatus{
  Address addr;
  Address sourceAddr;
  Address destAddr;
  int     status;
  int     userMessageReference;
  string  originalAddr;
  bool isMobileRequest;
  string msc;
  string imsi;

  AbonentStatus(const Address& addr):
    addr(addr),
    status(0),
    userMessageReference(0),
    isMobileRequest(false)
  {
  }
  AbonentStatus(const AbonentStatus& as,int status,const string& _msc,const string& _imsi):
    addr(as.addr),
    sourceAddr(as.sourceAddr),
    destAddr(as.destAddr),
    status(status),
    userMessageReference(as.userMessageReference),
    originalAddr(as.originalAddr),
    isMobileRequest(as.isMobileRequest),
    msc(_msc),imsi(_imsi)
  {
  }
  static const int OFFLINE=0;
  static const int ONLINE=1;
  static const int UNKNOWNVALUE=2;
};

struct AlertNotification{
  Address src;
  Address dst;
  int status;
};

struct ForwardData{
  ForwardData(SMSId id,bool reschedule):id(id),reschedule(reschedule),allowDivert(false){}
  SMSId id;
  bool reschedule;
  bool allowDivert;
};

struct SmsResp
{
private:
  char* messageId;
  uint32_t status;
  Descriptor descriptor;
  bool dataSm;
  int delay;
  SMS* sms;
  bool diverted;
  int inDlgId;
public:

  bool haveDpf;
  int dpfResult;
  bool haveDeliveryFailureReason;
  uint8_t deliveryFailureReason;
  bool haveNetworkErrorCode;
  uint8_t networkErrorCode[3];
  std::string additionalStatusInfoText;
  int ussd_session_id;


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

  const char* get_messageId()
  {
    return messageId;
  }

  uint32_t get_status()
  {
    return status;
  }
  void setDescriptor(const Descriptor& dsc)
  {
    descriptor=dsc;
  }
  const Descriptor& getDescriptor()
  {
    return descriptor;
  }
  void set_delay(int newdelay)
  {
    delay=newdelay;
  }
  int get_delay()
  {
    return delay;
  }
  void set_dataSm()
  {
    dataSm=true;
  }
  bool get_dataSm()
  {
    return dataSm;
  }

  void set_sms(SMS* s)
  {
    sms=s;
  }
  SMS* get_sms()
  {
    return sms;
  }

  bool get_diverted()
  {
    return diverted;
  }

  void set_diverted(bool val)
  {
    diverted=val;
  }


  void set_inDlgId(int dlgId)
  {
    inDlgId=dlgId;
  }
  int get_inDlgId()const
  {
    return inDlgId;
  }

  SmsResp() : messageId(0), status(0),dataSm(false),delay(-1),sms(0),diverted(false),
    haveDpf(false),dpfResult(0),inDlgId(0),
    haveDeliveryFailureReason(false),deliveryFailureReason(0),
    haveNetworkErrorCode(false),ussd_session_id(-1)
    {};
  ~SmsResp()
  {
    if ( messageId ) delete messageId;
    if (sms)delete sms;
  }
};

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
    int oalen=oa.toString(oabuf,(int)sizeof(oabuf));
    int dalen=da.toString(dabuf,(int)sizeof(dabuf));
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
  const char* get_messageId() {return messageId;}
  UnsuccessElement* get_unsuccess() { return unsuccess; }
  void set_unsuccessCount(unsigned c) { no_unsuccess = c; }
  uint32_t get_status() { return status; }
  SubmitMultiResp() : messageId(0), status(0) {};
  ~SubmitMultiResp() { if ( messageId ) delete messageId; }
};

struct KillMrCacheItemCmd{
  Address org;
  Address dst;
  uint16_t mr;
};

class SmeProxy;

struct INSmsChargeResponse{
  SMSId id;
  SMS sms;
  int result;
  int contractType;
  std::string inmanError;
  struct SubmitContext{
    SmeProxy* srcProxy;
    SmeProxy* dstProxy;
    int createSms;
    bool needToSendResp;
    int dialogId;
    int dest_proxy_index;
    bool isForwardTo;
    bool diverted;
    bool routeHide;
    bool generateDeliver;
    Address dst;
#ifdef SMSEXTRA
    bool noDestChange;
#endif
    bool transit;
    smsc::router::ReplyPath replyPath;
    int priority;

    int inDlgId;
  };

  SubmitContext cntx;
};

struct INFwdSmsChargeResponse{
  SMSId id;
  SMS sms;
  int result;
  std::string inmanError;
  struct ForwardContext{
    bool allowDivert;
    bool reschedulingForward;
    int inDlgId;
  };
  ForwardContext cntx;
};


struct AlertData{
  SMS* sms;
  int inDlgId;
  AlertData(SMS* argSms,int argInDlgId):sms(argSms),inDlgId(argInDlgId){}
  ~AlertData()
  {
    if(sms)delete sms;
  }
};

struct _SmscCommand
{
  mutable int ref_count;
  CommandId cmdid;
  uint32_t dialogId;
  void* dta;
  int intData;
  int status;
  Mutex mutex;
  SmeProxy *proxy;
  int priority;
  SmeSystemId sourceId;
  _SmscCommand() : ref_count(0), dta(0), proxy(0),priority(SmscCommandDefaultPriority){};
  ~_SmscCommand()
  {
    switch ( cmdid )
    {
    case DELIVERY:
    case SUBMIT:
      if(dta)delete (SMS*)dta;
      break;

    case ALERT:
      if(dta)delete ( (AlertData*)dta );
      break;

    case SUBMIT_MULTI_SM:
      delete ( (SubmitMultiSm*)dta );
      break;
    case SUBMIT_MULTI_SM_RESP:
      delete ( (SubmitMultiResp*)dta );
      break;

    case DELIVERY_RESP:
    case SUBMIT_RESP:
      delete ( (SmsResp*)dta );
      break;

    case REPLACE:
      delete ( (ReplaceSm*)dta);
      break;

    case QUERY:
      delete ( (QuerySm*)dta);
      break;
    case QUERY_RESP:
      delete ((QuerySmResp*)dta);
      break;
    case CANCEL:
      delete ( (CancelSm*)dta);
      break;

    case HLRALERT:
      delete ( (Address*)dta);
      break;
    case QUERYABONENTSTATUS:
    case QUERYABONENTSTATUS_RESP:
      delete ( (AbonentStatus*)dta);
      break;
    case SMPP_PDU:
      if(dta)disposePdu((SmppHeader*)dta);
      break;
    case ALERT_NOTIFICATION:
      if(dta)delete (AlertNotification*)dta;
      break;
    case FORWARD:
      if(dta)delete (ForwardData*)dta;
      break;
    case KILLMRCACHEITEM:
      if(dta)delete (KillMrCacheItemCmd*)dta;
      break;
    case INSMSCHARGERESPONSE:
      if(dta)delete (INSmsChargeResponse*)dta;
      break;
    case INFWDSMSCHARGERESPONSE:
      if(dta)delete (INFwdSmsChargeResponse*)dta;
      break;
    case UNKNOWN:
    case GENERIC_NACK:
    case UNBIND_RESP:
    case REPLACE_RESP:
    case CANCEL_RESP:
    case ENQUIRELINK:
    case ENQUIRELINK_RESP:
    case SMEALERT:
    case KILLEXPIREDTRANSACTIONS:
    case UNBIND:
      // nothing to delete
      break;
    default:
      __warning2__("~SmscCommand:unprocessed cmdid %d",cmdid);
    }
  }

  uint32_t get_dialogId() const { return dialogId; }
  void set_dialogId(uint32_t dlgId) { dialogId=dlgId; }
  CommandId get_commandId() const { return cmdid; }
  SMS* get_sms() const { return (SMS*)dta; }
  SMS* get_sms_and_forget() { SMS* s = (SMS*)dta; dta = 0; return s;}
  const AlertData& get_alertData(){return *(AlertData*)dta;}
  const ReplaceSm& get_replaceSm(){return *(ReplaceSm*)dta;}
  const QuerySm& get_querySm(){return *(QuerySm*)dta;}
  const CancelSm& get_cancelSm(){return *(CancelSm*)dta;}
  SmsResp* get_resp() const { return (SmsResp*)dta; }
  SubmitMultiResp* get_MultiResp() { return (SubmitMultiResp*)dta;}
  SubmitMultiSm* get_Multi() { return (SubmitMultiSm*)dta;}
  int get_priority(){return priority;};
  void set_priority(int newprio){priority=newprio;}
  const char* get_sourceId(){return sourceId.c_str();}
  const Address& get_address() { return *(Address*)dta; }
  void set_address(const Address& addr) { *(Address*)dta = addr; }

  SmppHeader* get_smppPdu(){return (SmppHeader*)dta;}

  int get_smeIndex(){return intData;}

  AlertNotification& get_alertNotification(){return *(AlertNotification*)dta;}

  AbonentStatus& get_abonentStatus()
  {
    return *((AbonentStatus*)dta);
  }

  bool is_reschedulingForward(){return ((ForwardData*)dta)->reschedule;}
  SMSId get_forwardMsgId(){return ((ForwardData*)dta)->id;}

  bool get_forwardAllowDivert(){return ((ForwardData*)dta)->allowDivert;}
  void set_forwardAllowDivert(bool val){((ForwardData*)dta)->allowDivert=val;}

  void set_status(int st){status=st;}
  int get_status(){return status;} // for enquirelink and unbind

  KillMrCacheItemCmd& get_KillMrCacheItem()
  {
    return *((KillMrCacheItemCmd*)dta);
  }

  int get_mode()
  {
    return (int)intData;
  }

  INSmsChargeResponse* get_chargeSmsResp()
  {
    return (INSmsChargeResponse*)dta;
  }
  INFwdSmsChargeResponse* get_fwdChargeSmsResp()
  {
    return (INFwdSmsChargeResponse*)dta;
  }
};

class SmscCommand
{
  _SmscCommand* cmd;
  static void unref(_SmscCommand*& cmd)
  {
    //__trace__(__PRETTY_FUNCTION__);
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

  static _SmscCommand* ref(_SmscCommand* cmd)
  {
    //__trace__(__PRETTY_FUNCTION__);
    //__require__ ( cmd != 0 );
    if ( !cmd )
    {
      //__warning__("cmd is zero");
      return 0;
    }
    MutexGuard guard(cmd->mutex);
    __require__ ( cmd->ref_count >= 0 );
    ++(cmd->ref_count);
    return cmd;
  }

  /*void copy(const _SmscCommand* _cmd)
  {
    __trace__(__PRETTY_FUNCTION__);
    if ( cmd ) unref(cmd);
    cmd = ref(const_cast<_SmscCommand*>(_cmd));
  }*/


  void dispose() // for debuging ;(
    {
      //__trace__(__PRETTY_FUNCTION__);
      if (cmd) unref(cmd);
    }

public:

  /*
  struct Status
  {
    static const int OK       = 0;
    static const int SYSERROR = 1;
    static const int INVSRC   = 2;
    static const int INVDST   = 3;
    static const int NOROUTE  = 4;
    static const int DBERROR  = 5;
    static const int INVALIDSCHEDULE   = 6;
    static const int INVALIDVALIDTIME  = 7;
    static const int INVALIDDATACODING = 8;
    static const int REPLACEFAIL       = 9;
    static const int QUERYFAIL         = 10;
    static const int CANCELFAIL        = 11;
    static const int INVALIDBINDSTATE  = 12;
    static const int INVALIDCMDID      = 13;
    static const int INVALIDMSGID      = 14;
  };
  */

  SmeProxy* getProxy()const{return cmd->proxy;}
  void setProxy(SmeProxy* newproxy){cmd->proxy=newproxy;}
  bool IsOk() const { return cmd != 0; }
  // specialized constructors (meta constructors)
  static SmscCommand makeSumbmitSm(const SMS& sms,uint32_t dialogId)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = SUBMIT;
    _cmd.dta = new SMS;
    *_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }

  /*static SmscCommand makeMultiSubmitSm(const SMS& sms,uint32_t dialogId)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = SUBMIT;
    _cmd.dta = new SMS;
    *_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }*/

  static SmscCommand makeDeliverySm(const SMS& sms,uint32_t dialogId)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = DELIVERY;
    _cmd.dta = new SMS;
    *_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }

  static SmscCommand makeSubmitSmResp(const char* messageId, uint32_t dialogId, uint32_t status,bool dataSm=false)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = SUBMIT_RESP;
    _cmd.dta = new SmsResp;
    _cmd.get_resp()->set_messageId(messageId);
    _cmd.get_resp()->set_status(status);
    if(dataSm)_cmd.get_resp()->set_dataSm();
    _cmd.dialogId = dialogId;
    return cmd;
  }

  static SmscCommand makeSubmitMultiResp(const char* messageId, uint32_t dialogId, uint32_t status)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = SUBMIT_MULTI_SM_RESP;
    _cmd.dta = new SubmitMultiResp;
    _cmd.get_MultiResp ()->set_messageId(messageId?messageId:"");
    _cmd.get_MultiResp()->set_status(status);
    _cmd.dialogId = dialogId;
    _cmd.get_MultiResp()->set_unsuccessCount(0);
    return cmd;
  }

  static SmscCommand makeDeliverySmResp(const char* messageId, uint32_t dialogId, uint32_t status)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = DELIVERY_RESP;
    _cmd.dta = new SmsResp;
    _cmd.get_resp()->set_messageId(messageId);
    _cmd.get_resp()->set_status(status);
    _cmd.dialogId = dialogId;
    return cmd;
  }

  static SmscCommand makeAlert(SMS *sms,int inDlgId)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = ALERT;
    _cmd.dta = new AlertData(sms,inDlgId);
    _cmd.dialogId = 0;
    return cmd;
  }

  static SmscCommand makeHLRAlert(const Address& addr)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = HLRALERT;
    _cmd.dta = new Address();
    _cmd.set_address(addr);
    _cmd.dialogId = 0;
    return cmd;
  }

  static SmscCommand makeSMEAlert(int idx)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = SMEALERT;
    _cmd.intData = idx;
    _cmd.dialogId = 0;
    return cmd;
  }

  static SmscCommand makeForward(SMSId id,bool reschedule=false)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = FORWARD;
    _cmd.dta = new ForwardData(id,reschedule);
    _cmd.dialogId = 0;
    return cmd;
  }

  static SmscCommand makeGenericNack(uint32_t dialogId,uint32_t status)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=GENERIC_NACK;
    _cmd.status=status;
    _cmd.dialogId=dialogId;
    return cmd;
  }


  static SmscCommand makeUnbind(int dialogId,int mode=0)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=UNBIND;
    _cmd.intData=mode;
    _cmd.status=0;
    _cmd.dialogId=dialogId;
    return cmd;
  }

  static SmscCommand makeUnbindResp(uint32_t dialogId,uint32_t status,int data)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=UNBIND_RESP;
    _cmd.intData=data;
    _cmd.status=status;
    _cmd.dialogId=dialogId;
    return cmd;
  }

  static SmscCommand makeReplaceSmResp(uint32_t dialogId,uint32_t status)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=REPLACE_RESP;
    _cmd.status=status;
    _cmd.dialogId=dialogId;
    return cmd;
  }

  static SmscCommand makeQuerySmResp(uint32_t dialogId,uint32_t status,
            SMSId id,time_t findate,uint8_t state,uint8_t netcode)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=QUERY_RESP;
    _cmd.dta=new QuerySmResp(status,id,findate,state,netcode);
    _cmd.dialogId=dialogId;
    return cmd;
  }

  static SmscCommand makeCancelSmResp(uint32_t dialogId,uint32_t status)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=CANCEL_RESP;
    _cmd.status=status;
    _cmd.dialogId=dialogId;
    return cmd;
  }

  static SmscCommand makeCancel(SMSId id,const Address& oa,const Address& da)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = CANCEL;
    _cmd.dta = new CancelSm(id,oa,da);
    _cmd.dialogId = 0;
    return cmd;
  }

  static SmscCommand makeCancel(SMSId id)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = CANCEL;
    _cmd.dta = new CancelSm(id);
    _cmd.dialogId = 0;
    return cmd;
  }

  static SmscCommand makeQueryAbonentStatus(const Address& addr)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = QUERYABONENTSTATUS;
    _cmd.dta = new AbonentStatus(addr);
    _cmd.dialogId = 0;
    return cmd;
  }

  static SmscCommand makeQueryAbonentStatusResp(const AbonentStatus& as,int status,const string& msc,const string& imsi)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = QUERYABONENTSTATUS_RESP;
    _cmd.dta = new AbonentStatus(as,status,msc,imsi);
    _cmd.dialogId = 0;
    return cmd;
  }

  static SmscCommand makeEnquireLink(uint32_t dialogId,uint32_t status,int data)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=ENQUIRELINK;
    _cmd.status=status;
    _cmd.intData=data;
    _cmd.dialogId=dialogId;
    return cmd;
  }

  static SmscCommand makeEnquireLinkResp(uint32_t dialogId,uint32_t status,int data)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=ENQUIRELINK_RESP;
    _cmd.status=status;
    _cmd.intData=data;
    _cmd.dialogId=dialogId;
    return cmd;
  }

  static SmscCommand makeCommand(CommandId cmdId,uint32_t dialogId,uint32_t status,void* data)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=cmdId;
    _cmd.status=status;
    _cmd.dta=data;
    _cmd.dialogId=dialogId;
    return cmd;
  }

  static SmscCommand makeSmppPduCommand(SmppHeader* pdu,int channel/*1 receiver,2 transmitter*/)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=SMPP_PDU;
    _cmd.dta=pdu;
    _cmd.dialogId=channel;
    return cmd;
  }

  static SmscCommand makeAlertNotificationCommand(uint32_t dialogId,
                                                  const Address& src,
                                                  const Address& dst,
                                                  int status)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=ALERT_NOTIFICATION;
    AlertNotification *an=new AlertNotification;
    an->src=src;
    an->dst=dst;
    an->status=status;
    _cmd.dta=an;
    _cmd.dialogId=0;
    return cmd;
  }

  static SmscCommand makeKillMrCacheItemCmd(const Address& org,const Address& dst,uint16_t mr)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=KILLMRCACHEITEM;
    KillMrCacheItemCmd *item=new KillMrCacheItemCmd;
    item->org=org;
    item->dst=dst;
    item->mr=mr;
    _cmd.dta=item;
    _cmd.dialogId=0;
    return cmd;
  }

  static SmscCommand makeKillExpiredTransactions()
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=KILLEXPIREDTRANSACTIONS;
    _cmd.dta=0;
    _cmd.dialogId=0;
    return cmd;
  }

  static SmscCommand makeINSmsChargeResponse(SMSId id,const SMS& sms,const INSmsChargeResponse::SubmitContext& cntx,int result)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=INSMSCHARGERESPONSE;
    INSmsChargeResponse* resp=new INSmsChargeResponse;
    resp->id=id;
    resp->sms=sms;
    resp->cntx=cntx;
    resp->result=result;
    _cmd.dta=resp;
    _cmd.dialogId=0;
    return cmd;
  }

  static SmscCommand makeINFwdSmsChargeResponse(SMSId id,const SMS& sms,const INFwdSmsChargeResponse::ForwardContext& context,int result)
  {
    SmscCommand cmd;
    cmd.cmd=new _SmscCommand;
    _SmscCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=INFWDSMSCHARGERESPONSE;
    INFwdSmsChargeResponse* resp=new INFwdSmsChargeResponse;
    resp->id=id;
    resp->sms=sms;
    resp->cntx=context;
    resp->result=result;
    _cmd.dta=resp;
    _cmd.dialogId=0;
    return cmd;
  }

  static void makeSMSBody(SMS* sms,const SmppHeader* pdu,uint32_t smeFlags)
  {
    const PduXSm* xsm = reinterpret_cast<const PduXSm*>(pdu);
    //(SMS*)_cmd->dta =  new SMS;
    fetchSmsFromSmppPdu((PduXSm*)xsm,sms,smeFlags);
    SMS &s=*sms;//((SMS*)_cmd->dta);
    if(s.getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)!=0x3 && s.getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)
    {
      unsigned len;
      const unsigned char* data;
      if(s.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
      {
        data=(const unsigned char*)s.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      }else
      {
        data=(const unsigned char*)s.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
      }
      if(len==0 || *data>len)
      {
        throw Exception("SmscCommand: Invalid pdu (udhi length > message length)");
      }
    }
  }

  ~SmscCommand() {
     //__trace__(__PRETTY_FUNCTION__);
     dispose();
  }
  SmscCommand() : cmd (0) {}
  SmscCommand(SmppHeader* pdu,uint32_t smeFlags=0) : cmd (0)
  {
    __require__ ( pdu != NULL );
    auto_ptr<_SmscCommand> _cmd(ref(new _SmscCommand()));
    switch ( pdu->commandId )
    {
      case SmppCommandSet::QUERY_SM:
      {
        _cmd->cmdid=QUERY;
        _cmd->dta=new QuerySm(reinterpret_cast<PduQuerySm*>(pdu));
        goto end_construct;
      }
      case SmppCommandSet::QUERY_SM_RESP:
      {
        _cmd->cmdid=QUERY_RESP;
        PduQuerySmResp* resp=reinterpret_cast<PduQuerySmResp*>(pdu);
        uint64_t id=0;
        if(resp->messageId.size())sscanf(resp->messageId.cstr(),"%lld",&id);
        time_t findate=0;
        if(resp->finalDate.size())findate=smppTime2CTime(resp->finalDate);
        _cmd->dta=new QuerySmResp(resp->get_header().get_commandStatus(),id,findate,resp->get_messageState(),resp->get_errorCode());
        _cmd->dialogId=resp->get_header().get_sequenceNumber();
        goto end_construct;
      }
      case SmppCommandSet::DATA_SM:
      {
        _cmd->cmdid = SUBMIT;
        PduDataSm* dsm = reinterpret_cast<PduDataSm*>(pdu);
        _cmd->dta =  new SMS;
        if(!fetchSmsFromDataSmPdu(dsm,(SMS*)(_cmd->dta),smeFlags))throw Exception("Invalid data coding");
        ((SMS*)_cmd->dta)->setIntProperty(Tag::SMPP_DATA_SM,1);
        goto end_construct;
      }
      case SmppCommandSet::SUBMIT_SM:
      {
        _cmd->cmdid = SUBMIT;
        goto sms_pdu;
      }
      case SmppCommandSet::DELIVERY_SM:
      {
        _cmd->cmdid = DELIVERY;
        goto sms_pdu;
      }
      case SmppCommandSet::SUBMIT_SM_RESP:
      {
        _cmd->cmdid = SUBMIT_RESP;
        goto sms_resp;
      }
      case SmppCommandSet::DATA_SM_RESP:
      case SmppCommandSet::DELIVERY_SM_RESP:
      {
        _cmd->cmdid = DELIVERY_RESP;
        goto sms_resp;
      }
      case SmppCommandSet::REPLACE_SM:
      {
        _cmd->cmdid=REPLACE;
        _cmd->dta=new ReplaceSm(reinterpret_cast<PduReplaceSm*>(pdu));
        goto end_construct;
      }
      case SmppCommandSet::REPLACE_SM_RESP:
      {
        _cmd->cmdid=REPLACE_RESP;
        _cmd->status=pdu->get_commandStatus();
        _cmd->dialogId=pdu->get_sequenceNumber();
        goto end_construct;
      }
      case SmppCommandSet::CANCEL_SM:
      {
        _cmd->cmdid=CANCEL;
        _cmd->dta=new CancelSm(reinterpret_cast<PduCancelSm*>(pdu));
        goto end_construct;
      }
      case SmppCommandSet::CANCEL_SM_RESP:
      {
        _cmd->cmdid=CANCEL_RESP;
        _cmd->status=pdu->get_commandStatus();
        _cmd->dialogId=pdu->get_sequenceNumber();
        goto end_construct;
      }
      case SmppCommandSet::ENQUIRE_LINK:
      {
        _cmd->cmdid=ENQUIRELINK;
        goto end_construct;
      }
      case SmppCommandSet::ENQUIRE_LINK_RESP:
      {
        _cmd->cmdid=ENQUIRELINK_RESP;
        _cmd->status=pdu->get_commandStatus();
        goto end_construct;
      }
      case SmppCommandSet::SUBMIT_MULTI:
      {
        PduMultiSm* pduX = reinterpret_cast<PduMultiSm*>(pdu);
        _cmd->cmdid=SUBMIT_MULTI_SM;
        _cmd->dta=new SubmitMultiSm;
        SubmitMultiSm& sm=*((SubmitMultiSm*)_cmd->dta);
        makeSMSBody(&((SubmitMultiSm*)_cmd->dta)->msg,pdu,smeFlags);
        unsigned u = 0;
        unsigned uu = pduX->message.numberOfDests;
        for ( ; u < uu; ++u )
        {
          sm.dests[u].dest_flag = (pduX->message.dests[u].flag == 0x02);
          __trace2__(":SUBMIT_MULTI_COMMAND: dest_flag = %d",pduX->message.dests[u].flag);
          if ( pduX->message.dests[u].flag == 0x01 ) // SME address
          {
            sm.dests[u].value = pduX->message.dests[u].get_value()?pduX->message.dests[u].get_value():"";
            sm.dests[u].ton = pduX->message.dests[u].get_typeOfNumber();
            sm.dests[u].npi = pduX->message.dests[u].get_numberingPlan();
          }
          else if (pduX->message.dests[u].flag == 0x02)// Distribution list
          {
            sm.dests[u].value = pduX->message.dests[u].get_value()?pduX->message.dests[u].get_value():"";
          }
          else
          {
            __warning2__("submitmulti has invalid address flag 0x%x",pduX->message.dests[u].flag);
            throw runtime_error("submitmulti has invalid address flag");
          }
        }
        sm.number_of_dests = uu;
        goto end_construct;
      }
      default: throw Exception("Unsupported command id %08X",pdu->commandId);
    }
    //__unreachable__("command id is not processed");

    sms_pdu:
    {
      _cmd->dta =  new SMS;
      makeSMSBody((SMS*)_cmd->dta,pdu,smeFlags);
      goto end_construct;
    }
    sms_resp:
    {
      PduXSmResp* xsm = reinterpret_cast<PduXSmResp*>(pdu);
      _cmd->dta = new SmsResp;
      ((SmsResp*)_cmd->dta)->set_messageId(xsm->get_messageId());
      ((SmsResp*)_cmd->dta)->set_status(xsm->header.get_commandStatus());

      if(pdu->commandId==SmppCommandSet::DELIVERY_SM_RESP || pdu->commandId==SmppCommandSet::DATA_SM_RESP)
      {
        int status=((SmsResp*)_cmd->dta)->get_status()&0xffff;
        if(status==SmppStatusSet::ESME_ROK)
        {
          ((SmsResp*)_cmd->dta)->set_status(
            MAKE_COMMAND_STATUS(CMD_OK,status)
            );
        }else
        if(smsc::system::Status::isErrorPermanent(status))
        {
          ((SmsResp*)_cmd->dta)->set_status(
            MAKE_COMMAND_STATUS(CMD_ERR_PERM,status)
            );
        }else
        {
          ((SmsResp*)_cmd->dta)->set_status(
            MAKE_COMMAND_STATUS(CMD_ERR_TEMP,status)
            );
        }
      }
      if(pdu->commandId==SmppCommandSet::DATA_SM_RESP)
      {
        SmsResp* resp=((SmsResp*)_cmd->dta);
        resp->set_dataSm();
        PduDataSmResp* dsmResp=reinterpret_cast<PduDataSmResp*>(pdu);
        if(dsmResp->get_optional().has_deliveryFailureReason())
        {
          resp->deliveryFailureReason=dsmResp->get_optional().get_deliveryFailureReason();
          resp->haveDeliveryFailureReason=true;
        }
        if(dsmResp->get_optional().has_networkErrorCode())
        {
          memcpy(resp->networkErrorCode,dsmResp->get_optional().get_networkErrorCode(),3);
          resp->haveNetworkErrorCode=true;
        }
        if(dsmResp->get_optional().has_additionalStatusInfoText())
        {
          resp->additionalStatusInfoText=dsmResp->get_optional().get_additionalStatusInfoText();
        }
      }
      goto end_construct;
    }
    end_construct:
    _cmd->dialogId=pdu->get_sequenceNumber();
    cmd = _cmd.release();
    return;
  }

  static uint32_t makeSmppStatus(uint32_t status);
  static bool standardErrorCodes;

  SmppHeader* makePdu(uint32_t smeFlags)
  {
    _SmscCommand& c = *cmd;
    switch ( c.get_commandId() )
    {
    case SUBMIT:
      {
        auto_ptr<PduXSm> xsm(new PduXSm);
        xsm->header.set_commandId(SmppCommandSet::SUBMIT_SM);
        xsm->header.set_sequenceNumber(c.get_dialogId());
        fillSmppPduFromSms(xsm.get(),c.get_sms(),smeFlags);
        return reinterpret_cast<SmppHeader*>(xsm.release());
      }
    case DELIVERY:
      {
        if(c.get_sms()->getIntProperty(Tag::SMPP_DATA_SM))
        {
          auto_ptr<PduDataSm> xsm(new PduDataSm);
          xsm->header.set_commandId(SmppCommandSet::DATA_SM);
          xsm->header.set_sequenceNumber(c.get_dialogId());
          fillDataSmFromSms(xsm.get(),c.get_sms(),smeFlags);
          return reinterpret_cast<SmppHeader*>(xsm.release());
        }else
        {
          auto_ptr<PduXSm> xsm(new PduXSm);
          xsm->header.set_commandId(SmppCommandSet::DELIVERY_SM);
          xsm->header.set_sequenceNumber(c.get_dialogId());
          fillSmppPduFromSms(xsm.get(),c.get_sms(),smeFlags);
          xsm->message.set_scheduleDeliveryTime("");
          xsm->message.set_validityPeriod("");
          xsm->message.set_replaceIfPresentFlag(0);
          return reinterpret_cast<SmppHeader*>(xsm.release());
        }
      }
    case SUBMIT_RESP:
      {
        if(c.get_resp()->get_dataSm())
        {
          auto_ptr<PduDataSmResp> xsm(new PduDataSmResp);
          xsm->header.set_commandId(SmppCommandSet::DATA_SM_RESP);
          xsm->header.set_sequenceNumber(c.get_dialogId());
          xsm->header.set_commandStatus(makeSmppStatus(c.get_resp()->get_status()));
          xsm->set_messageId(c.get_resp()->get_messageId());
          if(c.get_resp()->haveDpf)
          {
            xsm->get_optional().set_dpfResult(c.get_resp()->dpfResult);
          }
          if(c.get_resp()->haveDeliveryFailureReason)
          {
            xsm->get_optional().set_deliveryFailureReason(c.get_resp()->deliveryFailureReason);
          }
          if(c.get_resp()->haveNetworkErrorCode)
          {
            xsm->get_optional().set_networkErrorCode(c.get_resp()->networkErrorCode);
          }
          if(c.get_resp()->additionalStatusInfoText.length())
          {
            xsm->get_optional().set_additionalStatusInfoText(c.get_resp()->additionalStatusInfoText.c_str());
          }
          return reinterpret_cast<SmppHeader*>(xsm.release());
        }else
        {
          auto_ptr<PduXSmResp> xsm(new PduXSmResp);
          xsm->header.set_commandId(SmppCommandSet::SUBMIT_SM_RESP);
          xsm->header.set_sequenceNumber(c.get_dialogId());
          xsm->header.set_commandStatus(makeSmppStatus(c.get_resp()->get_status()));
          xsm->set_messageId(c.get_resp()->get_messageId());
          if((smeFlags&sfSmppPlus) && c.get_resp()->ussd_session_id!=-1)
          {
            xsm->set_ussdSessionId(c.get_resp()->ussd_session_id);
          }
          return reinterpret_cast<SmppHeader*>(xsm.release());
        }
      }
    case SUBMIT_MULTI_SM_RESP:
      {
        {
          auto_ptr<PduMultiSmResp> xsmR(new PduMultiSmResp);
          xsmR->header.set_commandId(SmppCommandSet::SUBMIT_MULTI_RESP);
          xsmR->header.set_sequenceNumber(c.get_dialogId());
          SubmitMultiResp* mr = c.get_MultiResp();
          xsmR->header.set_commandStatus(makeSmppStatus(mr->get_status()));
          xsmR->set_messageId(mr->get_messageId());
          if ( mr->no_unsuccess ) {
            vector<UnsuccessDeliveries> ud;
            ud.resize(mr->no_unsuccess);
            for ( unsigned u=0; u<mr->no_unsuccess; ++u )
            {
              ud[u].addr.set_numberingPlan(mr->unsuccess[u].addr.getNumberingPlan());
              ud[u].addr.set_typeOfNumber(mr->unsuccess[u].addr.getTypeOfNumber());
              ud[u].addr.set_value(mr->unsuccess[u].addr.value);
              ud[u].errorStatusCode = mr->unsuccess[u].errcode;
            }
            xsmR->set_sme(&ud[0],mr->no_unsuccess);
            xsmR->set_noUnsuccess(mr->no_unsuccess);
          }
          return reinterpret_cast<SmppHeader*>(xsmR.release());
        }
      }
    case DELIVERY_RESP:
      {
        auto_ptr<PduXSmResp> xsm(new PduXSmResp);
        xsm->header.set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        xsm->header.set_sequenceNumber(c.get_dialogId());
        xsm->header.set_commandStatus(makeSmppStatus(c.get_resp()->get_status()));
        xsm->set_messageId(c.get_resp()->get_messageId());
        return reinterpret_cast<SmppHeader*>(xsm.release());
      }
    case GENERIC_NACK:
      {
        auto_ptr<PduGenericNack> gnack(new PduGenericNack);
        gnack->header.set_commandId(SmppCommandSet::GENERIC_NACK);
        gnack->header.set_sequenceNumber(c.get_dialogId());
        gnack->header.set_commandStatus(makeSmppStatus((uint32_t)c.status));
        return reinterpret_cast<SmppHeader*>(gnack.release());
      }
    case UNBIND:
      {
        auto_ptr<PduUnbind> unb(new PduUnbind);
        unb->header.set_commandId(SmppCommandSet::UNBIND);
        unb->header.set_sequenceNumber(c.get_dialogId());
        unb->header.set_commandStatus(0);
        return reinterpret_cast<SmppHeader*>(unb.release());
      }
    case UNBIND_RESP:
      {
        auto_ptr<PduUnbindResp> unb(new PduUnbindResp);
        unb->header.set_commandId(SmppCommandSet::UNBIND_RESP);
        unb->header.set_sequenceNumber(c.get_dialogId());
        unb->header.set_commandStatus(makeSmppStatus((uint32_t)c.status));
        return reinterpret_cast<SmppHeader*>(unb.release());
      }
    case REPLACE:
      {
        auto_ptr<PduReplaceSm> rpl(new PduReplaceSm);
        rpl->header.set_commandId(SmppCommandSet::REPLACE_SM);
        rpl->header.set_sequenceNumber(c.get_dialogId());
        rpl->header.set_commandStatus(0);

        const ReplaceSm *r=&c.get_replaceSm();

        rpl->set_messageId(r->messageId.get());
        rpl->set_source(Address2PduAddress(r->sourceAddr.get()));
        char buf[64];
        cTime2SmppTime(r->scheduleDeliveryTime,buf);
        rpl->set_scheduleDeliveryTime(buf);
        cTime2SmppTime(r->validityPeriod,buf);
        rpl->set_validityPeriod(buf);
        rpl->set_registredDelivery(r->registeredDelivery);
        rpl->set_smDefaultMsgId(r->smDefaultMsgId);
        rpl->shortMessage.copy(r->shortMessage.get(),r->smLength);

        return reinterpret_cast<SmppHeader*>(rpl.release());
      }
    case REPLACE_RESP:
      {
        auto_ptr<PduReplaceSmResp> repl(new PduReplaceSmResp);
        repl->header.set_commandId(SmppCommandSet::REPLACE_SM_RESP);
        repl->header.set_sequenceNumber(c.get_dialogId());
        repl->header.set_commandStatus(makeSmppStatus((uint32_t)c.status));
        return reinterpret_cast<SmppHeader*>(repl.release());
      }
    case QUERY:
      {
        auto_ptr<PduQuerySm> query(new PduQuerySm);
        query->header.set_commandId(SmppCommandSet::QUERY_SM);
        query->header.set_sequenceNumber(c.get_dialogId());
        query->header.set_commandStatus(makeSmppStatus((uint32_t)c.status));
        query->set_messageId(c.get_querySm().messageId.get());
        query->set_source(Address2PduAddress(c.get_querySm().sourceAddr.get()));
        return reinterpret_cast<SmppHeader*>(query.release());
      }
    case QUERY_RESP:
      {
        auto_ptr<PduQuerySmResp> qresp(new PduQuerySmResp);
        qresp->header.set_commandId(SmppCommandSet::QUERY_SM_RESP);
        qresp->header.set_sequenceNumber(c.get_dialogId());
        ((QuerySmResp*)c.dta)->fillPdu(qresp.get());
        qresp->get_header().set_commandStatus
        (
          makeSmppStatus
          (
            ((QuerySmResp*)c.dta)->commandStatus
          )
        );
        return reinterpret_cast<SmppHeader*>(qresp.release());
      }
    case CANCEL:
      {
        auto_ptr<PduCancelSm> cnc(new PduCancelSm);
        cnc->header.set_commandId(SmppCommandSet::CANCEL_SM);
        cnc->header.set_sequenceNumber(c.get_dialogId());
        cnc->header.set_commandStatus(makeSmppStatus((uint32_t)c.status));

        if(c.get_cancelSm().serviceType.get())
          cnc->set_serviceType(c.get_cancelSm().serviceType.get());
        if(c.get_cancelSm().messageId.get())
          cnc->set_messageId(c.get_cancelSm().messageId.get());

        if(c.get_cancelSm().sourceAddr.get())
          cnc->set_source(Address2PduAddress(c.get_cancelSm().sourceAddr.get()));
        if(c.get_cancelSm().destAddr.get())
          cnc->set_dest(Address2PduAddress(c.get_cancelSm().destAddr.get()));

        return reinterpret_cast<SmppHeader*>(cnc.release());
      }
    case CANCEL_RESP:
      {
        auto_ptr<PduCancelSmResp> cresp(new PduCancelSmResp);
        cresp->header.set_commandId(SmppCommandSet::CANCEL_SM_RESP);
        cresp->header.set_sequenceNumber(c.get_dialogId());
        cresp->header.set_commandStatus(makeSmppStatus((uint32_t)c.status));
        return reinterpret_cast<SmppHeader*>(cresp.release());
      }
    case ENQUIRELINK:
      {
        auto_ptr<PduEnquireLink> pdu(new PduEnquireLink);
        pdu->header.set_commandId(SmppCommandSet::ENQUIRE_LINK);
        pdu->header.set_sequenceNumber(c.get_dialogId());
        pdu->header.set_commandStatus(0);
        return reinterpret_cast<SmppHeader*>(pdu.release());
      }
    case ENQUIRELINK_RESP:
      {
        auto_ptr<PduEnquireLink> pdu(new PduEnquireLinkResp);
        pdu->header.set_commandId(SmppCommandSet::ENQUIRE_LINK_RESP);
        pdu->header.set_sequenceNumber(c.get_dialogId());
        pdu->header.set_commandStatus(makeSmppStatus((uint32_t)c.status));
        return reinterpret_cast<SmppHeader*>(pdu.release());
      }
    case SMPP_PDU:
      {
        SmppHeader* pdu=(SmppHeader*)c.dta;
        c.dta=0;
        return pdu;
      }
    case ALERT_NOTIFICATION:
      {
        auto_ptr<PduAlertNotification> pdu(new PduAlertNotification);
        AlertNotification& an=c.get_alertNotification();
        pdu->header.set_commandId(SmppCommandSet::ALERT_NOTIFICATION);
        pdu->header.set_sequenceNumber(c.get_dialogId());
        pdu->header.set_commandStatus(0);
        pdu->source.set_typeOfNumber(an.src.type);
        pdu->source.set_numberingPlan(an.src.plan);
        pdu->source.set_value(an.src.value);
        pdu->esme.set_typeOfNumber(an.dst.type);
        pdu->esme.set_numberingPlan(an.dst.plan);
        pdu->esme.set_value(an.dst.value);
        pdu->optional.set_msAvailableStatus(an.status);
        return reinterpret_cast<SmppHeader*>(pdu.release());
      }
    default:
      throw runtime_error("unknown commandid");
    }
    return 0; // for compiler
  }

  SmscCommand(const SmscCommand& _cmd)
  {
    // copy(_cmd.cmd);
    // if ( cmd ) unref(cmd);
    //__trace2__("%s(_cmd)",__PRETTY_FUNCTION__);
    //__watch__((void*)_cmd.cmd);
    cmd = ref(_cmd.cmd);
  }

  const SmscCommand& operator = (const SmscCommand& _cmd)
  {
    // copy(_cmd.cmd);
    //__trace2__("%s(_cmd)",__PRETTY_FUNCTION__);
    //__watch__((void*)_cmd.cmd);
    //__watch__((void*)cmd);
    if (cmd) unref(cmd);
    cmd = ref(_cmd.cmd);
    return _cmd;
  }

  _SmscCommand* operator->() const
  {
    __require__(cmd);
    return cmd;
  }
};

} //smeman
} //smsc

#endif
