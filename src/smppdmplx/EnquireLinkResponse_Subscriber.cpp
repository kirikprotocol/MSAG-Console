#include <memory>

#include "EnquireLinkResponse_Subscriber.hpp"
#include "Publisher.hpp"
#include "SMPP_EnquireLink_Resp.hpp"

#include "SMPP_Constants.hpp"
#include "SMPPSession.hpp"
#include "SessionCache.hpp"
#include "SocketPool_Singleton.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

static int toRegisterSubscriber() {
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::EnquireLinkResponse_Subscriber());

  return 0;
}

static int subscriberIsRegistered = toRegisterSubscriber();

smpp_dmplx::EnquireLinkResponse_Subscriber::~EnquireLinkResponse_Subscriber() {}

smpp_dmplx::SMPP_Subscriber::handle_result_t
smpp_dmplx::EnquireLinkResponse_Subscriber::handle(std::auto_ptr<smpp_dmplx::SMPP_message>& smpp, smsc::core_ax::network::Socket& socket)
{
  if ( smpp->getCommandId() == SMPP_message::ENQUIRE_LINK_RESP ) {
    SMPP_EnquireLink_Resp* enquireResponse = dynamic_cast<SMPP_EnquireLink_Resp*>(smpp.get());

    smsc_log_info(dmplxlog,"EnquireLinkResponse_Subscriber::handle::: got ENQUIRE_LINK_RESPONSE message for processing from socket=[%s]. Message dump=[%s]", socket.toString().c_str(), enquireResponse->toString().c_str());

    SessionCache::search_result_t searchResult =
      SessionCache::getInstance().getSession(socket);

    if ( searchResult.first == false ) {
      // ��� ������, �� ������� ������� ������, ��� ������������������ ������.
      // ����� ������������ ����������  �  ������� ����� �� ����.
      SocketPool_Singleton::getInstance().remove_socket(socket);
    }
    SMPPSession& smppSession = searchResult.second;
    // ���������, ��� ������ � SME/SMSC ��������� � ���������� ��������� - � �������, �� �������
    if ( smppSession.updateSessionState(SMPPSession::GOT_APP_MESSAGE) != SMPPSession::OPERATION_SUCCESS ) {
      // SME/SMSC ������� ������ EnquireLink_Resp, �� ������ ��������� � ���������,
      // �� ����������� ��������� ���������� �������. ������� ������
      // �� ����, ����� ������������ ���������� ��� ����� SME/SMSC � 
      // ������� ����� �� ����.
      SessionCache::getInstance().removeSession(socket);
      SocketPool_Singleton::getInstance().remove_socket(socket);
    } else {
    }
    return RequestWasProcessed;
  } else
    return RequestIsNotForMe;
}
