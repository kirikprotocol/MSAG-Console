#include "DcpServer.hpp"
#include "informer/data/UserInfo.h"

namespace eyeline{
namespace informer{
namespace dcp{

/*
 * limit conn per ip
 * auth timeout
 * total conn count
 * conn per sec limit per ip/total
 */

eyeline::protogen::ProtocolSocketBase* DcpServer::onConnect(net::Socket* clnt,int connId)
{
  return new DcpClientSocket(clnt,connId);
}

void DcpServer::mkOkResponse(int connId,int seqNum)
{
  messages::OkResponse resp;
  resp.messageSetSeqNum(seqNum);
  enqueueCommand(connId,resp,proto,false);
}

void DcpServer::mkFailResponse(int connId,int seq,int status,const std::string& msg)
{
  messages::FailResponse resp;
  resp.setStatus(status);
  resp.setStatusMessage(msg);
  resp.messageSetSeqNum(seq);
  enqueueCommand(connId,resp,proto,false);
}


DcpClientSocket* DcpServer::getSocketByConnId(int connId)
{
  SocketsMap::iterator it=clnts.find(connId);
  if(it==clnts.end())
  {
    throw smsc::util::Exception("connecttion %d not found",connId);
  }
  return (DcpClientSocket*)it->second;
}

void DcpServer::onHandleCommand(eyeline::protogen::ProtocolSocketBase::Packet& pkt)
{
  try{
    bool auth;
    {
      sync::MutexGuard mg(clntsMon);
      DcpClientSocket* dcs=getSocketByConnId(pkt.connId);
      auth=dcs->isAuthorized();
    }
    if(!auth)
    {
      eyeline::protogen::framework::SerializerBuffer sb;
      sb.setExternalData(pkt.data,pkt.dataSize);
      int32_t tag=sb.readInt32();
      int32_t seq=sb.readInt32();
      if(tag!=messages::UserAuth::messageGetTag())
      {
        mkFailResponse(pkt.connId,seq,DcpError::NotAuthorized,"");
        return;
      }
    }
    proto.decodeAndHandleMessage(pkt.data,pkt.dataSize,pkt.connId);
  }catch(InfosmeException& e)
  {
    eyeline::protogen::framework::SerializerBuffer sb;
    sb.setExternalData(pkt.data,pkt.dataSize);
    sb.skip(4);//tag
    int err=DcpError::CommandHandlingError;
    switch(e.getCode())
    {
      case EXC_NOTAUTH:err=DcpError::NotAuthorized;break;
      case EXC_ACCESSDENIED:err=DcpError::AdminRoleRequired;break;
      case EXC_DLVLIMITEXCEED:err=DcpError::TooManyDeliveries;break;
    }
    mkFailResponse(pkt.connId,sb.readInt32(),err,e.what());
  }
  catch(std::exception& e)
  {
    eyeline::protogen::framework::SerializerBuffer sb;
    sb.setExternalData(pkt.data,pkt.dataSize);
    sb.skip(4);//tag
    mkFailResponse(pkt.connId,sb.readInt32(),DcpError::CommandHandlingError,e.what());
  }
}

void DcpServer::onDisconnect(eyeline::protogen::ProtocolSocketBase* sck)
{
  DcpClientSocket* dcpSck=(DcpClientSocket*)sck;
  if(dcpSck->isAuthorized())
  {
    sync::MutexGuard mg(usersMtx);
    const char* userId=dcpSck->getUserInfo()->getUserId();
    UsersMap::iterator it=usersMap.find(userId);
    if(it!=usersMap.end())
    {
      it->second.connCount--;
      smsc_log_debug(log,"Decreased connCount for userId=%s => %d",userId,it->second.connCount);
      if(it->second.connCount==0)
      {
        smsc_log_debug(log,"Deleting user conn info for userId=%s",userId);
        usersMap.erase(it);
      }
    }else
    {
      smsc_log_warn(log,"user conn info not found for userId=%s",dcpSck->getUserInfo()->getUserId());
    }
  }else
  {
    smsc_log_debug(log,"Deleting connId=%d, not authorized conn",sck->getConnId());
  }
  delete sck;
}


void DcpServer::handle(const messages::UserAuth& inmsg)
{
  UserInfoPtr ui=core->getUserInfo(inmsg.getUserId().c_str());
  int connId=inmsg.messageGetConnId();
  if(!ui.get() || inmsg.getPassword()!=ui->getPassword())
  {
    mkFailResponse(connId,inmsg.messageGetSeqNum(),DcpError::AuthFailed,"user not found or password mismatch");
    return;
  }
  {
    sync::MutexGuard mg(clntsMon);
    getSocketByConnId(connId)->setAuthorized(true);
  }
  mkOkResponse(connId,inmsg.messageGetSeqNum());
}


void DcpServer::handle(const messages::GetUserStats& inmsg)
{
  UserInfoPtr ui;
  {
    sync::MutexGuard mg(clntsMon);
    DcpClientSocket* sck=getSocketByConnId(inmsg.messageGetConnId());
    if(!checkAuth(sck,inmsg))
    {
      return;
    }
    if(inmsg.hasUserId())
    {
      if(!checkAdmin(sck,inmsg))
      {
        return;
      }
      ui=core->getUserInfo(inmsg.getUserId().c_str());
    }else
    {
      ui=sck->getUserInfo();
    }
  }
  messages::GetUserStatsResp resp;
  {
    sync::MutexGuard mg(usersMtx);
    UsersMap::iterator it=usersMap.find(ui->getUserId());
    if(it==usersMap.end())
    {
      resp.setCurrentConnectionsCount(0);
    }else
    {
      resp.setCurrentConnectionsCount((int16_t)it->second.connCount);
    }
  }
  UserDlvStats stats;
  ui->getStats(stats);
  resp.setCurrentActiveDeliveriesCount(stats.active);
  resp.setCurrentCancelledDeliveriesCount(stats.cancelled);
  resp.setCurrentFinishedDeliveriesCount(stats.finished);
  resp.setCurrentPausedDeliveriesCount(stats.paused);
  resp.setCurrentPlannedDeliveriesCount(stats.planned);
  enqueueResp(resp,inmsg);
}

static void fillDeliveryInfoDataFromMsg(DeliveryInfoData& did,const messages::DeliveryInfo& di)
{
  did.name=di.getName();
  did.priority=di.getPriority();
  did.transactionMode=di.getTransactionMode();
  did.startDate=di.getStartDate();
  did.endDate=di.getEndDate();
  did.activePeriodEnd=di.getActivePeriodStart();
  did.activePeriodStart=di.getActivePeriodEnd();
  did.activeWeekDays=di.getActiveWeekDays();
  if(di.hasValidityDate())
  {
    did.validityDate=di.getValidityDate();
  }
  if(di.hasValidityPeriod())
  {
    did.validityPeriod=di.getValidityPeriod();
  }
  did.flash=di.getFlash();
  did.useDataSm=di.getUseDataSm();
  switch(di.getDeliveryMode().getValue())
  {
    case messages::DeliveryMode::SMS:did.deliveryMode=DLVMODE_SMS;break;
    case messages::DeliveryMode::USSD_PUSH:did.deliveryMode=DLVMODE_USSDPUSH;break;
    case messages::DeliveryMode::USSD_PUSH_VLR:did.deliveryMode=DLVMODE_USSDPUSHVLR;break;
  }
  did.owner=di.getOwner();
  did.retryOnFail=di.getRetryOnFail();
  if(di.hasRetryPolicy())
  {
    did.retryPolicy=di.getRetryPolicy();
  }
  did.replaceMessage=di.getReplaceMessage();
  if(di.hasSvcType())
  {
    did.svcType=di.getSvcType();
  }
  if(di.hasUserData())
  {
    did.userData=di.getUserData();
  }

  did.sourceAddress=di.getSourceAddress();

}

static void fillMsgFromDeliveryInfoData(messages::DeliveryInfo& di,const DeliveryInfoData& did)
{
  di.setName(did.name);
  di.setPriority(did.priority);
  di.setTransactionMode(did.transactionMode);
  di.setStartDate(did.startDate);
  di.setEndDate(did.endDate);
  di.setActivePeriodStart(did.activePeriodEnd);
  di.setActivePeriodEnd(did.activePeriodStart);
  di.setActiveWeekDays(did.activeWeekDays);
  if(!did.validityDate.empty())
  {
    di.setValidityDate(did.validityDate);
  }
  if(!did.validityPeriod.empty())
  {
    di.setValidityPeriod(did.validityPeriod);
  }
  di.setFlash(did.flash);
  di.setUseDataSm(did.useDataSm);
  switch(did.deliveryMode)
  {
    case DLVMODE_SMS:di.setDeliveryMode(messages::DeliveryMode::SMS);break;
    case DLVMODE_USSDPUSH:di.setDeliveryMode(messages::DeliveryMode::USSD_PUSH);break;
    case DLVMODE_USSDPUSHVLR:di.setDeliveryMode(messages::DeliveryMode::USSD_PUSH_VLR);break;
  }
  di.setOwner(did.owner);
  di.setRetryOnFail(did.retryOnFail);
  if(!did.retryPolicy.empty())
  {
    di.setRetryPolicy(did.retryPolicy);
  }
  di.setReplaceMessage(did.replaceMessage);
  if(!did.svcType.empty())
  {
    di.setSvcType(did.svcType);
  }
  if(!did.userData.empty())
  {
    di.setUserData(did.userData);
  }

  di.setSourceAddress(did.sourceAddress);

}


void DcpServer::handle(const messages::CreateDelivery& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryInfoData did;
  const messages::DeliveryInfo& di=inmsg.getInfo();
  fillDeliveryInfoDataFromMsg(did,di);

  dlvid_type id=core->addDelivery(*ui.get(),did);

  messages::CreateDeliveryResp resp;
  resp.setDeliveryId(id);
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::ModifyDelivery& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryInfoData did;
  const messages::DeliveryInfo& di=inmsg.getInfo();
  fillDeliveryInfoDataFromMsg(did,di);
  core->getDelivery(*ui.get(),inmsg.getDeliveryId())->updateDlvInfo(did);
  mkOkResponse(inmsg);
}

void DcpServer::handle(const messages::DropDelivery& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  core->deleteDelivery(*ui.get(),inmsg.getDeliveryId());
  mkOkResponse(inmsg);
}

/*
static msgtime_type parseDate(const std::string& str)
{
  int year,mon,day;
  int hour,min,sec;
  if(sscanf(str.c_str(),"%d.%d.%d %d:%d:%d",&day,&mon,&year,&hour,&min,&sec)!=6)
  {
    throw InfosmeException(EXC_GENERIC,"Failed to parse date in DD:MM:YY hh:mm:ss format:%s",str.c_str());
  }
  year+=2000;
  ulonglong ymd=((((year*100+mon)*100+day)*100+hour)*100+min)*100+sec;
  return (msgtime_type)ymdToMsgTime(ymd);
}
*/

void DcpServer::handle(const messages::ChangeDeliveryState& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DlvState state;
  msgtime_type date=0;
  switch(inmsg.getState().getStatus().getValue())
  {
    case messages::DeliveryStatus::Active:state=DLVSTATE_ACTIVE;break;
    case messages::DeliveryStatus::Cancelled:state=DLVSTATE_CANCELLED;break;
    case messages::DeliveryStatus::Finished:state=DLVSTATE_FINISHED;break;
    case messages::DeliveryStatus::Paused:state=DLVSTATE_PAUSED;break;
    case messages::DeliveryStatus::Planned:
      state=DLVSTATE_PLANNED;
      if(inmsg.getState().hasDate())
      {
        date=parseDateTime(inmsg.getState().getDate().c_str());
      }
      break;
  }
  core->getDelivery(*ui.get(),inmsg.getDeliveryId())->setState(state,date);
  mkOkResponse(inmsg);
}

void DcpServer::handle(const messages::AddDeliveryMessages& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  MessageList lst;
  for(std::vector<messages::DeliveryMessage>::const_iterator it=inmsg.getMessages().begin(),end=inmsg.getMessages().end();it!=end;it++)
  {
    lst.push_back(MessageLocker());
    MessageLocker& ml=lst.back();
    ml.msg.subscriber=parseAddress(it->getAbonent().c_str());
    if(it->getMsgType()==messages::MessageType::TextMessage)
    {
      ml.msg.text=new MessageText(it->getText().c_str(),0);
    }else
    {
      ml.msg.text=new MessageText(0,it->getIndex());
    }
    ml.msg.userData=it->getUserData().c_str();
  }
  dlv->addNewMessages(lst.begin(),lst.end());
  messages::AddDeliveryMessagesResp resp;
  std::vector<int64_t>& ids=resp.getMessageIdsRef();
  for(MessageList::iterator it=lst.begin(),end=lst.end();it!=end;++it)
  {
    ids.push_back(it->msg.msgId);
  }
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::DropDeliveryMessages& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  //TODO: drop messages
  //dlv->

  mkOkResponse(inmsg);
}

void DcpServer::handle(const messages::GetDeliveryGlossary& inmsg)
{

}

void DcpServer::handle(const messages::ModifyDeliveryGlossary& inmsg)
{

}

void DcpServer::handle(const messages::GetDeliveryState& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  const DeliveryInfo& di=dlv->getDlvInfo();
  messages::GetDeliveryStateResp resp;
  messages::DeliveryState& respState=resp.getStateRef();
  messages::DeliveryStatistics& respStats=resp.getStatsRef();
  //TODO: fill state and stats
  //switch(dlv->get)
  //
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::GetDeliveryInfo& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  const DeliveryInfo& di=dlv->getDlvInfo();
  const DeliveryInfoData& did=di.getDeliveryData();
  messages::GetDeliveryInfoResp resp;
  fillMsgFromDeliveryInfoData(resp.getInfoRef(),did);
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::GetDeliveriesList& inmsg)
{

}

void DcpServer::handle(const messages::GetDeliveriesListNext& inmsg)
{

}

void DcpServer::handle(const messages::CountDeliveries& inmsg)
{

}

void DcpServer::handle(const messages::RequestMessagesState& inmsg)
{

}

void DcpServer::handle(const messages::GetNextMessagesPack& inmsg)
{

}

void DcpServer::handle(const messages::CountMessages& inmsg)
{

}

void DcpServer::handle(const messages::GetDeliveryHistory& inmsg)
{

}


}
}
}
