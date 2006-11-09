/* "$Id$" */
#include "INManComm.hpp"
#include "inman/interaction/MsgBilling.hpp"
#include "util/Exception.hpp"

namespace smsc{
namespace system{

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
//  smsc::inman::interaction::SerializerInap::getInstance();
  smsc::inman::interaction::INPSerializer::getInstance();
  if(socket->Init(host.c_str(),port,0)==-1)
  {
    throw smsc::util::Exception("Failed to resolve %s:%d",host.c_str(),port);
    return;
  }
  if(socket->Connect()==-1)
  {
    warn2(log,"Failed to connect to %s:%d",host.c_str(),port);
    socket=new net::Socket;
    return;
  }
  socketOk=true;
}

static void FillChargeOp(SMSId id,smsc::inman::interaction::ChargeSms& op,const SMS& sms)
{
  op.setDestinationSubscriberNumber(sms.getDealiasedDestinationAddress().toString());
  op.setCallingPartyNumber(sms.getOriginatingAddress().toString());
  op.setCallingIMSI(sms.getOriginatingDescriptor().imsi);
  op.setSMSCAddress(INManComm::scAddr.toString());
  op.setSubmitTimeTZ(sms.getSubmitTime());
  op.setTPShortMessageSpecificInfo(0x11);
  op.setTPProtocolIdentifier(sms.getIntProperty(Tag::SMPP_PROTOCOL_ID));
  op.setTPDataCodingScheme(sms.getIntProperty(Tag::SMPP_DATA_CODING));
  op.setTPValidityPeriod(sms.getValidTime()-time(NULL));
  op.setLocationInformationMSC(sms.getOriginatingDescriptor().msc);
  op.setCallingSMEid(sms.getSourceSmeId());
  op.setRouteId(sms.getRouteId());
  op.setServiceId(sms.getServiceId());
  op.setUserMsgRef(sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)?sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE):-1);
  op.setMsgId(id);
  op.setServiceOp(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP)?sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP):-1);
  op.setPartsNum(sms.getIntProperty(Tag::SMSC_ORIGINALPARTSNUM));
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
  info2(log,"ChargeSms %lld/%d",id,dlgId);
  ctx.inDlgId=dlgId;

  smsc::inman::interaction::SPckChargeSms pck;
  pck.Hdr().dlgId = dlgId;
  FillChargeOp(id, pck.Cmd(), sms);
  smsc::inman::interaction::ObjectBuffer buf(400);
  pck.serialize(buf);

  debug2(log,"Buffer size:%d",buf.getDataSize());
  {
    sync::MutexGuard mg(reqMtx);
    ReqData* rd=new ReqData;
    rd->id=id;
    rd->chargeType=ReqData::ctSubmit;
    rd->sms=sms;
    rd->sbmCtx=new smsc::smeman::INSmsChargeResponse::SubmitContext(ctx);
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
  info2(log,"ChargeSmsFwd %lld/%d",id,dlgId);
  ctx.inDlgId=dlgId;

  smsc::inman::interaction::SPckChargeSms pck;
  pck.Cmd().setForwarded();
  pck.Hdr().dlgId = dlgId;
  FillChargeOp(id, pck.Cmd(), sms);
  smsc::inman::interaction::ObjectBuffer buf(400);
  pck.serialize(buf);
  {
    sync::MutexGuard mg(reqMtx);
    ReqData* rd=new ReqData;
    rd->id=id;
    rd->chargeType=ReqData::ctForward;
    rd->sms=sms;
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

void INManComm::Report(int dlgId,const SMS& sms,bool final)
{
  info2(log,"Report:%d/%d",dlgId,sms.lastResult);

  smsc::inman::interaction::SPckDeliverySmsResult pck;
  pck.Hdr().dlgId = dlgId;
  pck.Cmd().setResultValue(sms.lastResult);
  pck.Cmd().setDestIMSI(sms.getDestinationDescriptor().imsi);
  pck.Cmd().setDestMSC(sms.getDestinationDescriptor().msc);
  pck.Cmd().setDestSMEid(sms.getDestinationSmeId());
  pck.Cmd().setDeliveryTime(time(NULL));
  if(sms.hasStrProperty(Tag::SMSC_DIVERTED_TO))
    pck.Cmd().setDivertedAdr(sms.getStrProperty(Tag::SMSC_DIVERTED_TO));

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
    debug2(log,"Processing expired request for msgId=%lld",it->second->second->id);
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
    reqDataMap.erase(it->second);
    delete it->second->second;
    timeMap.erase(it);
    sync::MutexGuard mg2(queueMtx);
    queue.Push(cmd);
  }
}

int INManComm::Execute()
{
  smsc::inman::interaction::ObjectBuffer buf(0);
  while(!isStopping)
  {
    while(!socketOk && !isStopping)
    {
      ProcessExpiration();
      sync::MutexGuard mg(mon);
      if(socket->Init(host.c_str(),port,0)==-1 || socket->Connect()==-1)
      {
        warn2(log,"reconnect attempt to %s:%d failed",host.c_str(),port);
        mon.wait(5000);
        continue;
      }else
      {
        socketOk=true;
        packetWriter.assignSocket(socket);
      }
    }
    while(!socket->canRead(1) && !isStopping)
    {
      ProcessExpiration();
    }

    int packetSize;
    if(socket->ReadAll((char*)&packetSize,4)<=0)
    {
      info2(log,"Socket read failed:%d",errno);
      socketOk=false;
      socket->Close();
      socket=new net::Socket();
      continue;
    }
    packetSize=ntohl(packetSize);
    buf.reset(packetSize);
    if(socket->ReadAll((char*)buf.get(),packetSize)<=0)
    {
      info2(log,"Socket read failed:%d",errno);
      socketOk=false;
      socket->Close();
      socket=new net::Socket();
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
      socket=new net::Socket();
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
    {
      sync::MutexGuard mg(reqMtx);

      ReqDataMap::iterator it=reqDataMap.find(hdr->dlgId);
      if(it==reqDataMap.end())
      {
        info2(log,"Request for response with dlgId=%d not found", hdr->dlgId);
        continue;
      }
      debug2(log,"Received response for dlgId=%d, value=%d", hdr->dlgId, result->GetValue());
      if(it->second->chargeType==ReqData::ctSubmit)
      {
        cmd=smsc::smeman::SmscCommand::makeINSmsChargeResponse
          (
            it->second->id,
            it->second->sms,
            *it->second->sbmCtx,
            result->GetValue()==smsc::inman::interaction::ChargeSmsResult::CHARGING_POSSIBLE
          );
        delete it->second->sbmCtx;
      }else
      {
        cmd=smsc::smeman::SmscCommand::makeINFwdSmsChargeResponse
          (
            it->second->id,
            it->second->sms,
            *it->second->fwdCtx,
            result->GetValue()==smsc::inman::interaction::ChargeSmsResult::CHARGING_POSSIBLE
          );
        delete it->second->fwdCtx;
      }
      timeMap.erase(it->second->tmIt);
      delete it->second;
      reqDataMap.erase(it);
    }
    {
      sync::MutexGuard mg(queueMtx);
      queue.Push(cmd);
    }
  }
  return 0;
}

}//system
}//smsc
