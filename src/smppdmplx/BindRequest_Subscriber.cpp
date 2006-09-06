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
      //  ��������� � ���� ������ socket_���_������_�_SME -- ������_�_SME
      SessionCache::getInstance().makeSocketSessionEntry(socket,
                                                         smppSessionToSME);
      smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: make SME authentication");
      if ( smppSessionToSME.authenticate(bindRequest->getPassword()) == SMPPSession::AUTH_SUCCESS ) {
        // ���� �������������� �������,
        // �� �������� ��������� ������ � SME �� GOT_BIND_REQ
        smppSessionToSME.updateSessionState(SMPPSession::GOT_BIND_REQ);

        // �������� �� ���� ����������� ������ � SMSC, ���������������
        // ����������� � smpp-������� �������� systemId
        SMPPSessionSwitch::search_result_t smscSessFromCache =
          SMPPSessionSwitch::getInstance().getSharedSessionToSMSC(bindRequest->getSystemId());

        if ( smscSessFromCache.first == false ) {
          // ���� ����������� ������ � smsc ��� �� ����������, �� ������� ��.
          smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: Try establish smpp session with SMSC");
          SMPPSession newSmscSession(bindRequest->getSystemId());
          newSmscSession.connectSession();
          smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: connect to SMSC was established");
          // �������� � ��� �����, ��������������� �������������� ���������� � SMSC
          SocketPool_Singleton::getInstance().push_socket(newSmscSession.getSocketToPeer());
          // ��������� �������� sequenceNumber, ���������� �� SME, �� ���������
          // ������������� �������� ��� ������� systemId � ��������� � ���� 
          // ������, �� ������� ������� ������ BindRequest
          uint32_t seqNumToSmsc = 
            CacheOfSMPP_message_traces::getInstance().putMessageTraceToCache(smpp->getSequenceNumber(), smppSessionToSME);
          smpp->setSequenceNumber(seqNumToSmsc);
          smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: send BIND_REQUEST message to SMSC. Message dump=[%s]", smpp->toString().c_str());
          std::auto_ptr<BufferedOutputStream> outBuf = smpp->marshal();

          // �������� ������ BindRequest � SMSC
          PendingOutDataQueue::scheduleDataForSending(*outBuf, newSmscSession.getSocketToPeer());

          newSmscSession.updateSessionState(SMPPSession::GOT_BIND_REQ);
          // ������������� ������������ systemId -- ������_�_SMSC
          SMPPSessionSwitch::getInstance().setSharedSessionToSMSC(newSmscSession, bindRequest->getSystemId());

          //  ��������� � ���� ������ �����_�_SMSC -- ������_�_SMSC
          SessionCache::getInstance().makeSocketSessionEntry(newSmscSession.getSocketToPeer(),
                                                             newSmscSession);
          // � ��������� ������_��_SME � ������ �������� ������ � ���������
          // ��������� systemId
          SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
        } else {
          if ( !smscSessFromCache.second.bindInProgress() ) {
            // ������ � SMSC ��� ������� �������� systemId ��� �����������.
            // ���������� �������� ������������� ����� �� ���������� ������
            // BindRequest
            std::auto_ptr<SMPP_BindResponse> bindResponse(bindRequest->prepareResponse(ESME_ROK));
            smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: send positive BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
            std::auto_ptr<BufferedOutputStream> outBuf = bindResponse->marshal();
            PendingOutDataQueue::scheduleDataForSending(*outBuf, socket);
            // �������� ��������� ������ � SME
            smppSessionToSME.updateSessionState(SMPPSession::GOT_BIND_RESP);
            // � ��������� ������_��_SME � ������ �������� ������ � ���������
            // ��������� systemId
            SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);

          } else {
            // ��� ������� systemId � SMSC ��� ������ ������ BindRequest,
            // ������� ��������� ������ BindRequest � SMSC �� ��������. 
            // ������ ����� ��������� ������ ��� ������� SME � ����. 
            // ��� ��������� ������ �� SMSC �� ��������� ������ BindRequest
            // ��� ������� systemId, ��� ���� ������������ �� ������ ������
            // �������� ��������� ������ �� ��������, ��������������� �������
            // ����������� ������ �� SMSC � ��������� �������������� ������
            // �� ��������������� SME.
            CacheOfPendingBindReqFromSME::getInstance().putPendingReqToCache(smppSessionToSME, dynamic_cast<SMPP_BindRequest*>(smpp.release()));
            // � ��������� ������_��_SME � ������ �������� ������ � ���������
            // ��������� systemId
            SMPPSessionSwitch::getInstance().addActiveSmeSession(smppSessionToSME);
          }
        }
      } else {
        // ����������� BindResponse �� �������� ESME_RINVPASWD
        std::auto_ptr<SMPP_BindResponse> bindResponse(bindRequest->prepareResponse(ESME_RINVPASWD));
        std::auto_ptr<BufferedOutputStream> outBuf = bindResponse->marshal();
        smsc_log_info(dmplxlog,"BindRequest_Subscriber::handle::: send negative BIND_RESPONSE message to SME. Message dump=[%s]", bindResponse->toString().c_str());
        PendingOutDataQueue::scheduleDataForSending(*outBuf, socket);
        // ������� ������ � SME.
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
