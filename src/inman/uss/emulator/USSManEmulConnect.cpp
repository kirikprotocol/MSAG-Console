#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <inman/uss/ussmessages.hpp>

#include "USSManEmulConnect.hpp"
#include "MessagesProcessor.hpp"

namespace smsc {
namespace inman {
namespace uss {

void USSManEmulConnect::onPacketReceived(interaction::Connect* conn, // указатель на объект, обслуживающий соединение с клиентом ussman'a
                                         std::auto_ptr<interaction::SerializablePacketAC>& recv_cmd) throw(std::exception)
{
  interaction::USSPacketAC* requestPacket =
    static_cast<smsc::inman::interaction::USSPacketAC*>(recv_cmd.get());

  interaction::USSRequestMessage* requestObject = 
    static_cast<smsc::inman::interaction::USSRequestMessage*>(requestPacket->pCmd());
  smsc_log_debug(_logger, "USSManEmulConnect::onPacketReceived: got request object=[%s]",
                 requestObject->toString().c_str());

  _requestProcessor = MessagesProcessor::getInstance().processRequest(requestPacket->dialogId(), *requestObject, conn);
}

void USSManEmulConnect::onConnectError(interaction::Connect* conn, std::auto_ptr<CustomException>& p_exc)
{}

void USSManEmulConnect::close(interaction::ConnectAC* conn)
{
  smsc_log_debug(_logger, "USSManEmulConnect::close: Enter it");
  if ( _requestProcessor ) {
    _requestProcessor->cancelRequestProcessing(conn->getId());
    _requestProcessor = NULL;
  }
}

}
}
}
