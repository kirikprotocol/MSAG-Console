#include <memory>

#include "GenericRequest_Subscriber.hpp"
#include "Publisher.hpp"
#include "SMPP_GenericRequest.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SMPPSession.hpp"
#include "SessionCache.hpp"
#include "BufferedOutputStream.hpp"
#include "SocketPool_Singleton.hpp"
#include "CacheOfSMPP_message_traces.hpp"
#include "PendingOutDataQueue.hpp"
#include "SessionHelper.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

static int toRegisterSubscriber() {
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::GenericRequest_Subscriber());

  return 0;
}

static int subscriberIsRegistered = toRegisterSubscriber();

smpp_dmplx::GenericRequest_Subscriber::~GenericRequest_Subscriber() {}

smpp_dmplx::SMPP_Subscriber::handle_result_t
smpp_dmplx::GenericRequest_Subscriber::handle(std::auto_ptr<smpp_dmplx::SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_GenericRequest::GENERIC_REQUEST ) {
    SMPP_GenericRequest* genericRequest = dynamic_cast<SMPP_GenericRequest*>(smpp.get());

    smsc_log_info(dmplxlog,"GenericRequest_Subscriber::handle::: got GENERIC_REQUEST message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), genericRequest->toString().c_str());
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

      if ( sessonToSMSC_searchResult.first == true ) {
        // Нашли сессию с SMSC. Можем переслать запрос в SMSC.
        // Предварительно проверяем, что сессия находится в правильном
        // состоянии.
        if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
          // SME прислал запрос, но сессия находится в состоянии,
          // не позволяющем принимать прикладные запросы. Удаляем сессию
          // из кэша, херем транспортное соединение для этого SME и 
          // удаляем сокет из пула.
          SessionHelper::terminateSessionToSme(socket);
        else {
          // Заменили sequnce_number, полученный в запросе на монотонно
          // возрастающее значение sequnce_number поддерживаемое для SMSC с
          // с заданным значением systemId. Исходное значение sequnce_number
          // сохранено в кэше.
          uint32_t seqNumForSMC = 
            CacheOfSMPP_message_traces::getInstance().putMessageTraceToCache(smpp->getSequenceNumber(), smppSession);
          genericRequest->setSequenceNumber(seqNumForSMC);

          std::auto_ptr<BufferedOutputStream> outBuf = genericRequest->marshal();
          smsc_log_info(dmplxlog,"GenericRequest_Subscriber::handle::: send GENERIC_REQUEST message to SMSC. Message dump=[%s]", genericRequest->toString().c_str());
          SMPPSession& sessionToSMSC = sessonToSMSC_searchResult.second;
          PendingOutDataQueue::scheduleDataForSending(*outBuf,sessionToSMSC.getSocketToPeer());
        }
      } else {
        // Не нашли сессию с SMSC. Значит объект smppSession соответствует
        // сессии с SMSC.

        if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
          // SMSC прислал запрос, но сессия находится в состоянии,
          // не позволяющем принимать прикладные запросы. Удаляем сессию
          // из кэша, херем транспортное соединение с SMSC и 
          // удаляем сокет из пула.
          SessionHelper::terminateSessionToSmsc(socket, smppSession.getSystemId());
        else {
          // По значению systemId получаем из кеша подходящую сессию с SME
          smpp_dmplx::SMPPSessionSwitch::search_result_t
            smseSessFromCache_searchRes = SMPPSessionSwitch::getInstance().getCrossedSession(systemId);
          if ( smseSessFromCache_searchRes.first == true ) {
            // Получили сессию с SME. Можем отправлять запрос.
            std::auto_ptr<BufferedOutputStream> outBuf = genericRequest->marshal();
            PendingOutDataQueue::scheduleDataForSending(*outBuf,smseSessFromCache_searchRes.second.getSocketToPeer());
          }
        }
      }
    } else {
      // Для сокета, на котором получен запрос, нет зарегистрированной сессии.
      // Ну это чиста проделки Фикса! Бред, короче.
      // Херем транспортное соединение  и  удаляем сокет из пула.
      SessionHelper::terminateSessionToSme(socket);
    }

    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}
