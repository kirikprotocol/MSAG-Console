/*
  $Id$
*/

#if !defined __Cpp_Header__smsccmd_h__
#define __Cpp_Header__smsccmd_h__

/*
Для реализации кода команнды, так же,  
можно использовать и полиморфизм для класса _SmscCommand
и реализовать несколько фектори : для создания команд из Smpp/Map PDU 
и создание PDU из команд
*/

#include "sms/sms.h"
#include "smpp/smpp_structures.h"
#include "smpp/smpp_sms.h"
#include <string.h>
#include <memory>
#include <inttypes.h>

namespace smsc{
namespace smeman{

using std::auto_ptr;
using namespace smsc::smpp;
using smsc::sms::SMS;
using smsc::sms::Address;

enum CommandId
{
  UNKNOWN,
  DELIVERY,
  DELIVERY_RESP,
  SUBMIT,
  SUBMIT_RESP
};

struct SmsResp
{
private:
  char* messageId;
  uint32_t status;
public:
  void set_messageId(const char* msgid)
  {
    if ( messageId ) delete( messageId);
    messageId = new char[strlen(msgid+1)];
    strcpy(messageId,msgid);
  }
  void set_status(uint32_t st) { status = st; }
  const char* get_messageId() {return messageId;}
  uint32_t get_status() { return status; }
  SmsResp() : messageId(0), status(0) {};
  ~SmsResp() { if ( messageId ) delete messageId; }
};

struct _SmscCommand
{
  mutable int ref_count;
  CommandId cmdid;
  uint32_t dialogId;
  void* dta;
  _SmscCommand() : ref_count(0), dta(0){};
  ~_SmscCommand()
  {
    switch ( cmdid )
    {
    case DELIVERY:
    case SUBMIT:
      delete ( (SMS*)dta );
    case DELIVERY_RESP:
    case SUBMIT_RESP:
      delete ( (SmsResp*)dta );
    case UNKNOWN:
      __unreachable__("incorrect state dat != NULL && cmdid == UNKNOWN");
    default:
      __unreachable__("unprocessed cmdid");
    }
  }
  uint32_t get_dialogId() { return dialogId; }
  CommandId get_commandId() { return cmdid; }
  SMS* get_sms() { return (SMS*)dta; }
  SmsResp* get_resp() { return (SmsResp*)dta; }
};

class SmscCommand
{
  _SmscCommand* cmd;
  void unref(_SmscCommand*& cmd)
  {
    __require__ ( cmd != 0 );
    __require__ ( cmd->ref_count > 0 );
    if ( --cmd->ref_count == 0 )
    {
      delete cmd;
      cmd = 0;
    }
  }
  _SmscCommand* ref(_SmscCommand* cmd)
  {
    __require__ ( cmd != 0 );
    __require__ ( cmd->ref_count >= 0 );
    ++cmd->ref_count;
    return cmd;
  }
  
  void copy(const _SmscCommand& _cmd)
  {
    if ( cmd ) unref(cmd);
    cmd = ref(const_cast<_SmscCommand*>(&_cmd));
  }


	void dispose() // for debuging ;(
		{ if ( cmd ) delete cmd; }

public:
  
  struct Status
  {
    static const int ERROR = 8;
    static const int OK = 0;
  };
  
  // specialized constructors (meta constructors)
  static SmscCommand makeSumbmitSm(const SMS& sms,uint32_t dialogId)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.cmdid = SUBMIT;
    _cmd.dta = new SMS;
    *_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }
  
  static SmscCommand makeDeliverySm(const SMS& sms,uint32_t dialogId)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.cmdid = DELIVERY;
    _cmd.dta = new SMS;
    *_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }
  
  static SmscCommand makeSubmitSmResp(const char* messageId, uint32_t dialogId, uint32_t status)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.cmdid = SUBMIT_RESP;
    _cmd.dta = new SmsResp;
    _cmd.get_resp()->set_messageId(messageId);
    _cmd.get_resp()->set_status(status);
    _cmd.dialogId = dialogId;
    return cmd;
  }
  static SmscCommand makeDeliverySmResp(const char* messageId, uint32_t dialogId, uint32_t status)
  {
    SmscCommand cmd;
    cmd.cmd = new _SmscCommand;
    _SmscCommand& _cmd = *cmd.cmd;
    _cmd.cmdid = DELIVERY_RESP;
    _cmd.dta = new SmsResp;
    _cmd.get_resp()->set_messageId(messageId);
    _cmd.get_resp()->set_status(status);
    _cmd.dialogId = dialogId;
    return cmd;
  }
	~SmscCommand() { dispose(); }
  //SmscCommand() : cmd (0) {}
  SmscCommand(SmppHeader* pdu=0) : cmd (0)
  {
    if (!pdu) return;
		//__require__ ( pdu != NULL );
    auto_ptr<_SmscCommand> _cmd(ref(new _SmscCommand()));
    switch ( pdu->commandId )
    {
      //case GENERIC_NACK:  reinterpret_cast<PduGenericNack*>(_pdu)->dump(log); break;
      //case BIND_RECIEVER: reinterpret_cast<PduBindTRX*>(_pdu)->dump(log); break;
      //case BIND_RECIEVER_RESP: reinterpret_cast<PduBindTRXResp*>(_pdu)->dump(log); break;
      //case BIND_TRANSMITTER: reinterpret_cast<PduBindTRX*>(_pdu)->dump(log); break;
      //case BIND_TRANSMITTER_RESP: reinterpret_cast<PduBindTRXResp*>(_pdu)->dump(log); break;
      //case QUERY_SM: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
      //case QUERY_SM_RESP: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
    case SmppCommandSet::SUBMIT_SM:  _cmd->cmdid = DELIVERY; goto sms_pdu;
    case SmppCommandSet::DELIVERY_SM: _cmd->cmdid = SUBMIT; goto sms_pdu;
    case SmppCommandSet::SUBMIT_SM_RESP: _cmd->cmdid = SUBMIT_RESP; goto sms_resp;
    case SmppCommandSet::DELIVERY_SM_RESP: _cmd->cmdid = DELIVERY_RESP; goto sms_resp;
      //case DELIVERY_SM: reinterpret_cast<PduDeliverySm*>(_pdu)->dump(log); break;
      //case DELIVERY_SM_RESP: reinterpret_cast<PduDeliverySmResp*>(_pdu)->dump(log); break;
      //case UNBIND: reinterpret_cast<PduUnbind*>(_pdu)->dump(log); break;
      //case UNBIND_RESP: reinterpret_cast<PduUnbindResp*>(_pdu)->dump(log); break;
      //case REPLACE_SM: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
      //case REPLACE_SM_RESP: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
      //case CANCEL_SM: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
      //case CANCEL_SM_RESP: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
      //case BIND_TRANCIEVER: reinterpret_cast<PduBindTRX*>(_pdu)->dump(log); break;
      //case BIND_TRANCIEVER_RESP: reinterpret_cast<PduBindTRXResp*>(_pdu)->dump(log); break;
      //case OUTBIND: reinterpret_cast<PduOutBind*>(_pdu)->dump(log); break;
      //case ENQUIRE_LINK: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
      //case ENQUIRE_LINK_RESP: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
      //case SUBMIT_MULTI: reinterpret_cast<PduMultiSm*>(_pdu)->dump(log); break;
      //case SUBMIT_MULTI_RESP: reinterpret_cast<PduMultiSmResp*>(_pdu)->dump(log); break;
      //case ALERT_NOTIFICATION: return reinterpret_cast<Pdu*>(_pdu)->size();
      //case DATA_SM: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
      //case DATA_SM_RESP: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
    }
    __unreachable__("command id is not processed");
    sms_pdu:
    {
      PduXSm* xsm = reinterpret_cast<PduXSm*>(pdu);
      (SMS*)_cmd->dta =  new SMS;
      fetchSmsFromSmppPdu(xsm,(SMS*)(_cmd->dta));
      //delete (SMS*)_cmd; _cmd = 0;
      goto end_construct;
    }
    sms_resp:
    {
      PduXSmResp* xsm = reinterpret_cast<PduXSmResp*>(pdu);
      (SmsResp*)_cmd->dta = new SmsResp;
      //fetchSmsFromSmppPdu(xsm,&_cmd->sms);
      ((SmsResp*)_cmd->dta)->set_messageId(xsm->get_messageId());
      ((SmsResp*)_cmd->dta)->set_status(xsm->header.get_commandStatus());
      //delete (*(SmsResp*))_cmd; _cmd = 0;
      goto end_construct;
    }
    // unreachable
    //_pdu.release();
    end_construct:
     cmd = _cmd.release();
     return;
  }

  SmppHeader* makePdu()
  {
    _SmscCommand& c = *cmd;
    switch ( c.get_commandId() )
    {
    case SUBMIT:
      {
        auto_ptr<PduXSm> xsm(new PduXSm);
        xsm->header.set_commandId(SmppCommandSet::SUBMIT_SM);
        xsm->header.set_sequenceNumber(c.get_dialogId());
        fillSmppPduFromSms(xsm.get(),c.get_sms());
        return reinterpret_cast<SmppHeader*>(xsm.release());
      } 
    case DELIVERY:
      {
        auto_ptr<PduXSm> xsm(new PduXSm);
        xsm->header.set_commandId(SmppCommandSet::DELIVERY_SM);
        xsm->header.set_sequenceNumber(c.get_dialogId());
        fillSmppPduFromSms(xsm.get(),c.get_sms());
        return reinterpret_cast<SmppHeader*>(xsm.release());
      } 
    case SUBMIT_RESP:
      {
        auto_ptr<PduXSmResp> xsm(new PduXSmResp);
        xsm->header.set_commandId(SmppCommandSet::SUBMIT_SM_RESP);
        xsm->header.set_sequenceNumber(c.get_dialogId());
        xsm->header.set_commandStatus(c.get_resp()->get_status());
        xsm->set_messageId(c.get_resp()->get_messageId());
        return reinterpret_cast<SmppHeader*>(xsm.release());
      } 
    case DELIVERY_RESP:
      {
        auto_ptr<PduXSmResp> xsm(new PduXSmResp);
        xsm->header.set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        xsm->header.set_sequenceNumber(c.get_dialogId());
        xsm->header.set_commandStatus(c.get_resp()->get_status());
        xsm->set_messageId(c.get_resp()->get_messageId());
        return reinterpret_cast<SmppHeader*>(xsm.release());
      } 
    default:
      __unreachable__("unknown commandid");
    }
  }

  SmscCommand(const SmscCommand& _cmd)
  {
   // copy(_cmd.cmd);
   // if ( cmd ) unref(cmd);
    cmd = ref((_SmscCommand*)(&_cmd.cmd));
  }

  const SmscCommand& operator = (const SmscCommand& _cmd)
  {
    // copy(_cmd.cmd);
    if (cmd) unref(cmd);
    cmd = ref(_cmd.cmd);
    return _cmd;
  }

  _SmscCommand* operator->()
  {
    return cmd;
  }
};

}; //smeman
}; //smsc

#endif

