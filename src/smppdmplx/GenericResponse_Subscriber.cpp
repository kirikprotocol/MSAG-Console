#include <memory>

#include "GenericResponse_Subscriber.hpp"
#include "SMPP_GenericResponse.hpp"
#include "SMPPSession.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SessionCache.hpp"
#include "CacheOfSMPP_message_traces.hpp"
#include "BufferedOutputStream.hpp"
#include "SocketPool_Singleton.hpp"
#include "PendingOutDataQueue.hpp"
#include "SessionHelper.hpp"

namespace smpp_dmplx {

GenericResponse_Subscriber::GenericResponse_Subscriber()
  : _log(smsc::logger::Logger::getInstance("msg_hndlr")) {}

GenericResponse_Subscriber::~GenericResponse_Subscriber()
{}

SMPP_Subscriber::handle_result_t
GenericResponse_Subscriber::handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_GenericResponse::GENERIC_RESPONSE ) {
    SMPP_GenericResponse* genericResponse = static_cast<SMPP_GenericResponse*>(smpp.get());

    smsc_log_info(_log,"GenericResponse_Subscriber::handle::: got GENERIC_RESPONSE message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), genericResponse->toString().c_str());

    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);

    if ( sessionSearchResult.first == true ) {
      // Нашли в кэше сессию, соответствующую сокету, на котором 
      // получен запрос.
      // Найденная сессия может быть как сессией с SMSC, так и сессией с SME.
      SMPPSession& smppSession = sessionSearchResult.second;

      // По значению сессии пытаемся найти сессию с SMSC. Если нашли,
      // то наша сессия - это сессия с SME, иначе это сессия с SMSC.
      std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
        SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);

      if ( sessonToSMSC_searchResult.first == true ) {
        forwardMessageToSmsc(genericResponse, smppSession, sessonToSMSC_searchResult.second);
      } else 
        forwardMessageToSme(genericResponse, smppSession);
    } else
      // Для сокета, на котором получен запрос, нет зарегистрированной сессии.
      // Херем транспортное соединение  и  удаляем сокет из пула.
      SessionHelper::terminateSessionToSme(socket);

    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}

void
GenericResponse_Subscriber::forwardMessageToSmsc(SMPP_GenericResponse* genericResponse,
                                                 SMPPSession& smppSessionToSme,
                                                 SMPPSession& sessionToSMSC)
{
  if ( smppSessionToSme.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
    // SME прислал прикладное сообщение, но сессия находится в состоянии,
    // не позволяющем принимать прикладные запросы. Удаляем сессию
    // из кэша, херем транспортное соединение для этого SME и 
    // удаляем сокет из пула.
    SessionHelper::terminateSessionToSme(smppSessionToSme.getSocketToPeer());
  else {
    // Ответ, представленный объектом genericResponse - это ответ на запрос,
    // переданный из SMSC в SME.
    std::auto_ptr<BufferedOutputStream> outBuf = genericResponse->marshal();
    smsc_log_info(_log,"GenericResponse_Subscriber::forwardMessageToSmsc::: send GENERIC_RESPONSE message to SMSC. Message dump=[%s]", genericResponse->toString().c_str());
    PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf,sessionToSMSC.getSocketToPeer());
  }
}

void
GenericResponse_Subscriber::forwardMessageToSme(SMPP_GenericResponse* genericResponse,
                                                SMPPSession& sessionToSmsc)
{
  if ( sessionToSmsc.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
    // SMSC прислал запрос, но сессия находится в состоянии,
    // не позволяющем принимать прикладные запросы. Удаляем сессию
    // из кэша, херем транспортное соединение с SMSC и 
    // удаляем сокет из пула.
    SessionHelper::terminateSessionToSmsc(sessionToSmsc.getSocketToPeer(), sessionToSmsc.getSystemId());
  else {
    // Ответ, представленный объектом genericResponse - это ответ на запрос,
    // переданный из SME в SMSC.
    // Ищем след запроса в кэше.
    try {
      CacheOfSMPP_message_traces::MessageTrace_t messageTrace = 
        CacheOfSMPP_message_traces::getInstance().getMessageTraceFromCache(genericResponse->getSequenceNumber(), sessionToSmsc.getSystemId());
      genericResponse->setSequenceNumber(messageTrace.first);
      std::auto_ptr<BufferedOutputStream> outBuf = genericResponse->marshal();
      smsc_log_info(_log,"GenericResponse_Subscriber::forwardMessageToSme::: send GENERIC_RESPONSE message to SME. Message dump=[%s]", genericResponse->toString().c_str());
      PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf,messageTrace.second.getSocketToPeer());
    } catch (MessageTraceNotFound& ex) {
      smsc_log_info(_log,"GenericResponse_Subscriber::forwardMessageToSme: Catched exception [%s]", ex.what());
    }
  }
}

}
