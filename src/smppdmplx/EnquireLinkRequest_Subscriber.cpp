#include <memory>

#include "EnquireLinkRequest_Subscriber.hpp"
#include "SMPP_EnquireLink.hpp"
#include "SMPP_EnquireLink_Resp.hpp"
#include "SMPP_Constants.hpp"
#include "SMPPSession.hpp"
#include "SessionCache.hpp"
#include "SocketPool_Singleton.hpp"
#include "PendingOutDataQueue.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SequenceNumberGenerator.hpp"
#include "CacheOfSMPP_message_traces.hpp"

namespace smpp_dmplx {

EnquireLinkRequest_Subscriber::EnquireLinkRequest_Subscriber()
  : _log(smsc::logger::Logger::getInstance("msg_hndlr")) {}

EnquireLinkRequest_Subscriber::~EnquireLinkRequest_Subscriber() {}

SMPP_Subscriber::handle_result_t
EnquireLinkRequest_Subscriber::handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_message::ENQUIRE_LINK ) {
    SMPP_EnquireLink* enquireRequest = static_cast<SMPP_EnquireLink*>(smpp.get());

    smsc_log_info(_log,"EnquireLinkRequest_Subscriber::handle::: got ENQUIRE_LINK_REQUEST message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), enquireRequest->toString().c_str());

    SessionCache::search_result_t searchResult =
      SessionCache::getInstance().getSession(socket);

    if ( searchResult.first == false ) {
      // Для сокета, на котором получен запрос, нет зарегистрированной сессии.
      // Херем транспортное соединение  и  удаляем сокет из пула.
      SocketPool_Singleton::getInstance().remove_socket(socket);
    }
    SMPPSession& smppSession = searchResult.second;
    // Проверяем, что сессия с SME/SMSC находится в допустимом состоянии - к примеру, не закрыта
    if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS ) {
      // SME/SMSC прислал запрос EnquireLink, но сессия находится в состоянии,
      // не позволяющем принимать прикладные запросы. Удаляем сессию
      // из кэша, херем транспортное соединение для этого SME/SMSC и 
      // удаляем сокет из пула.
      SessionCache::getInstance().removeSession(socket);
      SocketPool_Singleton::getInstance().remove_socket(socket);
    } else {
      std::auto_ptr<SMPP_EnquireLink_Resp>
        enquireResponse(enquireRequest->prepareResponse(ESME_ROK));
      std::auto_ptr<BufferedOutputStream> outBuf = enquireResponse->marshal();
      smsc_log_info(_log,"EnquireLinkRequest_Subscriber::handle::: Send ENQUIRE_LINK_RESPONSE message. Message dump=[%s]", enquireResponse->toString().c_str());
      PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf,socket);

      // По значению сессии пытаемся найти сессию с SMSC. Если нашли,
      // то наша сессия - это сессия с SME, иначе это сессия с SMSC.
      std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
        SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);

      if ( sessonToSMSC_searchResult.first == true )
        forwardMessageToSmsc(enquireRequest, smppSession, sessonToSMSC_searchResult.second);
      else
        broadcastMessageToAllSme(enquireRequest, smppSession.getSystemId());
    }
    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}

void
EnquireLinkRequest_Subscriber::forwardMessageToSmsc(SMPP_EnquireLink* enquireRequest,
                                                    SMPPSession& smppSessionToSme,
                                                    SMPPSession& sessionToSmsc)
{
  uint32_t seqNumToSmsc = SequenceNumberGenerator::getInstance(sessionToSmsc.getSystemId())->getNextValue();
  CacheOfSMPP_message_traces::getInstance().putMessageTraceToCache(enquireRequest->getSequenceNumber(), smppSessionToSme, seqNumToSmsc);
  enquireRequest->setSequenceNumber(seqNumToSmsc);

  std::auto_ptr<BufferedOutputStream> outBuf = enquireRequest->marshal();
  smsc_log_info(_log,"EnquireLinkRequest_Subscriber::forwardMessageToSmsc::: send ENQUIRE_LINK_REQUEST message to SMSC. Message dump=[%s]", enquireRequest->toString().c_str());
  PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf, sessionToSmsc.getSocketToPeer());
}

void
EnquireLinkRequest_Subscriber::broadcastMessageToAllSme(SMPP_EnquireLink* enquireRequest,
                                                        const std::string& systemId)
{
  smsc_log_debug(_log, "EnquireLinkRequest_Subscriber::broadcastMessageToAllSme::: broadcast ENQUIRE_LINK_REQUEST to all connected SME");
  SMPPSessionSwitch::getInstance().broadcastMessageToAllSme(systemId, *enquireRequest);
}

}
