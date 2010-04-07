#include "SCMGMessageProcessor.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "eyeline/ss7na/common/TP.hpp"
#include "eyeline/ss7na/common/MessageHandlingDispatcherIface.hpp"
#include "eyeline/ss7na/common/io_dispatcher/MessagesFactory.hpp"

#include "messages/SCMGMessage.hpp"
#include "MessagesFactory.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

void
SCMGMessageProcessor::handle(const utilx::variable_data_t& scmg_msg_data)
{
  if ( scmg_msg_data.dataLen < MIN_MESSAGE_SIZE )
    throw utilx::DeserializationException("SCMGMessageProcessor::handle::: input SCMG message is too short, len=%u, min len=%u",
                                          scmg_msg_data.dataLen, MIN_MESSAGE_SIZE);
  messages::SCMGMessage* scmgMessage = MessagesFactory::getInstance().instanceMessage(*scmg_msg_data.data);
  if ( scmgMessage ) {
    common::TP tp(0, scmg_msg_data.dataLen, const_cast<uint8_t*>(scmg_msg_data.data), scmg_msg_data.dataLen);
    scmgMessage->deserialize(tp);
    MessageHandlingDispatcherIface* msgHandleDisp = scmgMessage->getHandlingDispatcher();
    if ( !msgHandleDisp ) {
      smsc_log_error(_logger, "SCMGMessageProcessor::handle::: fatal error - message handler isn't set for message type=%u",
                     scmgMessage->getMsgCode());
      throw smsc::util::Exception("SCCPMessageProcessor::handle::: can't handle scmg message");
    }
    msgHandleDisp->dispatch_handle();
  } else
    throw smsc::util::Exception("SCMGMessageProcessor::handle::: can't instantiate SCMG message for msg_code [=%u]",
                                *scmg_msg_data.data);
}

}}}}}
