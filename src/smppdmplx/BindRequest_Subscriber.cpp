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
      //  ��������� � ���� ������ socket_���_������_�_SME -- ������_�_SME
      SessionCache::getInstance().makeSocketSessionEntry(socket, smppSessionToSME);

      smsc_log_info(_log,"BindRequest_Subscriber::handle::: make SME authentication");
      if ( smppSessionToSME.authenticate(bindRequest->getPassword()) == SMPPSession::AUTH_SUCCESS ) {
        // ���� �������������� �������,
        // �� �������� ��������� ������ � SME �� GOT_BIND_REQ
        smppSessionToSME.updateSessionState(SMPPSession::GOT_BIND_REQ);

        // �������� �� ���� ����������� ������ � SMSC, ���������������
        // ����������� � smpp-������� �������� systemId
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

        // ������� ������ � SME.
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
  // �������� � ��� �����, ��������������� �������������� ���������� � SMSC
  SocketPool_Singleton::getInstance().push_socket(newSmscSession.getSocketToPeer());
  // ��������� �������� sequenceNumber, ���������� �� SME, �� ���������
  // ������������� �������� ��� ������� systemId � ��������� � ���� 
  // ������, �� ������� ������� ������ BindRequest

  uint32_t seqNumToSmsc = SequenceNumberGenerator::getInstance(bindRequest->getSystemId())->getNextValue();
  CacheOfSMPP_message_traces::getInstance().putMessageTraceToCache(bindRequest->getSequenceNumber(), smppSessionToSME, seqNumToSmsc);
  bindRequest->setSequenceNumber(seqNumToSmsc);
  smsc_log_info(_log,"BindRequest_Subscriber::initiateNewSessionToSmsc::: send BIND_REQUEST message to SMSC. Message dump=[%s]", bindRequest->toString().c_str());
  std::auto_ptr<BufferedOutputStream> outBuf = bindRequest->marshal();

  // �������� ������ BindRequest � SMSC
  PendingOutDataQueue::getInstance().scheduleDataForSending(*outBuf, newSmscSession.getSocketToPeer());

  newSmscSession.updateSessionState(SMPPSession::GOT_BIND_REQ);
  // ������������� ������������ systemId -- ������_�_SMSC
  SMPPSessionSwitch::getInstance().setSharedSessionToSMSC(newSmscSession, bindRequest->getSystemId());

  //  ��������� � ���� ������ �����_�_SMSC -- ������_�_SMSC
  SessionCache::getInstance().makeSocketSessionEntry(newSmscSession.getSocketToPeer(),
                                                     newSmscSession);
  // � ��������� ������_��_SME � ������ �������� ������ � ���������
  // ��������� systemId
  SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
}

void
BindRequest_Subscriber::confirmIncomingSessionRequest(SMPP_BindRequest* bindRequest,
                                                      SMPPSession& smppSessionToSME)
{
  // ������ � SMSC ��� ������� �������� systemId ��� �����������.
  // ���������� �������� ������������� ����� �� ���������� ������
  // BindRequest
  prepareBindResponseMessage(smppSessionToSME.getSocketToPeer(), bindRequest, ESME_ROK);
  // �������� ��������� ������ � SME
  smppSessionToSME.updateSessionState(SMPPSession::GOT_BIND_RESP);
  // � ��������� ������_��_SME � ������ �������� ������ � ���������
  // ��������� systemId
  SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
}

void
BindRequest_Subscriber::pendIncomingSessionRequest(SMPP_BindRequest* bindRequest,
                                                   SMPPSession& smppSessionToSME)
{
  // ��� ������� systemId � SMSC ��� ������ ������ BindRequest,
  // ������� ��������� ������ BindRequest � SMSC �� ��������. 
  // ������ ����� ��������� ������ ��� ������� SME � ����. 
  // ��� ��������� ������ �� SMSC �� ��������� ������ BindRequest
  // ��� ������� systemId, ��� ���� ������������ �� ������ ������
  // �������� ��������� ������ �� ��������, ��������������� �������
  // ����������� ������ �� SMSC � ��������� �������������� ������
  // �� ��������������� SME.
  CacheOfPendingBindReqFromSME::getInstance().putPendingReqToCache(smppSessionToSME, bindRequest);
  // � ��������� ������_��_SME � ������ �������� ������ � ���������
  // ��������� systemId
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
