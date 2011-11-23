#ifndef __SMSC_MCISME_ADMIN_ADMINSERVER_HPP__
# define __SMSC_MCISME_ADMIN_ADMINSERVER_HPP__

# include "eyeline/protogen/server/ServerBase.hpp"
# include "mcisme/admin/MCAServerProtocolHandler.hpp"
# include "mcisme/admin/MCAServerProtocol.hpp"
# include "mcisme/TaskProcessor.h"
# include "logger/Logger.h"

namespace smsc {
namespace mcisme {
namespace admin {

class AdminServer : public eyeline::protogen::ServerBase,
                    public MCAServerProtocolHandler {
public:
  explicit AdminServer(TaskProcessor& processor)
  : ServerBase("admin"), _processor(processor),
    _logger(logger::Logger::getInstance("admin"))
  {
    _proto.assignHandler(this);
  }

protected:
  virtual eyeline::protogen::ProtocolSocketBase* onConnect(core::network::Socket* clnt,int connId);
  virtual void onHandleCommand(eyeline::protogen::ProtocolSocketBase::Packet& pkt);
  virtual void onDisconnect(eyeline::protogen::ProtocolSocketBase* sck);

  void handle(const messages::FlushStats& msg);
  void handle(const messages::GetStats& msg);
  void handle(const messages::GetRunStats& msg);
  void handle(const messages::GetSched& msg);
  void handle(const messages::GetSchedList& msg);
  void handle(const messages::GetProfile& msg);
  void handle(const messages::SetProfile& msg);

  TaskProcessor& _processor;
  MCAServerProtocol _proto;
  logger::Logger* _logger;
};

}}}

#endif
