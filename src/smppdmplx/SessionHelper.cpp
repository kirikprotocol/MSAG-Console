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
    // Удаляем сессию для сокета с SME.
    SessionCache::getInstance().removeSession(socketToSme);
  }
  // Удаляем сокет с SME из пула сокетов для прослушивания запросов.
  SocketPool_Singleton::getInstance().remove_socket(socketToSme);
  IdleSocketsPool::getInstance().removeActiveSocket(socketToSme);
}

void
smpp_dmplx::SessionHelper::terminateSessionToSmsc(smsc::core_ax::network::Socket& socketToSmsc, const std::string& systemId)
{
  // Удаляем сессию для сокета с SMSC.
  SessionCache::getInstance().removeSession(socketToSmsc);
  // Удаляем из таблицы диспетчера информацию о сессии с SMSC.
  SMPPSessionSwitch::getInstance().dropSharedSessionToSMSC(systemId);
  // Удаляем сокет с SMSC из пула сокетов для прослушивания запросов.
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

    // По значению сессии пытаемся найти сессию с SMSC. Если нашли,
    // то наша сессия - это сессия с SME, иначе это сессия с SMSC.
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
