#include "SessionHelper.hpp"
#include "SessionCache.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SocketPool_Singleton.hpp"
#include "IdleSocketsPool.hpp"

void
smpp_dmplx::SessionHelper::terminateSessionToSme(smsc::core_ax::network::Socket& socketToSme)
{
  SessionCache::search_result_t searchRes = SessionCache::getInstance().getSession(socketToSme);
  if ( searchRes.first ) {
    SMPPSessionSwitch::getInstance().removeSmeSession(searchRes.second);
    // ������� ������ ��� ������ � SME.
    SessionCache::getInstance().removeSession(socketToSme);
  }
  // ������� ����� � SME �� ���� ������� ��� ������������� ��������.
  SocketPool_Singleton::getInstance().remove_socket(socketToSme);
  IdleSocketsPool::getInstance().removeActiveSocket(socketToSme);
}

void
smpp_dmplx::SessionHelper::terminateSessionToSmsc(smsc::core_ax::network::Socket& socketToSmsc, const std::string& systemId)
{
  // ������� ������ ��� ������ � SMSC.
  SessionCache::getInstance().removeSession(socketToSmsc);
  // ������� �� ������� ���������� ���������� � ������ � SMSC.
  SMPPSessionSwitch::getInstance().dropSharedSessionToSMSC(systemId);
  // ������� ����� � SMSC �� ���� ������� ��� ������������� ��������.
  SocketPool_Singleton::getInstance().remove_socket(socketToSmsc);
  IdleSocketsPool::getInstance().removeActiveSocket(socketToSmsc);
}

void
smpp_dmplx::SessionHelper::dropActiveSession(smsc::core_ax::network::Socket& socket)
{
  SessionCache::search_result_t searchRes = SessionCache::getInstance().getSession(socket);

  if ( searchRes.first ) {
    SMPPSession& smppSession = searchRes.second;
    std::string systemId = smppSession.getSystemId();

    // �� �������� ������ �������� ����� ������ � SMSC. ���� �����,
    // �� ���� ������ - ��� ������ � SME, ����� ��� ������ � SMSC.
    std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
      SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);
    if ( sessonToSMSC_searchResult.first == true )
      SMPPSessionSwitch::getInstance().removeSmeSession(smppSession);
    else
      SMPPSessionSwitch::getInstance().dropSharedSessionToSMSC(systemId);
    SessionCache::getInstance().removeSession(socket);
  }
  SocketPool_Singleton::getInstance().remove_socket(socket);
  IdleSocketsPool::getInstance().removeActiveSocket(socket);
}
