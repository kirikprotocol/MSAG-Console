/* "$Id$" */
#include "inman/interaction/msgbill/MsgBilling.hpp"
#include "util/Exception.hpp"
#include "smsc/smsc.hpp"

#include "INManComm.hpp"

namespace smsc{
namespace inmancomm{

Address INManComm::scAddr;

INManComm::INManComm(smsc::smeman::SmeRegistrar* argSmeReg):smeReg(argSmeReg)
{
  smeManMon=0;
  log=smsc::logger::Logger::getInstance("inmancom");
  systemId="INMANCOMM";
  dlgIdSeq=0;
  reqTimeOut=60;
  socket=new net::Socket;
  packetWriter.assignSocket(socket);
  socketOk=false;
  packetWriter.Start();
}


void INManComm::Init(const char* argHost,int argPort)
{
  host=argHost;
  port=argPort;

  smsc::inman::interaction::INPSerializer::getInstance()->
      registerCmdSet(smsc::inman::interaction::INPCSBilling::getInstance());

  if(socket->Init(host.c_str(),port,0)==-1)
  {
    throw smsc::util::Exception("Failed to resolve %s:%d",host.c_str(),port);
    return;
  }
  if(socket->Connect()==-1)
  {
    warn2(log,"Failed to connect to %s:%d",host.c_str(),port);
    return;
  }
  socketOk=true;
}

template <class OpClass>
static void FillChargeOp(SMSId id,OpClass& op,const SMS& sms)
{
  op.setDestinationSubscriberNumber(sms.getDealiasedDestinationAddress().toString().c_str());
  op.setCallingPartyNumber(sms.getOriginatingAddress().toString().c_str());
  op.setCallingIMSI(sms.getOriginatingDescriptor().imsi);
  op.setSMSCAddress(INManComm::scAddr.toString().c_str());
  op.setSubmitTimeTZ(sms.getSubmitTime());
  op.setTPShortMessageSpecificInfo(0x11);
  op.setTPProtocolIdentifier(sms.getIntProperty(Tag::SMPP_PROTOCOL_ID));
  op.setTPDataCodingScheme(sms.getIntProperty(Tag::SMPP_DATA_CODING));
  op.setTPValidityPeriod(sms.getValidTime()-time(NULL));
  op.setLocationInformationMSC(sms.getOriginatingDescriptor().msc);
  op.setCallingSMEid(sms.getSourceSmeId());
  op.setRouteId(sms.getRouteId());
  op.setServiceId(sms.getServiceId());
  op.setUserMsgRef(sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)?sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)&0xffff:-1);
  op.setMsgId(id);
  op.setServiceOp(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)?sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP):-1);
  op.setServiceType(sms.getEServiceType());
  bool chargeOnSubmit=false;
  if(sms.getBillingRecord()==BILLING_MT)
  {
    op.setMTcharge();
  }
  if(sms.getBillingRecord()==BILLING_CDR)
  {
    op.setForcedCDR();
    op.setChargeOnSubmit();
    chargeOnSubmit=true;
  }
  if(sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY)==Smsc::chargeOnSubmit)
  {
    op.setChargeOnSubmit();
    chargeOnSubmit=true;
  }
  if(chargeOnSubmit)
  {
    op.setPartsNum(1);
  }else
  {
    op.setPartsNum(sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM));
  }

  if(sms.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
  {
    unsigned len;
    sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
    op.setMsgLength(len);
  }else
  {
    unsigned len;
    sms.getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    op.setMsgLength(len);
  }
#ifdef SMSEXTRA
  if(sms.hasIntProperty(Tag::SMSC_EXTRAFLAGS))
  {
    op.setSmsXSrvs(sms.getIntProperty(Tag::SMSC_EXTRAFLAGS));
  }
#endif
}


void INManComm::ChargeSms(SMSId id,const SMS& sms,smsc::smeman::INSmsChargeResponse::SubmitContext& ctx)
{
  if(!socketOk)
  {
    throw smsc::util::Exception("Communication with inman failed");
  }

  int dlgId=getNewDlgId();
  info2(log,"ChargeSms smsid=%lld,dlgid=%d, cp=%d",id,dlgId,sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY));
  ctx.inDlgId=dlgId;

  smsc::inman::interaction::SPckChargeSms pck;
  pck.Hdr().dlgId = dlgId;
  FillChargeOp(id, pck.Cmd(), sms);

  smsc::inman::interaction::ObjectBuffer buf(400);
  pck.serialize(buf);


  debug2(log,"Buffer size:%d",buf.getDataSize());
  {
    ReqData* rd=new ReqData;
    rd->id=id;
    rd->chargeType=ReqData::ctSubmit;
    rd->sms=sms;
    rd->sbmCtx=new smsc::smeman::INSmsChargeResponse::SubmitContext(ctx);
    sync::MutexGuard mg(reqMtx);
    ReqDataMap::iterator it=reqDataMap.insert(ReqDataMap::value_type(dlgId,rd)).first;
    it->second->tmIt=timeMap.insert(TimeMap::value_type(time(NULL)+reqTimeOut,it));
    debug2(log,"ChargeSms: reqDataMap.size()=%d",reqDataMap.size());
  }
  packetWriter.enqueue((const char*)buf.get(),buf.getDataSize());

  //smsc::smeman::SmscCommand cmd=smsc::smeman::SmscCommand::makeINSmsChargeResponse(id,sms,ctx);
  //sync::MutexGuard mg(queueMtx);
  //queue.Push(cmd);
}

void INManComm::ChargeSms(SMSId id,const SMS& sms,smsc::smeman::INFwdSmsChargeResponse::ForwardContext& ctx)
{
  if(!socketOk)
  {
    throw smsc::util::Exception("Communication with inman failed");
  }

  int dlgId=getNewDlgId();
  info2(log,"ChargeSmsFwd smsid=%lld, dlgid=%d, cp=%d",id,dlgId,sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY));
  ctx.inDlgId=dlgId;

  smsc::inman::interaction::SPckChargeSms pck;
//  pck.Cmd().setForwarded();
  pck.Hdr().dlgId = dlgId;
  FillChargeOp(id, pck.Cmd(), sms);
  smsc::inman::interaction::ObjectBuffer buf(400);
  pck.serialize(buf);
  {
    ReqData* rd=new ReqData;
    rd->id=id;
    rd->chargeType=ReqData::ctForward;
    rd->sms=sms;
    sync::MutexGuard mg(reqMtx);
    rd->fwdCtx=new smsc::smeman::INFwdSmsChargeResponse::ForwardContext(ctx);
    ReqDataMap::iterator it=reqDataMap.insert(ReqDataMap::value_type(dlgId,rd)).first;
    it->second->tmIt=timeMap.insert(TimeMap::value_type(time(NULL)+reqTimeOut,it));
    debug2(log,"ChargeSmsFwd: reqDataMap.size()=%d",reqDataMap.size());
  }
  packetWriter.enqueue((const char*)buf.get(),buf.getDataSize());
  /*
  smsc::smeman::SmscCommand cmd=smsc::smeman::SmscCommand::makeINFwdSmsChargeResponse(id,sms,contex);
  sync::MutexGuard mg(queueMtx);
  queue.Push(cmd);
  info2(log,"FwdChargeSms %lld",id);
  */
}

void INManComm::FullReport(SMSId id,const SMS& sms)
{
  if(!socketOk)
  {
    throw smsc::util::Exception("Communication with inman failed");
  }
  smsc::inman::interaction::SPckDeliveredSmsData pck;
  pck.Hdr().dlgId=getNewDlgId();
  info2(log,"FullReport: smsId=%lld, dlgId=%d, cp=%d",id,pck.Hdr().dlgId,sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY));
  FillChargeOp(id,pck.Cmd(),sms);
  pck.Cmd().setResultValue(sms.lastResult);
  pck.Cmd().setDestIMSI(sms.getDestinationDescriptor().imsi);
  pck.Cmd().setDestMSC(sms.getDestinationDescriptor().msc);
  pck.Cmd().setDestSMEid(sms.getDestinationSmeId());
  pck.Cmd().setDeliveryTime(time(NULL));
  if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
    pck.Cmd().setDivertedAdr(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());
  smsc::inman::interaction::ObjectBuffer buf(200);
  pck.serialize(buf);
  packetWriter.enqueue((const char*)buf.get(),buf.getDataSize());
}


void INManComm::Report(int dlgId,const SMS& sms,bool final)
{
  info2(log,"Report:dlgid=%d, lr=%d, cp=%d, final=%s",dlgId,sms.lastResult,sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY),final?"Y":"N");

  smsc::inman::interaction::SPckDeliverySmsResult pck;
  pck.Hdr().dlgId = dlgId;
  pck.Cmd().setResultValue(sms.lastResult);
  pck.Cmd().setDestIMSI(sms.getDestinationDescriptor().imsi);
  pck.Cmd().setDestMSC(sms.getDestinationDescriptor().msc);
  pck.Cmd().setDestSMEid(sms.getDestinationSmeId());
  pck.Cmd().setDeliveryTime(time(NULL));
  pck.Cmd().setFinal(final);
  if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
    pck.Cmd().setDivertedAdr(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());

  smsc::inman::interaction::ObjectBuffer buf(200);
  pck.serialize(buf);

  packetWriter.enqueue((const char*)buf.get(),buf.getDataSize());
}


void INManComm::ProcessExpiration()
{
  MutexGuard mg(reqMtx);
  time_t now=time(NULL);
  //debug2(log,"ProcessExpiration called:%d/%d",now,timeMap.empty()?0:timeMap.begin()->first);
  while(!timeMap.empty() && timeMap.begin()->first<now)
  {
    TimeMap::iterator it=timeMap.begin();
    info2(log,"Processing expired request for msgId=%lld",it->second->second->id);
    smsc::smeman::SmscCommand cmd;
    if(it->second->second->chargeType==ReqData::ctSubmit)
    {
      cmd=smsc::smeman::SmscCommand::makeINSmsChargeResponse
        (
          it->second->second->id,
          it->second->second->sms,
          *it->second->second->sbmCtx,
          0
        );
      delete it->second->second->sbmCtx;
    }else
    {
      cmd=smsc::smeman::SmscCommand::makeINFwdSmsChargeResponse
        (
          it->second->second->id,
          it->second->second->sms,
          *it->second->second->fwdCtx,
          0
        );
      delete it->second->second->fwdCtx;
    }
    delete it->second->second;
    reqDataMap.erase(it->second);
    timeMap.erase(it);
    sync::MutexGuard mg2(queueMtx);
    queue.Push(cmd);
  }
}

int INManComm::Execute()
{
  smsc::inman::interaction::ObjectBuffer buf(0);
  time_t lastExpire=time(NULL);
  while(!isStopping)
  {
    while(!socketOk && !isStopping)
    {
      ProcessExpiration();
      sync::MutexGuard mg(mon);
      socket=new net::Socket();
      packetWriter.assignSocket(socket);
      if(socket->Init(host.c_str(),port,0)==-1 || socket->Connect()==-1)
      {
        warn2(log,"reconnect attempt to %s:%d failed",host.c_str(),port);
        mon.wait(5000);
        continue;
      }else
      {
        socketOk=true;
      }
    }

    do{
      time_t now=time(NULL);
      if(now!=lastExpire)
      {
        ProcessExpiration();
        lastExpire=now;
      }
    }
    while(!socket->canRead(1) && !isStopping);

    uint32_t packetSize;
    if(socket->ReadAll((char*)&packetSize,4)<=0)
    {
      info2(log,"Socket len read failed:%d",errno);
      socketOk=false;
      socket->Close();
      sleep(2);
      continue;
    }
    packetSize=ntohl(packetSize);
    if(packetSize>65536)
    {
      warn2(log,"Packet size too big:%d",packetSize);
      socketOk=false;
      socket->Close();
      sleep(2);
      continue;
    }
    buf.reset(packetSize);
    if(socket->ReadAll((char*)buf.get(),packetSize)<=0)
    {
      info2(log,"Socket packet read failed:%d",errno);
      socketOk=false;
      socket->Close();
      sleep(2);
      continue;
    }
    buf.setDataSize(packetSize);
    buf.setPos(0);
    std::auto_ptr<smsc::inman::interaction::INPPacketAC> pck;
    try {
        pck.reset(smsc::inman::interaction::INPSerializer::getInstance()->deserialize(buf));
        if ((pck->pHdr())->Id() != smsc::inman::interaction::INPCSBilling::HDR_DIALOG)
            throw smsc::util::Exception("unsupported Inman packet header");
        (pck->pCmd())->loadDataBuf();
    }catch(std::exception& e)
    {
      warn2(log,"Failed to deserialize buffer:%s",e.what());
      socket->Close();
      socketOk=false;
      sleep(2);
      continue;
    }

    if ((pck->pCmd())->Id() != smsc::inman::interaction::INPCSBilling::CHARGE_SMS_RESULT_TAG)
    {
      info2(log,"Unknown object id:%d", (pck->pCmd())->Id());
      continue;
    }

    smsc::inman::interaction::CsBillingHdr_dlg * hdr =
        static_cast<smsc::inman::interaction::CsBillingHdr_dlg*>(pck->pHdr());
    smsc::inman::interaction::ChargeSmsResult* result =
        static_cast<smsc::inman::interaction::ChargeSmsResult*>(pck->pCmd());

    smsc::smeman::SmscCommand cmd;
    std::auto_ptr<ReqData> rd;
    {
      sync::MutexGuard mg(reqMtx);

      ReqDataMap::iterator it=reqDataMap.find(hdr->dlgId);
      if(it==reqDataMap.end())
      {
        info2(log,"Request for response with dlgId=%d not found", hdr->dlgId);
        continue;
      }
      rd.reset(it->second);
      timeMap.erase(it->second->tmIt);
      reqDataMap.erase(it);
    }
    debug2(log,"Received response for dlgId=%d, value=%d", hdr->dlgId, result->GetValue());
    if(rd->chargeType==ReqData::ctSubmit)
    {
      cmd=smsc::smeman::SmscCommand::makeINSmsChargeResponse
          (
              rd->id,
              rd->sms,
              *rd->sbmCtx,
              result->GetValue()==smsc::inman::interaction::ChargeSmsResult::CHARGING_POSSIBLE
          );
      cmd->get_chargeSmsResp()->inmanError=result->getMsg();
      cmd->get_chargeSmsResp()->contractType=result->getContract();
      delete rd->sbmCtx;
    }else
    {
      cmd=smsc::smeman::SmscCommand::makeINFwdSmsChargeResponse
          (
              rd->id,
              rd->sms,
              *rd->fwdCtx,
              result->GetValue()==smsc::inman::interaction::ChargeSmsResult::CHARGING_POSSIBLE
          );
      cmd->get_fwdChargeSmsResp()->inmanError=result->getMsg();
      delete rd->fwdCtx;
    }
    {
      sync::MutexGuard mg(queueMtx);
      queue.Push(cmd);
    }
  }
  socket->Abort();
  return 0;
}

}//system
}//smsc
