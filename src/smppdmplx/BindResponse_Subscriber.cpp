#include <memory>
#include <util/Exception.hpp>
#include <utility>

#include "BindResponse_Subscriber.hpp"
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

namespace smpp_dmplx {

BindResponse_Subscriber::BindResponse_Subscriber()
  : _log(smsc::logger::Logger::getInstance("msg_hndlr"))
{}

BindResponse_Subscriber::~BindResponse_Subscriber() {}

SMPP_Subscriber::handle_result_t
BindResponse_Subscriber::handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_message::BIND_RECEIVER_RESP ||
       smpp->getCommandId() == SMPP_message::BIND_TRANSMITTER_RESP ||
       smpp->getCommandId() == SMPP_message::BIND_TRANSCEIVER_RESP ) {
    SMPP_BindResponse* bindResponse = static_cast<SMPP_BindResponse*>(smpp.get());

    smsc_log_info(_log,"BindResponse_Subscriber::handle::: got BIND_RESPONSE message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), bindResponse->toString().c_str());

    // По сокету, на котором получено сообщение, получаю сессию с SMSC.
    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);
    if ( sessionSearchResult.first == false ) {
      // Сообщение BindResponse получено не от SMSC
      smsc_log_error(_log,"smpp_dmplx::BindResponse_Subscriber::handle::: It was got BindResponse from socket not for smsc communication");
      // Убирает сокет на котором получено сообщение из пула сокетов.
      SocketPool_Singleton::getInstance().remove_socket(socket);
    } else
      completeSessionActivation(sessionSearchResult.second, bindResponse);

    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}

void
BindResponse_Subscriber::completeSessionActivation(SMPPSession& sessionToSmsc,
                                                   SMPP_BindResponse* bindResponse)
{
  const std::string& systemId = sessionToSmsc.getSystemId();

  SMPPSessionSwitch::search_result_t smscSessFromCache_searchRes;
  try {
    smscSessFromCache_searchRes = SMPPSessionSwitch::getInstance().getSharedSessionToSMSC(systemId);
  } catch (...) {
    smscSessFromCache_searchRes.first = false;
  }

  if ( smscSessFromCache_searchRes.first == false ) {
    smsc_log_error(_log, "BindResponse_Subscriber::completeSessionActivation:::  It was got BindResponse for unknown BindRequest");
    SessionHelper::terminateSessionToSmsc(sessionToSmsc.getSocketToPeer(), systemId);
    return;
  }

  try {
    // Ищем запрос BIND_REQUEST от SME на основании которого был
    // сформирован запрос BIND_REQUEST в SMSC
    CacheOfSMPP_message_traces::MessageTrace_t messageTrace = 
      CacheOfSMPP_message_traces::getInstance().getMessageTraceFromCache(bindResponse->getSequenceNumber(), systemId);

    // По факту получения ответа BIND_RESPONSE
    // Изменяем состояние разделяемой сессии с SMSC
    if ( sessionToSmsc.updateSessionState(SMPPSession::GOT_BIND_RESP) != SMPPSession::OPERATION_SUCCESS )
      terminateConnectionsPair(messageTrace.second, sessionToSmsc.getSocketToPeer(), systemId);
    else
      forwardBindResponseMessageToSme(messageTrace.first, messageTrace.second, bindResponse, sessionToSmsc.getSocketToPeer(), systemId);
  } catch (std::exception& ex) {
    try {
      smsc_log_error(_log,"BindResponse_Subscriber::completeSessionActivation::: Catched exception [%s]", ex.what());
      SessionHelper::terminateSessionToSmsc(sessionToSmsc.getSocketToPeer(), systemId);
    } catch (...) {}
    throw;
  }
}

void
BindResponse_Subscriber::terminateConnectionsPair(SMPPSession& smppSession,
                                                  smsc::core_ax::network::Socket& socketToSmsc,
                                                  const std::string& systemId)
{
  // Завершаем сессию с SMSC.
  SessionHelper::terminateSessionToSmsc(socketToSmsc, systemId);
  // Завершаем сессию с SME.
  SessionHelper::terminateSessionToSme(smppSession.getSocketToPeer());
}

void
BindResponse_Subscriber::forwardBindResponseMessageToSme(uint32_t originalSeqNum,
                                                         SMPPSession& smppSessionToSme,
                                                         SMPP_BindResponse* bindResponse,
                                                         smsc::core_ax::network::Socket& socketToSmsc,
                                                         const std::string& systemId)
{
  // заменяем значение sequenceNumber значением, сохраненнным в кэше
  bindResponse->setSequenceNumber(originalSeqNum);

  smsc_log_info(_log,"BindResponse_Subscriber::forwardBindResponseMessageToSme::: send BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
  // преобразовали в буфер для записи в сокет
  std::auto_ptr<BufferedOutputStream> bufToWriteForSme = bindResponse->marshal();

  // послали ответ в SME от которого был получен первый запрос
  // BindRequest
  PendingOutDataQueue::getInstance().scheduleDataForSending(*bufToWriteForSme, smppSessionToSme.getSocketToPeer());

  // Пошукали в кэше запросы BindRequest от других SME с тем же 
  // systemId. Для каждого найденного запроса сформировали ответ со 
  // статусом равным bindResponse->getCommandStatus()
  CacheOfPendingBindReqFromSME::getInstance().commitPendingReqInCache(systemId, *bindResponse);
  // Проверяем статус ответа
  if ( bindResponse->getCommandStatus() != ESME_ROK )
    // Если в ответе BIND_RESPONSE получена ошибка, то сессию 
    // установить не удалось и необходимо удалить разделяемую сессию
    // с SMSC из пула сессий для заданного значения systemId.
    SessionHelper::terminateSessionToSmsc(socketToSmsc, systemId);
  else
    smppSessionToSme.updateSessionState(SMPPSession::GOT_BIND_RESP);
}

}
