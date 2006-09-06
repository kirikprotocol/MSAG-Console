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

    // �� ������, �� ������� �������� ���������, ������� ������ � SMSC.
    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);
    if ( sessionSearchResult.first == false ) {
      // ��������� BindResponse �������� �� �� SMSC
      smsc_log_error(dmplxlog,"smpp_dmplx::BindResponse_Subscriber::handle::: It was got BindResponse from socket not for smsc communication");
      // ������� ����� �� ������� �������� ��������� �� ���� �������.
      SocketPool_Singleton::getInstance().remove_socket(socket);
    } else {
      std::string systemId = sessionSearchResult.second.getSystemId();

      try {
        SMPPSessionSwitch::search_result_t smscSessFromCache_searchRes =
          SMPPSessionSwitch::getInstance().getSharedSessionToSMSC(systemId);
        if ( smscSessFromCache_searchRes.first == false )
          throw smsc::util::Exception("smpp_dmplx::BindResponse_Subscriber::handle:::  It was got BindResponse for unknown BindRequest");

        // ���� ������ BIND_REQUEST �� SME �� ��������� �������� ���
        // ����������� ������ BIND_REQUEST � SMSC
        CacheOfSMPP_message_traces::MessageTrace_t messageTrace = 
          CacheOfSMPP_message_traces::getInstance().getMessageTraceFromCache(bindResponse->getSequenceNumber(), systemId);

        // �� ����� ��������� ������ BIND_RESPONSE
        // �������� ��������� ����������� ������ � SMSC
        if ( smscSessFromCache_searchRes.second.updateSessionState(SMPPSession::GOT_BIND_RESP) != SMPPSession::OPERATION_SUCCESS ) {
          // ��������� ������ � SMSC.
          SessionHelper::terminateSessionToSmsc(socket, systemId);
          // ��������� ������ � SME.
          SessionHelper::terminateSessionToSme(messageTrace.second.getSocketToPeer());
        } else {
          // �������� �������� sequenceNumber ���������, ������������ � ����
          bindResponse->setSequenceNumber(messageTrace.first);

          smsc_log_info(dmplxlog,"BindResponse_Subscriber::handle::: send BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
          // ������������� � ����� ��� ������ � �����
          std::auto_ptr<BufferedOutputStream>
            bufToWriteForSme = bindResponse->marshal();

          // ������� ����� � SME �� �������� ��� ������� ������ ������
          // BindRequest
          PendingOutDataQueue::scheduleDataForSending(*bufToWriteForSme,messageTrace.second.getSocketToPeer());

          // �������� � ���� ������� BindRequest �� ������ SME � ��� �� 
          // systemId. ��� ������� ���������� ������� ������������ ����� �� 
          // �������� ������ bindResponse->getCommandStatus()
          CacheOfPendingBindReqFromSME::getInstance().commitPendingReqInCache(systemId, *bindResponse);
          // ��������� ������ ������
          if ( bindResponse->getCommandStatus() != ESME_ROK )
            // ���� � ������ BIND_RESPONSE �������� ������, �� ������ 
            // ���������� �� ������� � ���������� ������� ����������� ������
            // � SMSC �� ���� ������ ��� ��������� �������� systemId.
            // ��������� ������ � SMSC.
            SessionHelper::terminateSessionToSmsc(socket, systemId);
          else
            messageTrace.second.updateSessionState(SMPPSession::GOT_BIND_RESP);
        }
      } catch (std::exception& ex) {
        try {
          smsc_log_error(dmplxlog,"BindResponse_Subscriber::handle::: Catch exception [%s]", ex.what());

          // ��������� ������ � SMSC.
          SessionHelper::terminateSessionToSmsc(socket, systemId);
        } catch (...) {}
        throw;
      }
    }
    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}
