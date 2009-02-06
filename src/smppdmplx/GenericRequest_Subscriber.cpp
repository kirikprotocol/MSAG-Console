#include <memory>

#include "GenericRequest_Subscriber.hpp"
#include "SMPP_GenericRequest.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SMPPSession.hpp"
#include "SessionCache.hpp"
#include "BufferedOutputStream.hpp"
#include "SocketPool_Singleton.hpp"
#include "CacheOfSMPP_message_traces.hpp"
#include "PendingOutDataQueue.hpp"
#include "SessionHelper.hpp"
#include "SequenceNumberGenerator.hpp"

namespace smpp_dmplx {

GenericRequest_Subscriber::GenericRequest_Subscriber()
  : _log(smsc::logger::Logger::getInstance("msg_hndlr")) {}

GenericRequest_Subscriber::~GenericRequest_Subscriber() {}

SMPP_Subscriber::handle_result_t
GenericRequest_Subscriber::handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_GenericRequest::GENERIC_REQUEST ) {
    SMPP_GenericRequest* genericRequest = static_cast<SMPP_GenericRequest*>(smpp.get());

    smsc_log_info(_log,"GenericRequest_Subscriber::handle::: got GENERIC_REQUEST message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), genericRequest->toString().c_str());
    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);

    if ( sessionSearchResult.first == true ) {
      // �� �������� ������ ����� � ���� SMPP ������.
      // ��������� ������ ����� ���� ��� ������� � SMSC, ��� � ������� � SME.
      SMPPSession& smppSession = sessionSearchResult.second;

      // �� �������� ������ �������� ����� ������ � SMSC. ���� �����,
      // �� ���� ������ - ��� ������ � SME, ����� ��� ������ � SMSC.
      std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
        SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);

      if ( sessonToSMSC_searchResult.first == true )
        // ����� ������ � SMSC. ����� ��������� ������ � SMSC.
        forwardMessageToSmsc(genericRequest, smppSession, sessonToSMSC_searchResult.second);
      else
        // �� ����� ������ � SMSC. ������ ������ smppSession �������������
        // ������ � SMSC.
        forwardMessageToSme(genericRequest, smppSession);
    } else
      // ��� ������, �� ������� ������� ������, ��� ������������������ ������.
      // ����� ������������ ����������  �  ������� ����� �� ����.
      SessionHelper::terminateSessionToSme(socket);

    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}

void
GenericRequest_Subscriber::forwardMessageToSmsc(SMPP_GenericRequest* genericRequest,
                                                SMPPSession& smppSessionToSme,
                                                SMPPSession& sessionToSMSC)
{
  // �������������� ���������, ��� ������ ��������� � ����������
  // ���������.
  if ( smppSessionToSme.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
    // SME ������� ������, �� ������ ��������� � ���������,
    // �� ����������� ��������� ���������� �������. ������� ������
    // �� ����, ����� ������������ ���������� ��� ����� SME � 
    // ������� ����� �� ����.
    SessionHelper::terminateSessionToSme(smppSessionToSme.getSocketToPeer());
  else {
    // �������� sequnce_number, ���������� � ������� �� ���������
    // ������������ �������� sequnce_number �������������� ��� SMSC �
    // � �������� ��������� systemId. �������� �������� sequence_number
    // ��������� � ����.
    uint32_t seqNumToSmsc = SequenceNumberGenerator::getInstance(sessionToSMSC.getSystemId())->getNextValue();
    CacheOfSMPP_message_traces::getInstance().putMessageTraceToCache(genericRequest->getSequenceNumber(), smppSessionToSme, seqNumToSmsc);
    genericRequest->setSequenceNumber(seqNumToSmsc);

    std::auto_ptr<BufferedOutputStream> outBuf = genericRequest->marshal();
    smsc_log_info(_log,"GenericRequest_Subscriber::forwardMessageToSmsc::: send GENERIC_REQUEST message to SMSC. Message dump=[%s]", genericRequest->toString().c_str());
    PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf,sessionToSMSC.getSocketToPeer());
  }
}

void
GenericRequest_Subscriber::forwardMessageToSme(SMPP_GenericRequest* genericRequest,
                                               SMPPSession& sessionToSMSC)
{
  if ( sessionToSMSC.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
    // SMSC ������� ������, �� ������ ��������� � ���������,
    // �� ����������� ��������� ���������� �������. ������� ������
    // �� ����, ����� ������������ ���������� � SMSC � 
    // ������� ����� �� ����.
    SessionHelper::terminateSessionToSmsc(sessionToSMSC.getSocketToPeer(), sessionToSMSC.getSystemId());
  else {
    // �� �������� systemId �������� �� ���� ���������� ������ � SME
    SMPPSessionSwitch::search_result_t
      smeSessFromCache_searchRes = SMPPSessionSwitch::getInstance().getCrossedSession(sessionToSMSC.getSystemId());
    if ( smeSessFromCache_searchRes.first == true ) {
      // �������� ������ � SME. ����� ���������� ������.
      std::auto_ptr<BufferedOutputStream> outBuf = genericRequest->marshal();
      PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf,smeSessFromCache_searchRes.second.getSocketToPeer());
    } else {
      smsc_log_error(_log, "GenericRequest_Subscriber::forwardMessageToSme::: session to sme has not been found. Terminate session to smsc");
      SessionHelper::terminateSessionToSmsc(sessionToSMSC.getSocketToPeer(), sessionToSMSC.getSystemId());
    }
  }
}

}
