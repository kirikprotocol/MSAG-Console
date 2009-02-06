#include <utility>

#include "SequenceNumberGenerator.hpp"
#include "BindRequest_Subscriber.hpp"
#include "SMPP_BindRequest.hpp"
#include "SMPPSession.hpp"
#include "SessionCache.hpp"
#include "SMPPSessionSwitch.hpp"
#include "CacheOfSMPP_message_traces.hpp"
#include "SocketPool_Singleton.hpp"
#include "PendingOutDataQueue.hpp"
#include "CacheOfPendingBindReqFromSME.hpp"
#include "SessionHelper.hpp"

namespace smpp_dmplx {

BindRequest_Subscriber::BindRequest_Subscriber()
  : _log(smsc::logger::Logger::getInstance("msg_hndlr"))
{}

BindRequest_Subscriber::~BindRequest_Subscriber() {}

SMPP_Subscriber::handle_result_t
BindRequest_Subscriber::handle(std::auto_ptr<smpp_dmplx::SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_message::BIND_RECEIVER ||
       smpp->getCommandId() == SMPP_message::BIND_TRANSMITTER ||
       smpp->getCommandId() == SMPP_message::BIND_TRANSCEIVER ) {

    SMPP_BindRequest* bindRequest = static_cast<SMPP_BindRequest*>(smpp.get());
    try {
      smsc_log_info(_log,"BindRequest_Subscriber::handle::: got BIND_REQUEST message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), bindRequest->toString().c_str());
      SMPPSession smppSessionToSME(bindRequest->getSystemId(), socket);
      //  Сохраняем в кеше связку socket_для_работы_с_SME -- сессия_с_SME
      SessionCache::getInstance().makeSocketSessionEntry(socket, smppSessionToSME);

      smsc_log_info(_log,"BindRequest_Subscriber::handle::: make SME authentication");
      if ( smppSessionToSME.authenticate(bindRequest->getPassword()) == SMPPSession::AUTH_SUCCESS ) {
        // Если аутентификация успешна,
        // то изменяем состояние сессия с SME на GOT_BIND_REQ
        smppSessionToSME.updateSessionState(SMPPSession::GOT_BIND_REQ);

        // получаем из кеша разделяемую сессию с SMSC, соответствующую
        // полученному в smpp-запросе значению systemId
        SMPPSessionSwitch::search_result_t smscSessFromCache =
          SMPPSessionSwitch::getInstance().getSharedSessionToSMSC(bindRequest->getSystemId());

        if ( smscSessFromCache.first == false )
          initiateNewSessionToSmsc(bindRequest, smppSessionToSME);
        else {
          if ( !smscSessFromCache.second.bindInProgress() )
            confirmIncomingSessionRequest(bindRequest, smppSessionToSME);
          else {
            pendIncomingSessionRequest(bindRequest, smppSessionToSME);
            smpp.release();
          }
        }

      } else {
        prepareBindResponseMessage(socket, bindRequest, ESME_RINVPASWD);

        // Удаляем сессию с SME.
        SessionHelper::terminateSessionToSme(socket);
      }
      return RequestWasProcessed;
    } catch (std::exception& ex) {
      smsc_log_error(_log,"BindRequest_Subscriber::handle::: catch unexpected exception [%s]", ex.what());
    }

    teminateSession(socket, bindRequest);

    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}

void
BindRequest_Subscriber::initiateNewSessionToSmsc(SMPP_BindRequest* bindRequest,
                                                 SMPPSession& smppSessionToSME)
{
  smsc_log_info(_log,"BindRequest_Subscriber::initiateNewSessionToSmsc::: Try establish smpp session with SMSC");
  SMPPSession newSmscSession(bindRequest->getSystemId());
  newSmscSession.connectSession();
  smsc_log_info(_log,"BindRequest_Subscriber::initiateNewSessionToSmsc::: connect to SMSC has been established");
  // Помещаем в пул сокет, соответствующий установленному соединению с SMSC
  SocketPool_Singleton::getInstance().push_socket(newSmscSession.getSocketToPeer());
  // Подменяем значение sequenceNumber, полученное от SME, на монотонно
  // возрастрающее значение для данного systemId и сохраняем в кэше 
  // сессию, от которой получен запрос BindRequest

  uint32_t seqNumToSmsc = SequenceNumberGenerator::getInstance(bindRequest->getSystemId())->getNextValue();
  CacheOfSMPP_message_traces::getInstance().putMessageTraceToCache(bindRequest->getSequenceNumber(), smppSessionToSME, seqNumToSmsc);
  bindRequest->setSequenceNumber(seqNumToSmsc);
  smsc_log_info(_log,"BindRequest_Subscriber::initiateNewSessionToSmsc::: send BIND_REQUEST message to SMSC. Message dump=[%s]", bindRequest->toString().c_str());
  std::auto_ptr<BufferedOutputStream> outBuf = bindRequest->marshal();

  // посылаем запрос BindRequest в SMSC
  PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf, newSmscSession.getSocketToPeer());

  newSmscSession.updateSessionState(SMPPSession::GOT_BIND_REQ);
  // Устанавливаем соответствие systemId -- сессия_с_SMSC
  SMPPSessionSwitch::getInstance().setSharedSessionToSMSC(newSmscSession, bindRequest->getSystemId());

  //  Сохраняем в кеше связку сокет_с_SMSC -- сессия_с_SMSC
  SessionCache::getInstance().makeSocketSessionEntry(newSmscSession.getSocketToPeer(),
                                                     newSmscSession);
  // и добавляем сессию_от_SME в список активных сессий с указанным
  // значением systemId
  SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
}

void
BindRequest_Subscriber::confirmIncomingSessionRequest(SMPP_BindRequest* bindRequest,
                                                      SMPPSession& smppSessionToSME)
{
  // Сессия с SMSC для данного значение systemId уже установлена.
  // Отправляем офигенно положительный ответ на полученный запрос
  // BindRequest
  prepareBindResponseMessage(smppSessionToSME.getSocketToPeer(), bindRequest, ESME_ROK);
  // изменяем состояние сессии с SME
  smppSessionToSME.updateSessionState(SMPPSession::GOT_BIND_RESP);
  // и добавляем сессию_от_SME в список активных сессий с указанным
  // значением systemId
  SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
}

void
BindRequest_Subscriber::pendIncomingSessionRequest(SMPP_BindRequest* bindRequest,
                                                   SMPPSession& smppSessionToSME)
{
  // Для данного systemId в SMSC уже послан запрос BindRequest,
  // поэтому очередной запрос BindRequest в SMSC не посылаем. 
  // Вместо этого сохраняем запрос для данного SME в кэше. 
  // При получении ответа от SMSC на первичный запрос BindRequest
  // для данного systemId, для всех кэшированных на данный момент
  // запросов формируем ответы со статусом, соответствующий статусу
  // полученного ответа от SMSC и рассылаем подготовленные ответы
  // по соответствующим SME.
  CacheOfPendingBindReqFromSME::getInstance().putPendingReqToCache(smppSessionToSME, bindRequest);
  // и добавляем сессию_от_SME в список активных сессий с указанным
  // значением systemId
  SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
}

void
BindRequest_Subscriber::teminateSession(smsc::core_ax::network::Socket& socket,
                                        SMPP_BindRequest* bindRequest)
{
  smsc_log_info(_log,"BindRequest_Subscriber::teminateSession::: Send negative bind_response and close session");
  try {
    prepareBindResponseMessage(socket, bindRequest, ESME_RBINDFAIL);
    SessionHelper::terminateSessionToSme(socket);
  } catch (...) {
    smsc_log_info(_log,"BindRequest_Subscriber::teminateSession::: catched unexpected exception");
  }
}

void
BindRequest_Subscriber::prepareBindResponseMessage(smsc::core_ax::network::Socket& socket,
                                                   SMPP_BindRequest* bindRequest,
                                                   smpp_status_t statusInResponse)
{
  std::auto_ptr<SMPP_BindResponse> bindResponse(bindRequest->prepareResponse(statusInResponse));
  std::auto_ptr<BufferedOutputStream> outBuf = bindResponse->marshal();
  smsc_log_info(_log,"BindRequest_Subscriber::prepareBindResponseMessage::: send BIND_RESPONSE message to SME with status=%d. Message dump=[%s]", statusInResponse, bindResponse->toString().c_str());
  PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf, socket);
}

}
