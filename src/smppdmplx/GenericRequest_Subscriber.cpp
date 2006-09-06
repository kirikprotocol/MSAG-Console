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
      // �� �������� ������ ����� � ���� SMPP ������.
      // ��������� ������ ����� ���� ��� ������� � SMSC, ��� � ������� � SME.
      SMPPSession& smppSession = sessionSearchResult.second;
      std::string systemId = smppSession.getSystemId();

      // �� �������� ������ �������� ����� ������ � SMSC. ���� �����,
      // �� ���� ������ - ��� ������ � SME, ����� ��� ������ � SMSC.
      std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
        SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);

      if ( sessonToSMSC_searchResult.first == true ) {
        // ����� ������ � SMSC. ����� ��������� ������ � SMSC.
        // �������������� ���������, ��� ������ ��������� � ����������
        // ���������.
        if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
          // SME ������� ������, �� ������ ��������� � ���������,
          // �� ����������� ��������� ���������� �������. ������� ������
          // �� ����, ����� ������������ ���������� ��� ����� SME � 
          // ������� ����� �� ����.
          SessionHelper::terminateSessionToSme(socket);
        else {
          // �������� sequnce_number, ���������� � ������� �� ���������
          // ������������ �������� sequnce_number �������������� ��� SMSC �
          // � �������� ��������� systemId. �������� �������� sequnce_number
          // ��������� � ����.
          uint32_t seqNumForSMC = 
            CacheOfSMPP_message_traces::getInstance().putMessageTraceToCache(smpp->getSequenceNumber(), smppSession);
          genericRequest->setSequenceNumber(seqNumForSMC);

          std::auto_ptr<BufferedOutputStream> outBuf = genericRequest->marshal();
          smsc_log_info(dmplxlog,"GenericRequest_Subscriber::handle::: send GENERIC_REQUEST message to SMSC. Message dump=[%s]", genericRequest->toString().c_str());
          SMPPSession& sessionToSMSC = sessonToSMSC_searchResult.second;
          PendingOutDataQueue::scheduleDataForSending(*outBuf,sessionToSMSC.getSocketToPeer());
        }
      } else {
        // �� ����� ������ � SMSC. ������ ������ smppSession �������������
        // ������ � SMSC.

        if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
          // SMSC ������� ������, �� ������ ��������� � ���������,
          // �� ����������� ��������� ���������� �������. ������� ������
          // �� ����, ����� ������������ ���������� � SMSC � 
          // ������� ����� �� ����.
          SessionHelper::terminateSessionToSmsc(socket, smppSession.getSystemId());
        else {
          // �� �������� systemId �������� �� ���� ���������� ������ � SME
          smpp_dmplx::SMPPSessionSwitch::search_result_t
            smseSessFromCache_searchRes = SMPPSessionSwitch::getInstance().getCrossedSession(systemId);
          if ( smseSessFromCache_searchRes.first == true ) {
            // �������� ������ � SME. ����� ���������� ������.
            std::auto_ptr<BufferedOutputStream> outBuf = genericRequest->marshal();
            PendingOutDataQueue::scheduleDataForSending(*outBuf,smseSessFromCache_searchRes.second.getSocketToPeer());
          }
        }
      }
    } else {
      // ��� ������, �� ������� ������� ������, ��� ������������������ ������.
      // �� ��� ����� �������� �����! ����, ������.
      // ����� ������������ ����������  �  ������� ����� �� ����.
      SessionHelper::terminateSessionToSme(socket);
    }

    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}
