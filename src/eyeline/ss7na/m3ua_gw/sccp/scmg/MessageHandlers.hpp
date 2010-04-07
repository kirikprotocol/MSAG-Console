#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGEHANDLERS_HPP__

# include "logger/Logger.h"

# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SOGMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SORMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SSAMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SSCMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SSPMessage.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SSTMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

class MessageHandlers {
public:
  MessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("sccp"))
  {}

  void handle(const messages::SOGMessage& message);
  void handle(const messages::SORMessage& message);
  void handle(const messages::SSAMessage& message);
  void handle(const messages::SSCMessage& message);
  void handle(const messages::SSPMessage& message);
  void handle(const messages::SSTMessage& message);

private:
  smsc::logger::Logger* _logger;
};

}}}}}

#endif
