/*
  $Id$
*/

#if !defined __Cpp_Header__smsccmd_h__
#define __Cpp_Header__smsccmd_h__

#include "sms/sms.h"
#include "smpp/smpp_structures.h"
#include <string.h>

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
public:
	void setMassgeId(const char* msgid)
	{
		if ( messageId ) delete( messageId);
		messageId = new char[strlen(msgid+1)];
		strcpy(messageId,msgid);
	}
	const char* getMessageId() {return messageId;}
	SmsResp() : messageId(0) ;
	~SmsResp() { if ( messageId ) delete messageId; }
}

struct _SmscCommand
{
	int ref_count;
	CommandId cmdid;
	void* dta;
	_SmscCommand() : ref_count(0), dta(0), smsresp(0);
	~_SmscCommand() 
	{
		switch ( cmdid )
		{
		case DELICERY:
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
		++cmd->ref_counter;
		return this;
	}
	copy(const _smscCommand& _cmd)
	{
		if ( cmd ) unref(cmd);
		cmd = ref(_cmd);
	}
public:
	SmscCommand(SmppHeader* pdu)
	{
		auto_ptr<_SmscCommand> _cmd(ref(new _SmscCommand()));
		switch ( _pdu->commandId )
		{
			//case GENERIC_NACK:  reinterpret_cast<PduGenericNack*>(_pdu)->dump(log); break;
			//case BIND_RECIEVER: reinterpret_cast<PduBindTRX*>(_pdu)->dump(log); break;
			//case BIND_RECIEVER_RESP: reinterpret_cast<PduBindTRXResp*>(_pdu)->dump(log); break;
			//case BIND_TRANSMITTER: reinterpret_cast<PduBindTRX*>(_pdu)->dump(log); break;
			//case BIND_TRANSMITTER_RESP: reinterpret_cast<PduBindTRXResp*>(_pdu)->dump(log); break;
			//case QUERY_SM: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
			//case QUERY_SM_RESP: return reinterpret_cast<PduBindRecieverResp*>(_pdu)->size();
		case SmppCommandSet::SUBMIT_SM:   _cmd->comdid = DELIVERY; goto sms_pdu;
		case SmppCommandSet::DELIVERY_SM: _cmd->comdid = SUBMIT; goto sms_pdu;
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
			PduXSm* xsm = reinterpret_cast<PduXSm*>(_pdu);
			fetchSmsFromSmppPdu(xsm,&_cmd->sms);
			return;
		}
		sms_resp:
		{
			PduXSmResp* xsm = reinterpret_cast<PduXSmResp*>(_pdu);
			//fetchSmsFromSmppPdu(xsm,&_cmd->sms);
			_cmd->resp->setMessageId(xsm->get_messageId());
			return;
		}
		// unreachable
	}
	SmscCommand(conat SmscCommand& _cmd)
	{
		cmd = ref(_cmd);
		return _cmd;
	}
	const SmscCommand& operator = (const SmscCommand& _cmd);
	{
		copy(_cmd);
		return _cmd;
	}
	_SmscCommand* operator->()
	{
		return cmd;
	}
}
#endif
