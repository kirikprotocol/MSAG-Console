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
      // ����� � ���� ������, ��������������� ������, �� ������� 
      // ������� ������.
      // ��������� ������ ����� ���� ��� ������� � SMSC, ��� � ������� � SME.
      SMPPSession& smppSession = sessionSearchResult.second;

      // �� �������� ������ �������� ����� ������ � SMSC. ���� �����,
      // �� ���� ������ - ��� ������ � SME, ����� ��� ������ � SMSC.
      std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
        SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);

      if ( sessonToSMSC_searchResult.first == true ) {
        if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
          // SME ������� ��������� ���������, �� ������ ��������� � ���������,
          // �� ����������� ��������� ���������� �������. ������� ������
          // �� ����, ����� ������������ ���������� ��� ����� SME � 
          // ������� ����� �� ����.
          SessionHelper::terminateSessionToSme(socket);
        else {
          SMPPSession& sessionToSMSC = sessonToSMSC_searchResult.second;
          // ����� ������ � SMSC. ����� ��������� ����� � SMSC.
          // �����, �������������� �������� smpp - ��� ����� �� ������,
          // ���������� �� SMSC � SME.
          std::auto_ptr<BufferedOutputStream> outBuf = genericResponse->marshal();
          smsc_log_info(dmplxlog,"GenericResponse_Subscriber::handle::: send GENERIC_RESPONSE message to SMSC. Message dump=[%s]", genericResponse->toString().c_str());
          PendingOutDataQueue::scheduleDataForSending(*outBuf,sessionToSMSC.getSocketToPeer());
        }
      } else {
        if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
          // SMSC ������� ������, �� ������ ��������� � ���������,
          // �� ����������� ��������� ���������� �������. ������� ������
          // �� ����, ����� ������������ ���������� � SMSC � 
          // ������� ����� �� ����.
          SessionHelper::terminateSessionToSmsc(socket, smppSession.getSystemId());
        else {
          // ������, �� ������� ��������� smppSession, ������������� ������
          // � SMSC. ������������� ����� ���������� ��������� � SME.
          // �����, �������������� �������� smpp - ��� ����� �� ������,
          // ���������� �� SME � SMSC.
          // ���� ���� ������� � ����.
          CacheOfSMPP_message_traces::MessageTrace_t messageTrace = 
            CacheOfSMPP_message_traces::getInstance().getMessageTraceFromCache(smpp->getSequenceNumber(), smppSession.getSystemId());
          genericResponse->setSequenceNumber(messageTrace.first);
          std::auto_ptr<BufferedOutputStream> outBuf = genericResponse->marshal();
          smsc_log_info(dmplxlog,"GenericResponse_Subscriber::handle::: send GENERIC_RESPONSE message to SME. Message dump=[%s]", genericResponse->toString().c_str());
          PendingOutDataQueue::scheduleDataForSending(*outBuf,messageTrace.second.getSocketToPeer());
        }
      }
    } else {
      // �� ��� ����� �������� �����! ����, ������.
      // ����� ������������ ����������  �  ������� ����� �� ����.
      SessionHelper::terminateSessionToSme(socket);
    }
    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}
