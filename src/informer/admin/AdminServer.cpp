#include "AdminServer.hpp"
#include "informer/data/CoreSmscStats.h"

namespace eyeline{
namespace informer{
namespace admin{

eyeline::protogen::ProtocolSocketBase* AdminServer::onConnect(net::Socket* clnt,int connId)
{
  return new eyeline::protogen::ProtocolSocketBase(clnt,connId);
}

void AdminServer::onHandleCommand(eyeline::protogen::ProtocolSocketBase::Packet& pkt)
{
  proto.decodeAndHandleMessage(pkt.data,pkt.dataSize,pkt.connId);
}

void AdminServer::onDisconnect(eyeline::protogen::ProtocolSocketBase* sck)
{
  delete sck;
}


void AdminServer::handle(const messages::ConfigOp& cmd)
{
  messages::ConfigOpResult resp;
  resp.messageSetSeqNum(cmd.messageGetSeqNum());
  try{
    if(!core)
    {
      throw InfosmeException(EXC_SYSTEM,"Not ready yet");
    }
    switch(cmd.getCfgId().getValue())
    {
      case messages::ConfigId::ciRegion:
      {
        switch(cmd.getOp().getValue())
        {
          case messages::ConfigOpId::coAdd:core->addRegion(cmd.getObjId());break;
          case messages::ConfigOpId::coRemove:core->deleteRegion(cmd.getObjId());break;
          case messages::ConfigOpId::coUpdate:core->updateRegion(cmd.getObjId());break;
        }
      }break;
      case messages::ConfigId::ciSmsc:
      {
        switch(cmd.getOp().getValue())
        {
          case messages::ConfigOpId::coAdd:core->addSmsc(cmd.getObjName().c_str());break;
          case messages::ConfigOpId::coRemove:core->deleteSmsc(cmd.getObjName().c_str());break;
          case messages::ConfigOpId::coUpdate:core->updateSmsc(cmd.getObjName().c_str());break;
        }
      }break;
      case messages::ConfigId::ciUser:
      {
        switch(cmd.getOp().getValue())
        {
          case messages::ConfigOpId::coAdd:core->addUser(cmd.getObjName().c_str());break;
          case messages::ConfigOpId::coRemove:core->deleteUser(cmd.getObjName().c_str());break;
          case messages::ConfigOpId::coUpdate:core->updateUserInfo(cmd.getObjName().c_str());break;
        }
      }break;
    }
    resp.setStatus(0);
  }catch(InfosmeException& e)
  {
    resp.setStatus(e.getCode());
    resp.setMsg(e.what());
  }catch(std::exception& e)
  {
    resp.setStatus(EXC_GENERIC);
    resp.setMsg(e.what());
  }
  enqueueCommand(cmd.messageGetConnId(),resp,proto,false);
}

void AdminServer::handle(const messages::SetDefaultSmsc& cmd)
{
  messages::SetDefaultSmscResp resp;
  resp.messageSetSeqNum(cmd.messageGetSeqNum());
  try{
    if(!core)
    {
      throw InfosmeException(EXC_SYSTEM,"Not ready yet.");
    }
    core->updateDefaultSmsc(cmd.getId().c_str());
    resp.setStatus(0);
  }catch(InfosmeException& e)
  {
    resp.setStatus(e.getCode());
  }catch(std::exception& e)
  {
    resp.setStatus(EXC_GENERIC);
  }

  enqueueCommand(cmd.messageGetConnId(),resp,proto,false);
}


void AdminServer::handle(const messages::LoggerGetCategories& cmd)
{
  std::auto_ptr<const smsc::logger::Logger::LogLevels> cats(smsc::logger::Logger::getLogLevels());
  char * k;
  messages::LoggerGetCategoriesResp resp;
  smsc::logger::Logger::LogLevel level;
  std::vector<messages::CategoryInfo>& result=resp.getCategoriesRef();
  for (smsc::logger::Logger::LogLevels::Iterator i = cats->getIterator(); i.Next(k, level); )
  {
    messages::CategoryInfo ci;
    ci.setName(k);
    ci.setLevel(smsc::logger::Logger::getLogLevel(level));
    result.push_back(ci);
  }
  resp.messageSetSeqNum(cmd.messageGetSeqNum());
  resp.setStatus(0);
  enqueueCommand(cmd.messageGetConnId(),resp,proto,false);
}

void AdminServer::handle(const messages::LoggerSetCategories& cmd)
{
  const std::vector<messages::CategoryInfo>& cats=cmd.getCategories();
  smsc::logger::Logger::LogLevels levels;
  for(std::vector<messages::CategoryInfo>::const_iterator it=cats.begin(),end=cats.end();it!=end;it++)
  {
    levels[it->getName().c_str()] = smsc::logger::Logger::getLogLevel(it->getLevel().c_str());
  }
  smsc::logger::Logger::setLogLevels(levels);
  messages::LoggerSetCategoriesResp resp;
  resp.messageSetSeqNum(cmd.messageGetSeqNum());
  resp.setStatus(0);
  enqueueCommand(cmd.messageGetConnId(),resp,proto,false);
}

void AdminServer::handle(const messages::SendTestSms& cmd)
{
  int code;
  messages::SendTestSmsResp resp;
  resp.messageSetSeqNum(cmd.messageGetSeqNum());
  try{
    if(!core)
    {
      throw InfosmeException(EXC_SYSTEM,"Not ready yet.");
    }
    personid_type abnt=parseAddress(cmd.getAbonent().c_str());
    DlvMode dlvMode;
    switch(cmd.getDeliveryMode().getValue())
    {
      case messages::DeliveryMode::SMS:dlvMode=DLVMODE_SMS;break;
      case messages::DeliveryMode::USSD_PUSH:dlvMode=DLVMODE_USSDPUSH;break;
      case messages::DeliveryMode::USSD_PUSH_VLR:dlvMode=DLVMODE_USSDPUSHVLR;break;
      default:throw InfosmeException(EXC_GENERIC,"Invalid delivery mode value:%d",cmd.getDeliveryMode().getValue());
    }
    code=core->sendTestSms(cmd.getSourceAddr().c_str(),abnt,cmd.getText().c_str(),cmd.getFlash(),dlvMode);
  }catch(InfosmeException& e)
  {
    code=-e.getCode();
  }catch(std::exception& e)
  {
    code=-EXC_GENERIC;
  }
  resp.setRespCode(code);
  smsc_log_debug(log,"sendTestSms resp: %s",resp.toString().c_str());
  enqueueCommand(cmd.messageGetConnId(),resp,proto,false);
}


void AdminServer::handle(const messages::GetSmscStats& cmd)
{
  messages::GetSmscStatsResp resp;
  resp.messageSetSeqNum(cmd.messageGetSeqNum());
  try{
    if(!core)
    {
      throw InfosmeException(EXC_SYSTEM,"Not ready yet.");
    }
    std::vector< CoreSmscStats > stats;
    core->getSmscStats(stats);
    std::vector<messages::SmscStats>& vec = resp.getSmscStatsRef();
    vec.reserve(stats.size());
    for ( std::vector< CoreSmscStats >::const_iterator i = stats.begin(), iend = stats.end();
          i != iend; ++i ) {
        vec.push_back(messages::SmscStats());
        messages::SmscStats& res = vec.back();
        res.setSmscId(i->smscId);
        res.setLiveTime(i->liveTime);
        res.setNumberOfRegions(i->nRegions);
        res.setMaxBandwidth(i->maxBandwidth);
        res.setAveragingPeriod(i->avgInterval);
        res.setCurrentLoad(i->currentLoad);
        res.setPendingResponses(i->nResponses);
        res.setPendingReceipts(i->nReceipts);
    }
    resp.setStatus(0);
  }catch(InfosmeException& e)
  {
    resp.setStatus(e.getCode());
  }catch(std::exception& e)
  {
    resp.setStatus(EXC_GENERIC);
  }
  enqueueCommand(cmd.messageGetConnId(),resp,proto,false);
}

}
}
}
