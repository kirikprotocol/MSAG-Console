#include <memory>

#include "UnbindRequest_Subscriber.hpp"
#include "Publisher.hpp"
#include "SMPP_Unbind.hpp"

#include "SMPPSessionSwitch.hpp"
#include "SMPPSession.hpp"
#include "SessionCache.hpp"
#include "BufferedOutputStream.hpp"
#include "PendingOutDataQueue.hpp"
#include "SessionHelper.hpp"
#include "SMPP_Constants.hpp"

namespace smpp_dmplx {

UnbindRequest_Subscriber::UnbindRequest_Subscriber()
  : _log(smsc::logger::Logger::getInstance("msg_hndlr")) {}

UnbindRequest_Subscriber::~UnbindRequest_Subscriber() {}

smpp_dmplx::SMPP_Subscriber::handle_result_t
smpp_dmplx::UnbindRequest_Subscriber::handle(std::auto_ptr<smpp_dmplx::SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_message::UNBIND ) {
    SMPP_Unbind* unbindRequest = dynamic_cast<SMPP_Unbind*>(smpp.get());

    smsc_log_info(_log,"UnbindRequest_Subscriber::handle::: got UNBIND message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), unbindRequest->toString().c_str());

    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);

    if ( sessionSearchResult.first == true ) {
      // По значению сокета нашли в кэше SMPP сессию.
      // Найденная сессия может быть как сессией с SMSC, так и сессией с SME.
      SMPPSession& smppSession = sessionSearchResult.second;
      std::string systemId = smppSession.getSystemId();

      // По значению сессии пытаемся найти сессию с SMSC. Если нашли,
      // то наша сессия - это сессия с SME, иначе это сессия с SMSC.
      std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
        SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);

      std::auto_ptr<SMPP_Unbind_Resp> unbindResponse(unbindRequest->prepareResponse(ESME_ROK));
      std::auto_ptr<BufferedOutputStream> outBuf = unbindResponse->marshal();
      smsc_log_info(_log,"UnbindRequest_Subscriber::handle::: Send UNBIND_RESPONSE message. Message dump=[%s]", unbindResponse->toString().c_str());
      PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf, socket);
    }

    SessionHelper::dropActiveSession(socket);
    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}

}
