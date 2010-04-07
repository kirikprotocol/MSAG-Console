#ifndef __EYELINE_SS7NA_SUAGW_SCCPSAP_MESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_SUAGW_SCCPSAP_MESSAGEHANDLERS_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"

# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/common/sccp_sap/MessageHandlers.hpp"

# include "eyeline/ss7na/libsccp/messages/BindMessage.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"
# include "eyeline/ss7na/libsccp/messages/UnbindMessage.hpp"

# include "eyeline/ss7na/sua_gw/io_dispatcher/ConnectMgr.hpp"
# include "eyeline/ss7na/sua_gw/sccp_sap/LinkSetInfoRegistry.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/CLDTMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {

class MessageHandlers : public common::sccp_sap::MessageHandlers {
public:
  MessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sccp_sap")),
    _cMgr(io_dispatcher::ConnectMgr::getInstance()),
    _linkSetInfoRegistry(LinkSetInfoRegistry::getInstance()) {}

  virtual void handle(const libsccp::BindMessage& message, const common::LinkId& link_id);
  virtual void handle(const libsccp::N_UNITDATA_REQ_Message& message,
                      const common::LinkId& link_id);
  virtual void handle(const libsccp::UnbindMessage& message, const common::LinkId& link_id);

private:
  sua_stack::messages::CLDTMessage createCLDTMessage(const libsccp::N_UNITDATA_REQ_Message& message);
  smsc::logger::Logger* _logger;
  io_dispatcher::ConnectMgr& _cMgr;
  LinkSetInfoRegistry& _linkSetInfoRegistry;
};

}}}}

#endif
