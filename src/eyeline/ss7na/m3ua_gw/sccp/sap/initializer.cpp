#include "eyeline/ss7na/common/io_dispatcher/MessagesFactory.hpp"
#include "eyeline/ss7na/common/io_dispatcher/MessagesInstancer.hpp"
#include "eyeline/ss7na/common/sccp_sap/MessagesHandlingDispatcher.hpp"

#include "eyeline/ss7na/libsccp/messages/BindMessage.hpp"
#include "eyeline/ss7na/libsccp/messages/UnbindMessage.hpp"
#include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"

#include "MessageHandlers.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace sap {

bool
registerMessageCreators()
{
  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     libsccp::BindMessage().getMsgCode(),
     new common::io_dispatcher::MessagesInstancer<common::sccp_sap::BindMessage_HandlingDispatcher<MessageHandlers> >(),
     PROTOCOL_LIBSCCP
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     libsccp::N_UNITDATA_REQ_Message().getMsgCode(),
     new common::io_dispatcher::MessagesInstancer<common::sccp_sap::N_UNITDATA_REQ_Message_HandlingDispatcher<MessageHandlers> >(),
     PROTOCOL_LIBSCCP
     );

  common::io_dispatcher::MessagesFactory::getInstance().registerMessageCreator
    (
     libsccp::UnbindMessage().getMsgCode(),
     new common::io_dispatcher::MessagesInstancer<common::sccp_sap::UnbindMessage_HandlingDispatcher<MessageHandlers> >(),
     PROTOCOL_LIBSCCP
     );
  return true;
}

}}}}}
