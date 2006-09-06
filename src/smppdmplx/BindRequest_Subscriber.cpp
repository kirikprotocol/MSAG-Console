#include "BindRequest_Subscriber.hpp"
#include "Publisher.hpp"
#include "SMPP_BindRequest.hpp"
#include "SMPP_Constants.hpp"
#include <utility>

#include "SMPPSession.hpp"
#include "SessionCache.hpp"
#include "SMPPSessionSwitch.hpp"
#include "CacheOfSMPP_message_traces.hpp"
#include "SocketPool_Singleton.hpp"
#include "PendingOutDataQueue.hpp"
#include "CacheOfPendingBindReqFromSME.hpp"
#include "SessionHelper.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

static int toRegisterSubscriber() {
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::BindRequest_Subscriber());

  return 0;
}

static int subscriberIsRegistered = toRegisterSubscriber();

smpp_dmplx::BindRequest_Subscriber::~BindRequest_Subscriber() {}

smpp_dmplx::SMPP_Subscriber::handle_result_t
smpp_dmplx::BindRequest_Subscriber::handle(std::auto_ptr<smpp_dmplx::SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_message::BIND_RECEIVER ||
       smpp->getCommandId() == SMPP_message::BIND_TRANSMITTER ||
       smpp->getCommandId() == SMPP_message::BIND_TRANSCEIVER ) {
    
    SMPP_BindRequest* bindRequest = dynamic_cast<SMPP_BindRequest*>(smpp.get());
    try {
      smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: got BIND_REQUEST message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), bindRequest->toString().c_str());
      SMPPSession smppSessionToSME(bindRequest->getSystemId(), socket);
      //  —охран€ем в кеше св€зку socket_дл€_работы_с_SME -- сесси€_с_SME
      SessionCache::getInstance().makeSocketSessionEntry(socket,
                                                         smppSessionToSME);
      smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: make SME authentication");
      if ( smppSessionToSME.authenticate(bindRequest->getPassword()) == SMPPSession::AUTH_SUCCESS ) {
        // ≈сли аутентификаци€ успешна,
        // то измен€ем состо€ние сесси€ с SME на GOT_BIND_REQ
        smppSessionToSME.updateSessionState(SMPPSession::GOT_BIND_REQ);

        // получаем из кеша раздел€емую сессию с SMSC, соответствующую
        // полученному в smpp-запросе значению systemId
        SMPPSessionSwitch::search_result_t smscSessFromCache =
          SMPPSessionSwitch::getInstance().getSharedSessionToSMSC(bindRequest->getSystemId());

        if ( smscSessFromCache.first == false ) {
          // ≈сли раздел€емой сессии с smsc еще не существует, то создаем ее.
          smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: Try establish smpp session with SMSC");
          SMPPSession newSmscSession(bindRequest->getSystemId());
          newSmscSession.connectSession();
          smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: connect to SMSC was established");
          // ѕомещаем в пул сокет, соответствующий установленному соединению с SMSC
          SocketPool_Singleton::getInstance().push_socket(newSmscSession.getSocketToPeer());
          // ѕодмен€ем значение sequenceNumber, полученное от SME, на монотонно
          // возрастрающее значение дл€ данного systemId и сохран€ем в кэше 
          // сессию, от которой получен запрос BindRequest
          uint32_t seqNumToSmsc = 
            CacheOfSMPP_message_traces::getInstance().putMessageTraceToCache(smpp->getSequenceNumber(), smppSessionToSME);
          smpp->setSequenceNumber(seqNumToSmsc);
          smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: send BIND_REQUEST message to SMSC. Message dump=[%s]", smpp->toString().c_str());
          std::auto_ptr<BufferedOutputStream> outBuf = smpp->marshal();

          // посылаем запрос BindRequest в SMSC
          PendingOutDataQueue::scheduleDataForSending(*outBuf, newSmscSession.getSocketToPeer());

          newSmscSession.updateSessionState(SMPPSession::GOT_BIND_REQ);
          // ”станавливаем соответствие systemId -- сесси€_с_SMSC
          SMPPSessionSwitch::getInstance().setSharedSessionToSMSC(newSmscSession, bindRequest->getSystemId());

          //  —охран€ем в кеше св€зку сокет_с_SMSC -- сесси€_с_SMSC
          SessionCache::getInstance().makeSocketSessionEntry(newSmscSession.getSocketToPeer(),
                                                             newSmscSession);
          // и добавл€ем сессию_от_SME в список активных сессий с указанным
          // значением systemId
          SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
        } else {
          if ( !smscSessFromCache.second.bindInProgress() ) {
            // —есси€ с SMSC дл€ данного значение systemId уже установлена.
            // ќтправл€ем офигенно положительный ответ на полученный запрос
            // BindRequest
            std::auto_ptr<SMPP_BindResponse> bindResponse(bindRequest->prepareResponse(ESME_ROK));
            smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: send positive BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
            std::auto_ptr<BufferedOutputStream> outBuf = bindResponse->marshal();
            PendingOutDataQueue::scheduleDataForSending(*outBuf, socket);
            // измен€ем состо€ние сессии с SME
            smppSessionToSME.updateSessionState(SMPPSession::GOT_BIND_RESP);
            // и добавл€ем сессию_от_SME в список активных сессий с указанным
            // значением systemId
            SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);

          } else {
            // ƒл€ данного systemId в SMSC уже послан запрос BindRequest,
            // поэтому очередной запрос BindRequest в SMSC не посылаем. 
            // ¬место этого сохран€ем запрос дл€ данного SME в кэше. 
            // ѕри получении ответа от SMSC на первичный запрос BindRequest
            // дл€ данного systemId, дл€ всех кэшированных на данный момент
            // запросов формируем ответы со статусом, соответствующий статусу
            // полученного ответа от SMSC и рассылаем подготовленные ответы
            // по соответствующим SME.
            CacheOfPendingBindReqFromSME::getInstance().putPendingReqToCache(smppSessionToSME, dynamic_cast<SMPP_BindRequest*>(smpp.release()));
            // и добавл€ем сессию_от_SME в список активных сессий с указанным
            // значением systemId
            SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
          }
        }
      } else {
        // подготовить BindResponse со статусом ESME_RINVPASWD
        std::auto_ptr<SMPP_BindResponse> bindResponse(bindRequest->prepareResponse(ESME_RINVPASWD));
        std::auto_ptr<BufferedOutputStream> outBuf = bindResponse->marshal();
        smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: send negative BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
        PendingOutDataQueue::scheduleDataForSending(*outBuf, socket);
        // ”дал€ем сессию с SME.
        SessionHelper::terminateSessionToSme(socket);
      }
      return RequestWasProcessed;
    } catch (std::exception& ex) {
      smsc_log_error(dmplxlog,"BindRequest_Subscriber::handle::: catch unexpected exception [%s]", ex.what());
    }
    smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: Send negative bind_response and close session");
    try {
      SessionHelper::terminateSessionToSme(socket);
      std::auto_ptr<SMPP_BindResponse> bindResponse(bindRequest->prepareResponse(ESME_RBINDFAIL));
      std::auto_ptr<BufferedOutputStream> outBuf = bindResponse->marshal();
      smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: send negative BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
      PendingOutDataQueue::scheduleDataForSending(*outBuf,socket);
    } catch (...) {}
    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}
