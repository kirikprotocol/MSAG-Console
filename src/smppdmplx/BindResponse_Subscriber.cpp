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

    // �� ������, �� ������� �������� ���������, ������� ������ � SMSC.
    SessionCache::search_result_t sessionSearchResult = 
      SessionCache::getInstance().getSession(socket);
    if ( sessionSearchResult.first == false ) {
      // ��������� BindResponse �������� �� �� SMSC
      smsc_log_error(_log,"smpp_dmplx::BindResponse_Subscriber::handle::: It was got BindResponse from socket not for smsc communication");
      // ������� ����� �� ������� �������� ��������� �� ���� �������.
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
    // ���� ������ BIND_REQUEST �� SME �� ��������� �������� ���
    // ����������� ������ BIND_REQUEST � SMSC
    CacheOfSMPP_message_traces::MessageTrace_t messageTrace = 
      CacheOfSMPP_message_traces::getInstance().getMessageTraceFromCache(bindResponse->getSequenceNumber(), systemId);

    // �� ����� ��������� ������ BIND_RESPONSE
    // �������� ��������� ����������� ������ � SMSC
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
  // ��������� ������ � SMSC.
  SessionHelper::terminateSessionToSmsc(socketToSmsc, systemId);
  // ��������� ������ � SME.
  SessionHelper::terminateSessionToSme(smppSession.getSocketToPeer());
}

void
BindResponse_Subscriber::forwardBindResponseMessageToSme(uint32_t originalSeqNum,
                                                         SMPPSession& smppSessionToSme,
                                                         SMPP_BindResponse* bindResponse,
                                                         smsc::core_ax::network::Socket& socketToSmsc,
                                                         const std::string& systemId)
{
  // �������� �������� sequenceNumber ���������, ������������ � ����
  bindResponse->setSequenceNumber(originalSeqNum);

  smsc_log_info(_log,"BindResponse_Subscriber::forwardBindResponseMessageToSme::: send BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
  // ������������� � ����� ��� ������ � �����
  std::auto_ptr<BufferedOutputStream> bufToWriteForSme = bindResponse->marshal();

  // ������� ����� � SME �� �������� ��� ������� ������ ������
  // BindRequest
  PendingOutDataQueue::getInstance().scheduleDataForSending(*bufToWriteForSme, smppSessionToSme.getSocketToPeer());

  // �������� � ���� ������� BindRequest �� ������ SME � ��� �� 
  // systemId. ��� ������� ���������� ������� ������������ ����� �� 
  // �������� ������ bindResponse->getCommandStatus()
  CacheOfPendingBindReqFromSME::getInstance().commitPendingReqInCache(systemId, *bindResponse);
  // ��������� ������ ������
  if ( bindResponse->getCommandStatus() != ESME_ROK )
    // ���� � ������ BIND_RESPONSE �������� ������, �� ������ 
    // ���������� �� ������� � ���������� ������� ����������� ������
    // � SMSC �� ���� ������ ��� ��������� �������� systemId.
    SessionHelper::terminateSessionToSmsc(socketToSmsc, systemId);
  else
    smppSessionToSme.updateSessionState(SMPPSession::GOT_BIND_RESP);
}

}
