#include "INManComm.hpp"
#include "inman/interaction/messages.hpp"
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
  reqTimeOut=30;
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
    socket=new net::Socket;
    return;
  }
  socketOk=true;
}

static void FillChargeOp(SMSId id,smsc::inman::interaction::ChargeSms& op,const SMS& sms)
{
  op.setDestinationSubscriberNumber(sms.getDestinationAddress().toString());
  op.setCallingPartyNumber(sms.getOriginatingAddress().toString());
  op.setCallingIMSI(sms.getOriginatingDescriptor().imsi);
  op.setSMSCAddress(INManComm::scAddr.toString());
  op.setSubmitTimeTZ(sms.getSubmitTime()+timezone);
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

  smsc::inman::interaction::ChargeSms op;

  op.setDialogId(dlgId);

  FillChargeOp(id,op,sms);

  smsc::inman::interaction::ObjectBuffer buf(16);
  smsc::inman::interaction::SerializerInap::getInstance()->serialize(&op,buf);
  packetWriter.enqueue(buf.get(),buf.GetPos());


  sync::MutexGuard mg(reqMtx);
  ReqData* rd=new ReqData;
  rd->id=id;
  rd->chargeType=ReqData::ctSubmit;
  rd->sms=sms;
  rd->sbmCtx=new smsc::smeman::INSmsChargeResponse::SubmitContext(ctx);
  ReqDataMap::iterator it=reqDataMap.insert(ReqDataMap::value_type(dlgId,rd)).first;
  it->second->tmIt=timeMap.insert(TimeMap::value_type(time(NULL)+reqTimeOut,it));

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

  smsc::inman::interaction::ChargeSms op;

  op.setDialogId(dlgId);

  FillChargeOp(id,op,sms);

  smsc::inman::interaction::ObjectBuffer buf(16);
  smsc::inman::interaction::SerializerInap::getInstance()->serialize(&op,buf);
  packetWriter.enqueue(buf.get(),buf.GetPos());


  sync::MutexGuard mg(reqMtx);
  ReqData* rd=new ReqData;
  rd->id=id;
  rd->chargeType=ReqData::ctForward;
  rd->sms=sms;
  rd->fwdCtx=new smsc::smeman::INFwdSmsChargeResponse::ForwardContext(ctx);
  ReqDataMap::iterator it=reqDataMap.insert(ReqDataMap::value_type(dlgId,rd)).first;
  it->second->tmIt=timeMap.insert(TimeMap::value_type(time(NULL)+reqTimeOut,it));
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
  smsc::inman::interaction::DeliverySmsResult res
    (
      sms.lastResult==Status::OK?
        smsc::inman::interaction::DELIVERY_SUCCESSED:
        smsc::inman::interaction::DELIVERY_FAILED,
      final
    );
  smsc::inman::interaction::ObjectBuffer buf(0);
  smsc::inman::interaction::SerializerInap::getInstance()->serialize(&res,buf);
  packetWriter.enqueue(buf.get(),buf.GetPos());
}


void INManComm::ProcessExpiration()
{
  MutexGuard mg(reqMtx);
  time_t now=time(NULL);
  debug2(log,"ProcessExpiration called:%d/%d",now,timeMap.empty()?0:timeMap.begin()->first);
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
    }else
    {
      cmd=smsc::smeman::SmscCommand::makeINFwdSmsChargeResponse
        (
          it->second->second->id,
          it->second->second->sms,
          *it->second->second->fwdCtx,
          0
        );
    }
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
      socket=new Socket();
      continue;
    }
    packetSize=ntohl(packetSize);
    buf.setSize(packetSize);
    if(socket->ReadAll(buf.get(),packetSize)<=0)
    {
      info2(log,"Socket read failed:%d",errno);
      socketOk=false;
      socket->Close();
      socket=new Socket();
      continue;
    }
    std::auto_ptr<smsc::inman::interaction::SerializableObject> obj;
    try{
      buf.SetPos(0);
      obj=std::auto_ptr<smsc::inman::interaction::SerializableObject>
        (
          smsc::inman::interaction::SerializerInap::getInstance()->deserialize(buf)
        );
    }catch(std::exception& e)
    {
      warn2(log,"Failed to deserialize buffer:%s",e.what());
      socket->Close();
      socketOk=false;
      socket=new Socket();
      continue;
    }
    if(obj->getObjectId()!=smsc::inman::interaction::CHARGE_SMS_RESULT_TAG)
    {
      info2(log,"Unknown object id:%d",obj->getObjectId());
      continue;
    }
    smsc::inman::interaction::ChargeSmsResult* result=(smsc::inman::interaction::ChargeSmsResult*)obj.get();

    smsc::smeman::SmscCommand cmd;
    {
      sync::MutexGuard mg(reqMtx);
      ReqDataMap::iterator it=reqDataMap.find(result->getDialogId());
      if(it==reqDataMap.end())
      {
        info2(log,"Request for response with dlgId=%d not found",result->getDialogId());
        continue;
      }
      if(it->second->chargeType==ReqData::ctSubmit)
      {
        cmd=smsc::smeman::SmscCommand::makeINSmsChargeResponse
          (
            it->second->id,
            it->second->sms,
            *it->second->sbmCtx,
            result->GetValue()==smsc::inman::interaction::CHARGING_POSSIBLE
          );
      }else
      {
        cmd=smsc::smeman::SmscCommand::makeINFwdSmsChargeResponse
          (
            it->second->id,
            it->second->sms,
            *it->second->fwdCtx,
            result->GetValue()==smsc::inman::interaction::CHARGING_POSSIBLE
          );
      }
      timeMap.erase(it->second->tmIt);
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
