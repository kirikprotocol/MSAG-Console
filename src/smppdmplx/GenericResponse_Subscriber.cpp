#include <memory>

#include "GenericResponse_Subscriber.hpp"
#include "SMPP_GenericResponse.hpp"
#include "SMPPSession.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SessionCache.hpp"
#include "CacheOfSMPP_message_traces.hpp"
#include "BufferedOutputStream.hpp"
#include "SocketPool_Singleton.hpp"
#include "PendingOutDataQueue.hpp"
#include "SessionHelper.hpp"

namespace smpp_dmplx {

GenericResponse_Subscriber::GenericResponse_Subscriber()
  : _log(smsc::logger::Logger::getInstance("msg_hndlr")) {}

GenericResponse_Subscriber::~GenericResponse_Subscriber()
{}

SMPP_Subscriber::handle_result_t
GenericResponse_Subscriber::handle(std::auto_ptr<SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_GenericResponse::GENERIC_RESPONSE ) {
    SMPP_GenericResponse* genericResponse = static_cast<SMPP_GenericResponse*>(smpp.get());

    smsc_log_info(_log,"GenericResponse_Subscriber::handle::: got GENERIC_RESPONSE message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), genericResponse->toString().c_str());

    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);

    if ( sessionSearchResult.first == true ) {
      // ����� � ���� ������, ��������������� ������, �� ������� 
      // ������� ������.
      // ��������� ������ ����� ���� ��� ������� � SMSC, ��� � ������� � SME.
      SMPPSession& smppSession = sessionSearchResult.second;

      // �� �������� ������ �������� ����� ������ � SMSC. ���� �����,
      // �� ���� ������ - ��� ������ � SME, ����� ��� ������ � SMSC.
      std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
        SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);

      if ( sessonToSMSC_searchResult.first == true ) {
        forwardMessageToSmsc(genericResponse, smppSession, sessonToSMSC_searchResult.second);
      } else 
        forwardMessageToSme(genericResponse, smppSession);
    } else
      // ��� ������, �� ������� ������� ������, ��� ������������������ ������.
      // ����� ������������ ����������  �  ������� ����� �� ����.
      SessionHelper::terminateSessionToSme(socket);

    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}

void
GenericResponse_Subscriber::forwardMessageToSmsc(SMPP_GenericResponse* genericResponse,
                                                 SMPPSession& smppSessionToSme,
                                                 SMPPSession& sessionToSMSC)
{
  if ( smppSessionToSme.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
    // SME ������� ���������� ���������, �� ������ ��������� � ���������,
    // �� ����������� ��������� ���������� �������. ������� ������
    // �� ����, ����� ������������ ���������� ��� ����� SME � 
    // ������� ����� �� ����.
    SessionHelper::terminateSessionToSme(smppSessionToSme.getSocketToPeer());
  else {
    // �����, �������������� �������� genericResponse - ��� ����� �� ������,
    // ���������� �� SMSC � SME.
    std::auto_ptr<BufferedOutputStream> outBuf = genericResponse->marshal();
    smsc_log_info(_log,"GenericResponse_Subscriber::forwardMessageToSmsc::: send GENERIC_RESPONSE message to SMSC. Message dump=[%s]", genericResponse->toString().c_str());
    PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf,sessionToSMSC.getSocketToPeer());
  }
}

void
GenericResponse_Subscriber::forwardMessageToSme(SMPP_GenericResponse* genericResponse,
                                                SMPPSession& sessionToSmsc)
{
  if ( sessionToSmsc.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS )
    // SMSC ������� ������, �� ������ ��������� � ���������,
    // �� ����������� ��������� ���������� �������. ������� ������
    // �� ����, ����� ������������ ���������� � SMSC � 
    // ������� ����� �� ����.
    SessionHelper::terminateSessionToSmsc(sessionToSmsc.getSocketToPeer(), sessionToSmsc.getSystemId());
  else {
    // �����, �������������� �������� genericResponse - ��� ����� �� ������,
    // ���������� �� SME � SMSC.
    // ���� ���� ������� � ����.
    try {
      CacheOfSMPP_message_traces::MessageTrace_t messageTrace = 
        CacheOfSMPP_message_traces::getInstance().getMessageTraceFromCache(genericResponse->getSequenceNumber(), sessionToSmsc.getSystemId());
      genericResponse->setSequenceNumber(messageTrace.first);
      std::auto_ptr<BufferedOutputStream> outBuf = genericResponse->marshal();
      smsc_log_info(_log,"GenericResponse_Subscriber::forwardMessageToSme::: send GENERIC_RESPONSE message to SME. Message dump=[%s]", genericResponse->toString().c_str());
      PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf,messageTrace.second.getSocketToPeer());
    } catch (MessageTraceNotFound& ex) {
      smsc_log_info(_log,"GenericResponse_Subscriber::forwardMessageToSme: Catched exception [%s]", ex.what());
    }
  }
}

}
