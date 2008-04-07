#include "scag/transport/smpp/SmppCommand.h"

namespace scag{
namespace transport{
namespace smpp{

smsc::core::synchronization::Mutex _SmppCommand::loggerMutex;
smsc::logger::Logger* _SmppCommand::logger = NULL;

smsc::core::synchronization::Mutex _SmppCommand::cntMutex;
uint32_t _SmppCommand::commandCounter = 0;
uint32_t _SmppCommand::stuid = 0;

uint32_t _SmppCommand::getCommandStatus() const
{
    return (cmdid == DELIVERY_RESP || cmdid == SUBMIT_RESP || cmdid == DATASM_RESP)
      ? ((SmsResp*)dta)->get_status() : status;
}

_SmppCommand::~_SmppCommand()
  {
    if(logger->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG))
    {
        uint32_t sc = 0;
        {
            MutexGuard mtxx(cntMutex);
            sc = --commandCounter;
        }
        smsc_log_debug(logger, "Command destroy: count=%d, addr=%s, usr=%d, uid=%d", sc, session.Get() ? session->getSessionKey().abonentAddr.toString().c_str() : "", session.Get() ?  session->getSessionKey().USR : 0, uid);
    }

    switch ( cmdid )
    {
    case DELIVERY:
    case SUBMIT:
    case DATASM:
      if(dta)delete ( (SmsCommand*)dta );
      break;

    case SUBMIT_MULTI_SM:
      delete ( (SubmitMultiSm*)dta );
      break;
    case SUBMIT_MULTI_SM_RESP:
      delete ( (SubmitMultiResp*)dta );
      break;

    case DELIVERY_RESP:
    case SUBMIT_RESP:
    case DATASM_RESP:
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
    case BIND_TRANSCEIVER:
      delete ((BindCommand*)dta);
      break;
    case ALERT_NOTIFICATION:
      delete ((AlertNotification*)dta);
      break;

    case UNKNOWN:
    case GENERIC_NACK:
    case UNBIND_RESP:
    case REPLACE_RESP:
    case CANCEL_RESP:
    case ENQUIRELINK:
    case ENQUIRELINK_RESP:
    case UNBIND:
    case BIND_RECIEVER_RESP:
    case BIND_TRANSMITTER_RESP:
    case BIND_TRANCIEVER_RESP:
    case PROCESSEXPIREDRESP:
      // nothing to delete
      break;
    default:
      __warning2__("~SmppCommand:unprocessed cmdid %d",cmdid);
    }
  }

  SmppCommand::SmppCommand(SmppHeader* pdu,bool forceDC) : cmd (0)
  {
    __require__ ( pdu != NULL );
    auto_ptr<_SmppCommand> _cmd(ref(new _SmppCommand()));
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
        _cmd->cmdid = DATASM;
        PduDataSm* dsm = reinterpret_cast<PduDataSm*>(pdu);
        _cmd->dta =  new SmsCommand;
        if(!fetchSmsFromDataSmPdu(dsm,_cmd->get_sms(),forceDC))throw Exception("Invalid data coding");
        _cmd->get_sms()->setIntProperty(Tag::SMPP_DATA_SM,1);
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
      {
        _cmd->cmdid = DATASM_RESP;
        goto sms_resp;
      }
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
      case SmppCommandSet::ALERT_NOTIFICATION:
      {
        _cmd->cmdid=ALERT_NOTIFICATION;
        _cmd->dta=new AlertNotification((PduAlertNotification*)pdu);
        goto end_construct;
      }
      case SmppCommandSet::ENQUIRE_LINK_RESP:
      {
        _cmd->cmdid=ENQUIRELINK_RESP;
        _cmd->dta=(void*)pdu->get_commandStatus();
        goto end_construct;
      }
      case SmppCommandSet::SUBMIT_MULTI:
      {
        PduMultiSm* pduX = reinterpret_cast<PduMultiSm*>(pdu);
        _cmd->cmdid=SUBMIT_MULTI_SM;
        _cmd->dta=new SubmitMultiSm;
        SubmitMultiSm& sm=*((SubmitMultiSm*)_cmd->dta);
        makeSMSBody(&((SubmitMultiSm*)_cmd->dta)->msg,pdu,forceDC);
        unsigned u = 0;
        unsigned uu = pduX->message.numberOfDests;
        for ( ; u < uu; ++u )
        {
          sm.dests[u].dest_flag = (pduX->message.dests[u].flag == 0x02);
          __trace2__(":SUBMIT_MULTI_COMMAND: dest_flag = %d",pduX->message.dests[u].flag);
          if ( pduX->message.dests[u].flag == 0x01 ) // SME address
          {
            sm.dests[u].value = pduX->message.dests[u].get_value();
            sm.dests[u].ton = pduX->message.dests[u].get_typeOfNumber();
            sm.dests[u].npi = pduX->message.dests[u].get_numberingPlan();
          }
          else if (pduX->message.dests[u].flag == 0x02)// Distribution list
          {
            sm.dests[u].value = pduX->message.dests[u].get_value();
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
      _cmd->dta =  new SmsCommand;
      makeSMSBody(_cmd->get_sms(),pdu,forceDC);
      goto end_construct;
    }
    sms_resp:
    {
      PduXSmResp* xsm = reinterpret_cast<PduXSmResp*>(pdu);
      _cmd->dta = new SmsResp;
      ((SmsResp*)_cmd->dta)->set_messageId(xsm->get_messageId());
      ((SmsResp*)_cmd->dta)->set_status(xsm->header.get_commandStatus());

      /*
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
      */
      if(pdu->commandId==SmppCommandSet::DATA_SM_RESP)
      {
        ((SmsResp*)_cmd->dta)->set_dataSm();
      }
      goto end_construct;
    }
    end_construct:
    _cmd->dialogId=pdu->get_sequenceNumber();
    cmd = _cmd.release();
    return;
  }

  // specialized constructors (meta constructors)
   SmppCommand SmppCommand::makeCommandSm(CommandId command, const SMS& sms,uint32_t dialogId)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = command;
    _cmd.dta = new SmsCommand(sms);
    //*_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeSubmitSm(const SMS& sms,uint32_t dialogId)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = SUBMIT;
    _cmd.dta = new SmsCommand(sms);
    //*_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeDeliverySm(const SMS& sms,uint32_t dialogId)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = DELIVERY;
    _cmd.dta = new SmsCommand(sms);
    //*_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeDataSm(const SMS& sms,uint32_t dialogId)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = DATASM;
    _cmd.dta = new SmsCommand(sms);
    //*_cmd.get_sms() = sms;
    _cmd.dialogId = dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeSubmitSmResp(const char* messageId, uint32_t dialogId, uint32_t status,bool dataSm)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = SUBMIT_RESP;
    _cmd.dta = new SmsResp;
    _cmd.get_resp()->set_messageId(messageId);
    _cmd.get_resp()->set_status(status);
    if(dataSm)_cmd.get_resp()->set_dataSm();
    _cmd.dialogId = dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeDataSmResp(const char* messageId, uint32_t dialogId, uint32_t status)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = DATASM_RESP;
    _cmd.dta = new SmsResp;
    _cmd.get_resp()->set_messageId(messageId);
    _cmd.get_resp()->set_status(status);
    _cmd.dialogId = dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeSubmitMultiResp(const char* messageId, uint32_t dialogId, uint32_t status)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = SUBMIT_MULTI_SM_RESP;
    _cmd.dta = new SubmitMultiResp;
    _cmd.get_MultiResp ()->set_messageId(messageId);
    _cmd.get_MultiResp()->set_status(status);
    _cmd.dialogId = dialogId;
    _cmd.get_MultiResp()->set_unsuccessCount(0);
    return cmd;
  }

   SmppCommand SmppCommand::makeDeliverySmResp(const char* messageId, uint32_t dialogId, uint32_t status)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = DELIVERY_RESP;
    _cmd.dta = new SmsResp;
    _cmd.get_resp()->set_messageId(messageId);
    _cmd.get_resp()->set_status(status);
    _cmd.dialogId = dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeGenericNack(uint32_t dialogId,uint32_t status)
  {
    SmppCommand cmd;
    cmd.cmd=new _SmppCommand;
    _SmppCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=GENERIC_NACK;
    _cmd.status=status;
    _cmd.dialogId=dialogId;
    return cmd;
  }


   SmppCommand SmppCommand::makeUnbind(int dialogId,int mode)
  {
    SmppCommand cmd;
    cmd.cmd=new _SmppCommand;
    _SmppCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=UNBIND;
    _cmd.dta=(void*)mode;
    _cmd.status=0;
    _cmd.dialogId=dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeUnbindResp(uint32_t dialogId,uint32_t status,void* data)
  {
    SmppCommand cmd;
    cmd.cmd=new _SmppCommand;
    _SmppCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=UNBIND_RESP;
    _cmd.dta=data;
    _cmd.status=status;
    _cmd.dialogId=dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeReplaceSmResp(uint32_t dialogId,uint32_t status)
  {
    SmppCommand cmd;
    cmd.cmd=new _SmppCommand;
    _SmppCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=REPLACE_RESP;
    _cmd.status=status;
    _cmd.dialogId=dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeQuerySmResp(uint32_t dialogId,uint32_t status,
            SMSId id,time_t findate,uint8_t state,uint8_t netcode)
  {
    SmppCommand cmd;
    cmd.cmd=new _SmppCommand;
    _SmppCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=QUERY_RESP;
    _cmd.dta=new QuerySmResp(status,id,findate,state,netcode);
    _cmd.dialogId=dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeCancelSmResp(uint32_t dialogId,uint32_t status)
  {
    SmppCommand cmd;
    cmd.cmd=new _SmppCommand;
    _SmppCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=CANCEL_RESP;
    _cmd.status=status;
    _cmd.dialogId=dialogId;
    return cmd;
  }

   SmppCommand SmppCommand::makeCancel(SMSId id,const Address& oa,const Address& da)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = CANCEL;
    _cmd.dta = new CancelSm(id,oa,da);
    _cmd.dialogId = 0;
    return cmd;
  }

   SmppCommand SmppCommand::makeCancel(SMSId id)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = CANCEL;
    _cmd.dta = new CancelSm(id);
    _cmd.dialogId = 0;
    return cmd;
  }

   SmppCommand SmppCommand::makeBindCommand(const char* sysId,const char* pwd,const char* addrRange,const char* sysType)
  {
    SmppCommand cmd;
    cmd.cmd = new _SmppCommand;
    _SmppCommand& _cmd = *cmd.cmd;
    _cmd.ref_count = 1;
    _cmd.cmdid = BIND_TRANSCEIVER;
    _cmd.dta = new BindCommand(sysId,pwd,addrRange,sysType);
    _cmd.dialogId = 1;
    return cmd;
  }

   SmppCommand SmppCommand::makeCommand(CommandId cmdId,uint32_t dialogId,uint32_t status,void* data)
  {
    SmppCommand cmd;
    cmd.cmd=new _SmppCommand;
    _SmppCommand& _cmd=*cmd.cmd;
    _cmd.ref_count=1;
    _cmd.cmdid=cmdId;
    _cmd.status=status;
    _cmd.dta=data;
    _cmd.dialogId=dialogId;
    return cmd;
  }

   void SmppCommand::makeSMSBody(SMS* sms,const SmppHeader* pdu,bool forceDC)
  {
    const PduXSm* xsm = reinterpret_cast<const PduXSm*>(pdu);
    fetchSmsFromSmppPdu((PduXSm*)xsm,sms,forceDC);
    SMS &s=*sms;
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
        throw Exception("SmppCommand: Invalid pdu (udhi length > message length)");
      }
    }
  }

  SmppHeader* SmppCommand::makePdu(bool forceDC)
  {
    _SmppCommand& c = *cmd;
    switch ( c.get_commandId() )
    {
    case SUBMIT:
      {
        auto_ptr<PduXSm> xsm(new PduXSm);
        xsm->header.set_commandId(SmppCommandSet::SUBMIT_SM);
        xsm->header.set_sequenceNumber(c.get_dialogId());
        fillSmppPduFromSms(xsm.get(),c.get_sms(),forceDC);
        return reinterpret_cast<SmppHeader*>(xsm.release());
      }
    case DELIVERY:
      {
        if(c.get_sms()->getIntProperty(Tag::SMPP_DATA_SM))
        {
          auto_ptr<PduDataSm> xsm(new PduDataSm);
          xsm->header.set_commandId(SmppCommandSet::DATA_SM);
          xsm->header.set_sequenceNumber(c.get_dialogId());
          fillDataSmFromSms(xsm.get(),c.get_sms(),forceDC);
          return reinterpret_cast<SmppHeader*>(xsm.release());
        }else
        {
          auto_ptr<PduXSm> xsm(new PduXSm);
          xsm->header.set_commandId(SmppCommandSet::DELIVERY_SM);
          xsm->header.set_sequenceNumber(c.get_dialogId());
          fillSmppPduFromSms(xsm.get(),c.get_sms());
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
          return reinterpret_cast<SmppHeader*>(xsm.release());
        }else
        {
          auto_ptr<PduXSmResp> xsm(new PduXSmResp);
          xsm->header.set_commandId(SmppCommandSet::SUBMIT_SM_RESP);
          xsm->header.set_sequenceNumber(c.get_dialogId());
          xsm->header.set_commandStatus(makeSmppStatus(c.get_resp()->get_status()));
          xsm->set_messageId(c.get_resp()->get_messageId());
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
            std::vector<UnsuccessDeliveries> ud;
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
    case DATASM:
      {
          auto_ptr<PduDataSm> xsm(new PduDataSm);
          xsm->header.set_commandId(SmppCommandSet::DATA_SM);
          xsm->header.set_sequenceNumber(c.get_dialogId());
          fillDataSmFromSms(xsm.get(),c.get_sms(),forceDC);
          return reinterpret_cast<SmppHeader*>(xsm.release());
      }
    case DATASM_RESP:
      {
          auto_ptr<PduDataSmResp> xsm(new PduDataSmResp);
          xsm->header.set_commandId(SmppCommandSet::DATA_SM_RESP);
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
    case BIND_TRANSCEIVER:
      {
        auto_ptr<PduBindTRX> pdu(new PduBindTRX);
        pdu->header.set_commandId(SmppCommandSet::BIND_TRANCIEVER);
        pdu->header.set_sequenceNumber(c.get_dialogId());
        pdu->header.set_commandStatus(makeSmppStatus((uint32_t)c.status));
        BindCommand& bnd=c.get_bindCommand();
        pdu->set_systemId(bnd.sysId.c_str());
        pdu->set_password(bnd.pass.c_str());
        pdu->set_interfaceVersion(0x34);
        pdu->set_systemType(bnd.systemType.c_str());
        int ton=0,npi=0;
        char addr[64]={0,};
        sscanf(bnd.addrRange.c_str(),".%d.%d.%64s",&ton,&npi,addr);
        pdu->get_addressRange().set_typeOfNumber(ton);
        pdu->get_addressRange().set_numberingPlan(npi);
        pdu->get_addressRange().set_value(addr);
        return reinterpret_cast<SmppHeader*>(pdu.release());
      }
    case BIND_RECIEVER_RESP:
      {
        auto_ptr<PduBindTRXResp> pdu(new PduBindTRXResp);
        pdu->header.set_commandId(SmppCommandSet::BIND_RECIEVER_RESP);
        pdu->header.set_sequenceNumber(c.get_dialogId());
        pdu->header.set_commandStatus(c.status);
        return reinterpret_cast<SmppHeader*>(pdu.release());
      }
    case BIND_TRANSMITTER_RESP:
      {
        auto_ptr<PduBindTRXResp> pdu(new PduBindTRXResp);
        pdu->header.set_commandId(SmppCommandSet::BIND_TRANSMITTER_RESP);
        pdu->header.set_sequenceNumber(c.get_dialogId());
        pdu->header.set_commandStatus(c.status);
        return reinterpret_cast<SmppHeader*>(pdu.release());
      }
    case BIND_TRANCIEVER_RESP:
      {
        auto_ptr<PduBindTRXResp> pdu(new PduBindTRXResp);
        pdu->header.set_commandId(SmppCommandSet::BIND_TRANCIEVER_RESP);
        pdu->header.set_sequenceNumber(c.get_dialogId());
        pdu->header.set_commandStatus(c.status);
        return reinterpret_cast<SmppHeader*>(pdu.release());
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

  SmsResp::SmsResp() : messageId(0), status(0),dataSm(false),sms(0), dir(dsdUnknown)
  {
    expiredUid = 0;
    expiredResp = false;
    bHasOrgCmd = false;
  };
  SmsResp::~SmsResp()
  {
    if ( messageId ) delete messageId;
    if (sms) delete sms;
  }

  void SmsResp::setOrgCmd(SmppCommand& o)
  {
    orgCmd = o; bHasOrgCmd = true;
  }

  void SmsResp::getOrgCmd(SmppCommand& o)
  {
    o = orgCmd;
  }

} //smpp
} //transport
} //scag
