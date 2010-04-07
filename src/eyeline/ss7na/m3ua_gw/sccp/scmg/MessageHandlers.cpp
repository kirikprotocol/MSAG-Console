#include "MessageHandlers.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

void
MessageHandlers::handle(const messages::SOGMessage& message)
{
  smsc_log_debug(_logger, "MessageHandlers::handle::: handle SOGMessage='%s'",
                 message.toString().c_str());
}

void
MessageHandlers::handle(const messages::SORMessage& message)
{
  smsc_log_debug(_logger, "MessageHandlers::handle::: handle SORMessage='%s'",
                 message.toString().c_str());
}

void
MessageHandlers::handle(const messages::SSAMessage& message)
{
  smsc_log_debug(_logger, "MessageHandlers::handle::: handle SSAMessage='%s'",
                 message.toString().c_str());
}

void
MessageHandlers::handle(const messages::SSCMessage& message)
{
  smsc_log_debug(_logger, "MessageHandlers::handle::: handle SSCMessage='%s'",
                 message.toString().c_str());
}

void
MessageHandlers::handle(const messages::SSPMessage& message)
{
  smsc_log_debug(_logger, "MessageHandlers::handle::: handle SSPMessage='%s'",
                 message.toString().c_str());
}

void
MessageHandlers::handle(const messages::SSTMessage& message)
{
  smsc_log_debug(_logger, "MessageHandlers::handle::: handle SSTMessage='%s'",
                 message.toString().c_str());
}

}}}}}
