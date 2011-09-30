/* "@(#)$Id$" */

#include "util/Exception.hpp"
#include "system/smsc.hpp"
//#include "system/INManComm.hpp"
#include "inman/interaction/msgbill/MsgBilling.hpp"
using smsc::inman::interaction::INPBilling;

namespace smsc{
namespace system{

static const INPBilling  protoDef; //Sms/USSd billing protocol definition

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
  info2(log,"ChargeSms: Id=%lld;dda=%s;dlgid=%d;cp=%d",id,sms.getDealiasedDestinationAddress().toString().c_str(),
      dlgId,sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY));
  ctx.inDlgId=dlgId;

  smsc::inman::interaction::SPckChargeSms pck;
  pck._Hdr.dlgId = dlgId;
  FillChargeOp(id, pck._Cmd, sms);

  smsc::inman::interaction::PacketBuffer_T<400> buf;
  pck.serialize(buf); //throws

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
  info2(log,"ChargeSmsFwd: Id=%lld;dda=%s;dlgid=%d;cp=%d",id,sms.getDealiasedDestinationAddress().toString().c_str(),
      dlgId,sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY));
  ctx.inDlgId=dlgId;

  smsc::inman::interaction::SPckChargeSms pck;
//  pck.Cmd().setForwarded();
  pck._Hdr.dlgId = dlgId;
  FillChargeOp(id, pck._Cmd, sms);

  smsc::inman::interaction::PacketBuffer_T<400> buf;
  pck.serialize(buf); //throws
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
  pck._Hdr.dlgId=getNewDlgId();
  info2(log,"FullReport: Id=%lld;dda=%s;dlgId=%d;cp=%d",id,sms.getDealiasedDestinationAddress().toString().c_str(),
      pck._Hdr.dlgId,sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY));
  FillChargeOp(id,pck._Cmd,sms);
  pck._Cmd.setResultValue(sms.lastResult);
  pck._Cmd.setDestIMSI(sms.getDestinationDescriptor().imsi);
  pck._Cmd.setDestMSC(sms.getDestinationDescriptor().msc);
  pck._Cmd.setDestSMEid(sms.getDestinationSmeId());
  pck._Cmd.setDeliveryTime(time(NULL));
  if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
    pck._Cmd.setDivertedAdr(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());

  smsc::inman::interaction::PacketBuffer_T<400> buf;
  pck.serialize(buf);
  packetWriter.enqueue((const char*)buf.get(),buf.getDataSize());
}


void INManComm::Report(SMSId id,int dlgId,const SMS& sms,bool final)
{
  info2(log,"Report: Id=%lld;dda=%s;dlgid=%d;lr=%d;cp=%d;final=%s",id,sms.getDealiasedDestinationAddress().toString().c_str(),
      dlgId,sms.lastResult,sms.getIntProperty(Tag::SMSC_CHARGINGPOLICY),final?"Y":"N");

  smsc::inman::interaction::SPckDeliverySmsResult pck;
  pck._Hdr.dlgId = dlgId;
  pck._Cmd.setResultValue(sms.lastResult);
  pck._Cmd.setDestIMSI(sms.getDestinationDescriptor().imsi);
  pck._Cmd.setDestMSC(sms.getDestinationDescriptor().msc);
  pck._Cmd.setDestSMEid(sms.getDestinationSmeId());
  pck._Cmd.setDeliveryTime(time(NULL));
  pck._Cmd.setFinal(final);
  if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
    pck._Cmd.setDivertedAdr(sms.getStrProperty(Tag::SMSC_DIVERTED_TO).c_str());

  smsc::inman::interaction::PacketBuffer_T<200> buf;
  pck.serialize(buf); //throws

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
  smsc::inman::interaction::PacketBuffer_T<200> buf;

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
    //check PDUid and deserialize buffer
    smsc::inman::interaction::SPckChargeSmsResult pck;
    try
    {
      INPBilling::PduId pduId = protoDef.isKnownPacket(buf);
      if (!pduId)
        throw smsc::util::Exception("unsupported INMan packet recieved");

      INPBilling::CommandTag_e
        cmdId = static_cast<INPBilling::CommandTag_e>(protoDef.getCmdId(pduId));

      if (cmdId != INPBilling::CHARGE_SMS_RESULT_TAG) {
        smsc_log_warn(log, "illegal INMan command received: %s", INPBilling::nameOfCmd(cmdId));
        continue;
      }
      pck.deserialize(buf, smsc::inman::interaction::SerializablePacketIface::dsmComplete); //throws
    } catch(const std::exception & exc)
    {
      smsc_log_warn(log, "Failed to deserialize buffer: %s", exc.what());
      socket->Close();
      socketOk = false;
      sleep(2);
      continue;
    }
    smsc::inman::interaction::INPBillingHdr_dlg * hdr = &pck._Hdr;
    smsc::inman::interaction::ChargeSmsResult * result = &pck._Cmd;

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
