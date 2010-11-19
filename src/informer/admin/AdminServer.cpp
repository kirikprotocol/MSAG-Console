#include "AdminServer.hpp"

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
  }catch(std::exception& e)
  {
    resp.setStatus(1);
    resp.setMsg(e.what());
  }
  enqueueCommand(cmd.messageGetConnId(),resp,proto,false);
}

void AdminServer::handle(const messages::SetDefaultSmsc& cmd)
{
  messages::SetDefaultSmscResp resp;
  resp.messageSetSeqNum(cmd.messageGetSeqNum());
  try{
    core->updateDefaultSmsc(cmd.getId().c_str());
    resp.setStatus(0);
  }catch(std::exception& e)
  {
    resp.setStatus(1);
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
  int code=0; //TODO:send sms in sync mode
  messages::SendTestSmsResp resp;
  resp.messageSetSeqNum(cmd.messageGetSeqNum());
  resp.setRespCode(code);
  enqueueCommand(cmd.messageGetConnId(),resp,proto,false);
}


}
}
}
