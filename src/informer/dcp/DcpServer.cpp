#include "DcpServer.hpp"
#include "informer/data/UserInfo.h"
#include <time.h>
#include "informer/alm/IActivityLogMiner.hpp"
#include "informer/dcp/messages/DeliveryMessageState.hpp"

namespace eyeline{
namespace informer{
namespace dcp{

namespace {

void fillDeliveryInfoDataFromMsg(DeliveryInfoData& did,const messages::DeliveryInfo& di)
{
  did.name=di.getName();
  did.priority=di.getPriority();
  did.transactionMode=di.getTransactionMode();
  did.startDate=di.getStartDate();
  did.endDate=di.getEndDate();
  did.activePeriodEnd=di.getActivePeriodStart();
  did.activePeriodStart=di.getActivePeriodEnd();
  did.activeWeekDays=di.getActiveWeekDays();
  /*
  if(di.hasValidityDate())
  {
    did.validityDate=di.getValidityDate();
  }
  */
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
  did.finalDlvRecords=di.getFinalDlvRecords();
  did.finalMsgRecords=di.getFinalMsgRecords();
}

void fillMsgFromDeliveryInfoData(messages::DeliveryInfo& di,const DeliveryInfoData& did)
{
  di.setName(did.name);
  di.setPriority(did.priority);
  di.setTransactionMode(did.transactionMode);
  di.setStartDate(did.startDate);
  di.setEndDate(did.endDate);
  di.setActivePeriodStart(did.activePeriodEnd);
  di.setActivePeriodEnd(did.activePeriodStart);
  di.setActiveWeekDays(did.activeWeekDays);
  /*
  if(!did.validityDate.empty())
  {
    di.setValidityDate(did.validityDate);
  }
  */
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
  di.setFinalDlvRecords(did.finalDlvRecords);
  di.setFinalMsgRecords(did.finalMsgRecords);

}

std::string msgTimeToDateStr(msgtime_type t)
{
  ::tm tm;
  time_t tmp=(time_t)t;
  gmtime_r(&tmp,&tm);
  char buf[64];
  sprintf(buf,"%02d.%02d.%02d",tm.tm_mday,tm.tm_mon+1,tm.tm_year%100);
  return buf;
}

std::string msgTimeToTimeStr(msgtime_type t)
{
  int sec=t%60;
  t/=60;
  int min=t%60;
  t/=60;
  int hour=t%24;
  char buf[64];
  sprintf(buf,"%02d:%02d:%02d",hour,min,sec);
  return buf;
}

std::string msgTimeToDateTimeStr(msgtime_type t)
{
  return msgTimeToDateStr(t)+" "+msgTimeToTimeStr(t);
}

messages::DeliveryStatus dlvStateToDeliveryStatus(DlvState st)
{
  switch(st)
  {
    case DLVSTATE_PAUSED:return messages::DeliveryStatus(messages::DeliveryStatus::Paused);
    case DLVSTATE_PLANNED:return messages::DeliveryStatus(messages::DeliveryStatus::Planned);
    case DLVSTATE_ACTIVE:return messages::DeliveryStatus(messages::DeliveryStatus::Active);
    case DLVSTATE_FINISHED:return messages::DeliveryStatus(messages::DeliveryStatus::Finished);
    case DLVSTATE_CANCELLED:return messages::DeliveryStatus(messages::DeliveryStatus::Cancelled);
  }
  throw InfosmeException(EXC_GENERIC,"Unknown delivery state %d",st);
}

}

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
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  messages::GetDeliveryGlossaryResp resp;
  dlv->getGlossary(resp.getGlossaryRef().getMessagesRef());
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::ModifyDeliveryGlossary& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  dlv->setGlossary(inmsg.getGlossary().getMessages());
  mkOkResponse(inmsg);
}

void DcpServer::handle(const messages::GetDeliveryState& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  const DeliveryInfo& di=dlv->getDlvInfo();
  messages::GetDeliveryStateResp resp;
  messages::DeliveryState& respState=resp.getStateRef();
  messages::DeliveryStatistics& respStats=resp.getStatsRef();
  msgtime_type planDate;
  respState.setStatus(dlvStateToDeliveryStatus(dlv->getState(&planDate)));
  if(respState.getStatus()==messages::DeliveryStatus::Planned)
  {
    respState.setDate(msgTimeToDateTimeStr(planDate));
  }

  DeliveryStats ds;
  dlv->getStats(ds);
  respStats.setDeliveredMessages(ds.dlvdMessages);
  respStats.setExpiredMessages(ds.expiredMessages);
  respStats.setFailedMessage(ds.failedMessages);
  respStats.setNewMessages(ds.getNewMessagesCount());
  respStats.setProcessMessage(ds.procMessages);

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

#define VECLOOP(it,type,vec) for(std::vector<type>::const_iterator it=vec.begin(),end=vec.end();it!=end;++it)

static bool isDeliveryMatchFilter(Delivery* dlv,const messages::DeliveriesFilter& flt)
{
  if(flt.hasNameFilter())
  {
    std::string nm=dlv->getDlvInfo().getName();
    bool found=false;
    VECLOOP(it,std::string,flt.getNameFilter())
    {
      if(nm.find(*it)!=std::string::npos)
      {
        found=true;
        break;
      }
    }
    if(!found)
    {
      return false;
    }
  }
  if(flt.hasStatusFilter())
  {
    bool found=false;
    VECLOOP(it,messages::DeliveryStatus,flt.getStatusFilter())
    {
      if(dlv->getState(0)==it->getValue())
      {
        found=true;
        break;
      }
    }
    if(!found)
    {
      return false;
    }
  }
  if(flt.hasStartDateFrom())
  {
    msgtime_type t=parseDateTime(flt.getStartDateFrom().c_str());
    if(dlv->getDlvInfo().getStartDate()<t)
    {
      return false;
    }
  }
  if(flt.hasStartDateTo())
  {
    msgtime_type t=parseDateTime(flt.getStartDateTo().c_str());
    if(dlv->getDlvInfo().getStartDate()>t)
    {
      return false;
    }
  }
  if(flt.hasEndDateFrom())
  {
    msgtime_type t=parseDateTime(flt.getEndDateFrom().c_str());
    if(dlv->getDlvInfo().getEndDate()<t)
    {
      return false;
    }
  }
  if(flt.hasEndDateTo())
  {
    msgtime_type t=parseDateTime(flt.getEndDateTo().c_str());
    if(dlv->getDlvInfo().getEndDate()>t)
    {
      return false;
    }
  }

  return true;
}

void DcpServer::handle(const messages::GetDeliveriesList& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  std::vector<UserInfoPtr> users;
  if(!ui->hasRole(USERROLE_ADMIN))
  {
    if(inmsg.getFilter().hasUserIdFilter() && !inmsg.getFilter().getUserIdFilter().empty() &&
        (
            inmsg.getFilter().getUserIdFilter().size()!=1 ||
            inmsg.getFilter().getUserIdFilter().back()!=ui->getUserId()
        ))
    {
      mkFailResponse(inmsg,DcpError::AdminRoleRequired,"admin role required for userid filter");
      return;
    }
    users.push_back(ui);
  }else
  {
    if(inmsg.getFilter().hasUserIdFilter())
    {
      VECLOOP(it,std::string,inmsg.getFilter().getUserIdFilter())
      {
        users.push_back(core->getUserInfo(it->c_str()));
      }
    }else
    {
      core->getUsers(users);
    }
  }
  std::vector<DeliveryPtr> dlvLst;
  VECLOOP(it,UserInfoPtr,users)
  {
    UserInfo::DeliveryList lst;
    (*it)->getDeliveries(lst);
    for(UserInfo::DeliveryList::iterator dit=lst.begin(),dend=lst.end();dit!=dend;++dit)
    {
      if(isDeliveryMatchFilter(dit->get(),inmsg.getFilter()))
      {
        dlvLst.push_back(*dit);
      }
    }
  }
  messages::GetDeliveriesListResp resp;
  {
    sync::MutexGuard mg(dlvReqMtx);
    time_t now=time(0);
    while(!dlvListReqTimeMap.empty() && dlvListReqTimeMap.begin()->first<now)
    {
      int reqId=dlvListReqTimeMap.begin()->second;
      DlvListReqMap::iterator it=dlvListReqMap.find(reqId);
      if(it!=dlvListReqMap.end())
      {
        smsc_log_info(log,"deliveries list request with reqId=%d expired",reqId);
        delete it->second;
        dlvListReqMap.erase(it);
      }
      dlvListReqTimeMap.erase(dlvListReqTimeMap.begin());
    }
    int reqId=dlvListReqIdSeq++;
    smsc_log_info(log,"created deliveries list reqId=%d for connId=%d (%lu total records)",reqId,inmsg.messageGetConnId(),dlvLst.size());
    resp.setReqId(reqId);
    DlvListRequest* req=new DlvListRequest(inmsg.messageGetConnId(),reqId);
    req->dlvLst.swap(dlvLst);
    req->last=req->dlvLst.begin();
    req->timeMapIt=dlvListReqTimeMap.insert(DlvListReqTimeMap::value_type(time(0)+dlvListReqExpirationTime,reqId));
    dlvListReqMap.insert(DlvListReqMap::value_type(reqId,req));
  }
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::GetDeliveriesListNext& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  messages::GetDeliveriesListNextResp resp;
  {
    sync::MutexGuard mg(dlvReqMtx);
    DlvListReqMap::iterator it=dlvListReqMap.find(inmsg.getReqId());
    if(it==dlvListReqMap.end())
    {
      mkFailResponse(inmsg,DcpError::RequestNotFound,"request for given id not found");
      return;
    }
    DlvListRequest* req=it->second;
    if(req->connId!=inmsg.messageGetConnId())
    {
      mkFailResponse(inmsg,DcpError::RequestNotFound,"request for given id do not belong to this connection");
      return;
    }
    dlvListReqTimeMap.erase(req->timeMapIt);
    req->timeMapIt=dlvListReqTimeMap.insert(DlvListReqTimeMap::value_type(time(0)+dlvListReqExpirationTime,inmsg.getReqId()));
    std::vector<messages::DeliveryListInfo>& info=resp.getInfoRef();
    resp.setMoreDeliveries(true);
    for(int i=0;i<inmsg.getCount();i++)
    {
      info.push_back(messages::DeliveryListInfo());
      messages::DeliveryListInfo& dli=info.back();
      Delivery* dlv=req->last->get();
      const DeliveryInfo& di=dlv->getDlvInfo();
      VECLOOP(fit,messages::DeliveryFields,req->fields)
      {
        switch(fit->getValue())
        {
          case messages::DeliveryFields::ActivityPeriod:dli.setActivityPeriodEnd(msgTimeToTimeStr(di.getActivePeriodEnd()));break;
          case messages::DeliveryFields::EndDate:dli.setEndDate(msgTimeToDateTimeStr(di.getEndDate()));break;
          case messages::DeliveryFields::Name:dli.setName(di.getName());break;
          case messages::DeliveryFields::StartDate:dli.setStartDate(msgTimeToDateTimeStr(di.getStartDate()));break;
          case messages::DeliveryFields::Status:dli.setStatus(dlvStateToDeliveryStatus(dlv->getState(0)));break;
          case messages::DeliveryFields::UserData:dli.setUserData(di.getUserData());break;
          case messages::DeliveryFields::UserId:dli.setUserId(dlv->getUserInfo().getUserId());break;
        }
      }
      req->last++;
      if(req->last==req->dlvLst.end())
      {
        resp.setMoreDeliveries(false);
        dlvListReqTimeMap.erase(req->timeMapIt);
        delete req;
        dlvListReqMap.erase(it);
        break;
      }
    }
  }
  smsc_log_info(log,"next portion of deliveries list for reqId=%d for connId=%d - %lu records, mms=%s)",
      inmsg.getReqId(),inmsg.messageGetConnId(),resp.getInfo().size(),resp.getMoreDeliveries()?"true":"false");
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::CountDeliveries& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  std::vector<UserInfoPtr> users;
  if(!ui->hasRole(USERROLE_ADMIN))
  {
    if(inmsg.getFilter().hasUserIdFilter() && !inmsg.getFilter().getUserIdFilter().empty() &&
        (
            inmsg.getFilter().getUserIdFilter().size()!=1 ||
            inmsg.getFilter().getUserIdFilter().back()!=ui->getUserId()
        ))
    {
      mkFailResponse(inmsg,DcpError::AdminRoleRequired,"admin role required for userid filter");
      return;
    }
    users.push_back(ui);
  }else
  {
    if(inmsg.getFilter().hasUserIdFilter())
    {
      VECLOOP(it,std::string,inmsg.getFilter().getUserIdFilter())
      {
        users.push_back(core->getUserInfo(it->c_str()));
      }
    }else
    {
      core->getUsers(users);
    }
  }
  int result=0;
  VECLOOP(it,UserInfoPtr,users)
  {
    UserInfo::DeliveryList lst;
    (*it)->getDeliveries(lst);
    for(UserInfo::DeliveryList::iterator dit=lst.begin(),dend=lst.end();dit!=dend;++dit)
    {
      if(isDeliveryMatchFilter(dit->get(),inmsg.getFilter()))
      {
        result++;
      }
    }
  }
  messages::CountDeliveriesResp resp;
  resp.setResult(result);
  enqueueResp(resp,inmsg);
}

namespace {

template <class MSG>
void fillFilter(const MSG& inmsg,alm::ALMRequestFilter& filter)
{
  if(inmsg.hasCodeFilter())
  {
    VECLOOP(it,int32_t,inmsg.getCodeFilter())
    {
      filter.codeFilter.insert(*it);
    }
  }
  if(inmsg.hasStates())
  {
    VECLOOP(it,messages::DeliveryMessageState,inmsg.getStates())
    {
      MsgState st;
      switch(it->getValue())
      {
        case messages::DeliveryMessageState::New:st=MSGSTATE_INPUT;break;
        case messages::DeliveryMessageState::Process:st=MSGSTATE_PROCESS;break;
        case messages::DeliveryMessageState::Delivered:st=MSGSTATE_DELIVERED;break;
        case messages::DeliveryMessageState::Failed:st=MSGSTATE_FAILED;break;
        case messages::DeliveryMessageState::Expired:st=MSGSTATE_EXPIRED;break;
      }
      filter.stateFilter.insert(st);
    }
  }
  if(inmsg.hasMsisdnFilter())
  {
    VECLOOP(it,std::string,inmsg.getMsisdnFilter())
    {
      filter.abonentFilter.insert(*it);
    }
  }
  filter.resultFields=0;
}

}

void DcpServer::handle(const messages::RequestMessagesState& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  alm::ALMRequestFilter filter;
  fillFilter(inmsg,filter);
  VECLOOP(it,messages::ReqField,inmsg.getFields())
  {
    switch(it->getValue())
    {
      case messages::ReqField::Abonent:filter.resultFields|=alm::rfAbonent;break;
      case messages::ReqField::Date:filter.resultFields|=alm::rfDate;break;
      case messages::ReqField::ErrorCode:filter.resultFields|=alm::rfErrorCode;break;
      case messages::ReqField::State:filter.resultFields|=alm::rfState;break;
      case messages::ReqField::Text:filter.resultFields|=alm::rfText;break;
      case messages::ReqField::UserData:filter.resultFields|=alm::rfUserData;break;
    }
  }
  messages::RequestMessagesStateResp resp;
  resp.setReqId(core->getALM().createRequest(inmsg.getDeliveryId(),filter));
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::GetNextMessagesPack& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  messages::GetNextMessagesPackResp resp;
  std::vector<alm::ALMResult> result;
  resp.setMoreMessages(core->getALM().getNext(inmsg.getReqId(),result,inmsg.getCount()));
  std::vector<messages::MessageInfo>& miv=resp.getInfoRef();
  VECLOOP(it,alm::ALMResult,result)
  {
    miv.push_back(messages::MessageInfo());
    messages::MessageInfo& mi=miv.back();
    if(it->resultFields&alm::rfAbonent)
    {
      mi.setAbonent(it->abonent.c_str());
    }
    if(it->resultFields&alm::rfDate)
    {
      mi.setDate(msgTimeToDateTimeStr(it->date));
    }
    if(it->resultFields&alm::rfErrorCode)
    {
      mi.setErrorCode(it->code);
    }
    if(it->resultFields&alm::rfState)
    {
      switch(it->state)
      {
        case MSGSTATE_INPUT:mi.setState(messages::DeliveryMessageState::New);break;
        case MSGSTATE_PROCESS:mi.setState(messages::DeliveryMessageState::Process);break;
        case MSGSTATE_DELIVERED:mi.setState(messages::DeliveryMessageState::Delivered);break;
        case MSGSTATE_FAILED:mi.setState(messages::DeliveryMessageState::Failed);break;
        case MSGSTATE_EXPIRED:mi.setState(messages::DeliveryMessageState::Expired);break;
      }
    }
    if(it->resultFields&alm::rfText)
    {
      mi.setText(it->text);
    }
    if(it->resultFields&alm::rfUserData)
    {
      mi.setUserData(it->userData);
    }
  }
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::CountMessages& inmsg)
{
  UserInfoPtr ui=getUserInfo(inmsg);
  DeliveryPtr dlv=core->getDelivery(*ui,inmsg.getDeliveryId());
  alm::ALMRequestFilter filter;
  fillFilter(inmsg,filter);
  messages::CountMessagesResp resp;
  resp.setCount(core->getALM().countRecords(inmsg.getDeliveryId(),filter));
  enqueueResp(resp,inmsg);
}

void DcpServer::handle(const messages::GetDeliveryHistory& inmsg)
{

}


}
}
}
