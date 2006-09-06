#include <memory>

#include "GenericResponse_Subscriber.hpp"
#include "Publisher.hpp"
#include "SMPP_GenericResponse.hpp"
#include "SMPPSession.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SessionCache.hpp"
#include "CacheOfSMPP_message_traces.hpp"
#include "BufferedOutputStream.hpp"
#include "SocketPool_Singleton.hpp"
#include "PendingOutDataQueue.hpp"
#include "SessionHelper.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

static int toRegisterSubscriber() {
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::GenericResponse_Subscriber());

  return 0;
}

static int subscriberIsRegistered = toRegisterSubscriber();

smpp_dmplx::GenericResponse_Subscriber::~GenericResponse_Subscriber()
{}

smpp_dmplx::SMPP_Subscriber::handle_result_t
smpp_dmplx::GenericResponse_Subscriber::handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_GenericResponse::GENERIC_RESPONSE ) {
    SMPP_GenericResponse* genericResponse = dynamic_cast<SMPP_GenericResponse*>(smpp.get());

    smsc_log_info(dmplxlog,"GenericResponse_Subscriber::handle::: got GENERIC_RESPONSE message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), genericResponse->toString().c_str());

    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);

    if ( sessionSearchResult.first == true ) {
      smsc_log_debug(dmplxlog,"GenericResponse_Subscriber::handle::: socket for session=[%s]", sessionSearchResult.second.getSocketToPeer().toString().c_str());
      // Нашли в кэше сессию, соответствующую сокету, на котором 
      // получен запрос.
      // Найденная сессия может быть как сессией с SMSC, так и сессией с SME.
      SMPPSession& smppSession = sessionSearchResult.second;

      // По значению сессии пытаемся найти сессию с SMSC. Если нашли,
      // то наша сессия - это сессия с SME, иначе это сессия с SMSC.
      std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
        SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);

      if ( sessonToSMSC_searchResult.first == true ) {
        if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
          // SME прислал прикладно сообщение, но сессия находится в состоянии,
          // не позволяющем принимать прикладные запросы. Удаляем сессию
          // из кэша, херем транспортное соединение для этого SME и 
          // удаляем сокет из пула.
          SessionHelper::terminateSessionToSme(socket);
        else {
          SMPPSession& sessionToSMSC = sessonToSMSC_searchResult.second;
          // Нашли сессию с SMSC. Можем переслать ответ в SMSC.
          // Ответ, представленный объектом smpp - это ответ на запрос,
          // переданный из SMSC в SME.
          std::auto_ptr<BufferedOutputStream> outBuf = genericResponse->marshal();
          smsc_log_info(dmplxlog,"GenericResponse_Subscriber::handle::: send GENERIC_RESPONSE message to SMSC. Message dump=[%s]", genericResponse->toString().c_str());
          PendingOutDataQueue::scheduleDataForSending(*outBuf,sessionToSMSC.getSocketToPeer());
        }
      } else {
        if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
          // SMSC прислал запрос, но сессия находится в состоянии,
          // не позволяющем принимать прикладные запросы. Удаляем сессию
          // из кэша, херем транспортное соединение с SMSC и 
          // удаляем сокет из пула.
          SessionHelper::terminateSessionToSmsc(socket, smppSession.getSystemId());
        else {
          // Объект, на который ссылается smppSession, соответствует сессии
          // с SMSC. Следовательно ответ необходимо отправить в SME.
          // Ответ, представленный объектом smpp - это ответ на запрос,
          // переданный из SME в SMSC.
          // Ищем след запроса в кэше.
          CacheOfSMPP_message_traces::MessageTrace_t messageTrace = 
            CacheOfSMPP_message_traces::getInstance().getMessageTraceFromCache(smpp->getSequenceNumber(), smppSession.getSystemId());
          genericResponse->setSequenceNumber(messageTrace.first);
          std::auto_ptr<BufferedOutputStream> outBuf = genericResponse->marshal();
          smsc_log_info(dmplxlog,"GenericResponse_Subscriber::handle::: send GENERIC_RESPONSE message to SME. Message dump=[%s]", genericResponse->toString().c_str());
          PendingOutDataQueue::scheduleDataForSending(*outBuf,messageTrace.second.getSocketToPeer());
        }
      }
    } else {
      // Ну это чиста проделки Фикса! Бред, короче.
      // Херем транспортное соединение  и  удаляем сокет из пула.
      SessionHelper::terminateSessionToSme(socket);
    }
    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}
