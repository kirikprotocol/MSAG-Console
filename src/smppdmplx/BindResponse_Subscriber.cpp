#include <memory>
#include <util/Exception.hpp>
#include <utility>

#include "BindResponse_Subscriber.hpp"
#include "Publisher.hpp"
#include "SMPP_BindResponse.hpp"
#include "CacheOfSMPP_message_traces.hpp"
#include "SMPPSession.hpp"
#include "SessionCache.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SMPP_Constants.hpp"
#include "SocketPool_Singleton.hpp"
#include "PendingOutDataQueue.hpp"
#include "SessionHelper.hpp"
#include "CacheOfPendingBindReqFromSME.hpp"
#include "SocketPool_Singleton.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

static int toRegisterSubscriber() {
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::BindResponse_Subscriber());

  return 0;
}

static int subscriberIsRegistered = toRegisterSubscriber();

smpp_dmplx::BindResponse_Subscriber::~BindResponse_Subscriber() {}

smpp_dmplx::SMPP_Subscriber::handle_result_t
smpp_dmplx::BindResponse_Subscriber::handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_message::BIND_RECEIVER_RESP ||
       smpp->getCommandId() == SMPP_message::BIND_TRANSMITTER_RESP ||
       smpp->getCommandId() == SMPP_message::BIND_TRANSCEIVER_RESP ) {
    SMPP_BindResponse* bindResponse = dynamic_cast<SMPP_BindResponse*>(smpp.get());

    smsc_log_info(dmplxlog,"BindResponse_Subscriber::handle::: got BIND_RESPONSE message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), bindResponse->toString().c_str());

    // По сокету, на котором получено сообщение, получаю сессию с SMSC.
    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);
    if ( sessionSearchResult.first == false ) {
      // Сообщение BindResponse получено не от SMSC
      smsc_log_error(dmplxlog,"smpp_dmplx::BindResponse_Subscriber::handle::: It was got BindResponse from socket not for smsc communication");
      // Убирает сокет на котором получено сообщение из пула сокетов.
      SocketPool_Singleton::getInstance().remove_socket(socket);
    } else {
      std::string systemId = sessionSearchResult.second.getSystemId();

      try {
        SMPPSessionSwitch::search_result_t smscSessFromCache_searchRes =
          SMPPSessionSwitch::getInstance().getSharedSessionToSMSC(systemId);
        if ( smscSessFromCache_searchRes.first == false )
          throw smsc::util::Exception("smpp_dmplx::BindResponse_Subscriber::handle:::  It was got BindResponse for unknown BindRequest");

        // Ищем запрос BIND_REQUEST от SME на основании которого был
        // сформирован запрос BIND_REQUEST в SMSC
        CacheOfSMPP_message_traces::MessageTrace_t messageTrace = 
          CacheOfSMPP_message_traces::getInstance().getMessageTraceFromCache(bindResponse->getSequenceNumber(), systemId);

        // По факту получения ответа BIND_RESPONSE
        // Изменяем состояние разделяемой сессии с SMSC
        if ( smscSessFromCache_searchRes.second.updateSessionState(SMPPSession::GOT_BIND_RESP) != SMPPSession::OPERATION_SUCCESS ) {
          // Завершаем сессию с SMSC.
          SessionHelper::terminateSessionToSmsc(socket, systemId);
          // Завершаем сессию с SME.
          SessionHelper::terminateSessionToSme(messageTrace.second.getSocketToPeer());
        } else {
          // заменяем значение sequenceNumber значением, сохраненнным в кэше
          bindResponse->setSequenceNumber(messageTrace.first);

          smsc_log_info(dmplxlog,"BindResponse_Subscriber::handle::: send BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
          // преобразовали в буфер для записи в сокет
          std::auto_ptr<BufferedOutputStream>
            bufToWriteForSme = bindResponse->marshal();

          // послали ответ в SME от которого был получен первый запрос
          // BindRequest
          PendingOutDataQueue::scheduleDataForSending(*bufToWriteForSme,messageTrace.second.getSocketToPeer());

          // Пошукали в кэше запросы BindRequest от других SME с тем же 
          // systemId. Для каждого найденного запроса сформировали ответ со 
          // статусом равным bindResponse->getCommandStatus()
          CacheOfPendingBindReqFromSME::getInstance().commitPendingReqInCache(systemId, *bindResponse);
          // Проверяем статус ответа
          if ( bindResponse->getCommandStatus() != ESME_ROK )
            // Если в ответе BIND_RESPONSE получена ошибка, то сессию 
            // установить не удалось и необходимо удалить разделяемую сессию
            // с SMSC из пула сессий для заданного значения systemId.
            // Завершаем сессию с SMSC.
            SessionHelper::terminateSessionToSmsc(socket, systemId);
          else
            messageTrace.second.updateSessionState(SMPPSession::GOT_BIND_RESP);
        }
      } catch (std::exception& ex) {
        try {
          smsc_log_error(dmplxlog,"BindResponse_Subscriber::handle::: Catch exception [%s]", ex.what());

          // Завершаем сессию с SMSC.
          SessionHelper::terminateSessionToSmsc(socket, systemId);
        } catch (...) {}
        throw;
      }
    }
    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}
