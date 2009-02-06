#include <logger/Logger.h>
#include "SessionHelper.hpp"
#include "SessionCache.hpp"
#include "SMPPSessionSwitch.hpp"
#include "SocketPool_Singleton.hpp"
#include "IdleSocketsPool.hpp"
#include "PendingOutDataQueue.hpp"
#include "CacheOfSMPP_message_traces.hpp"

namespace smpp_dmplx {

void
SessionHelper::terminateSessionToSme(smsc::core_ax::network::Socket& socketToSme)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sesshlpr");

  smsc_log_info(logger, "SessionHelper::terminateSessionToSme::: try terminate session to sme for socket [%s]", socketToSme.toString().c_str());
  SessionCache::search_result_t searchRes = SessionCache::getInstance().getSession(socketToSme);
  if ( searchRes.first ) {
    SMPPSessionSwitch::getInstance().removeSmeSession(searchRes.second);
    // Удаляем сессию для сокета с SME.
    SessionCache::getInstance().removeSession(socketToSme);
  }
  // Удаляем сокет с SME из пула сокетов для прослушивания запросов.
  SocketPool_Singleton::getInstance().remove_socket(socketToSme);
  IdleSocketsPool::getInstance().removeActiveSocket(socketToSme);
  PendingOutDataQueue::getInstance().cancelScheduledData(socketToSme);
  CacheOfSMPP_message_traces::getInstance().removeMessageTracesFromCache(socketToSme);
  smsc_log_info(logger, "SessionHelper::terminateSessionToSme::: session to sme for socket [%s] has been terminated", socketToSme.toString().c_str());
}

void
SessionHelper::terminateSessionToSmsc(smsc::core_ax::network::Socket& socketToSmsc, const std::string& systemId)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sesshlpr");

  smsc_log_info(logger, "SessionHelper::terminateSessionToSmsc::: try terminate session to smsc for socket [%s]", socketToSmsc.toString().c_str());
  // Удаляем сессию для сокета с SMSC.
  SessionCache::getInstance().removeSession(socketToSmsc);
  // Удаляем из таблицы диспетчера информацию о сессии с SMSC.
  SMPPSessionSwitch::getInstance().dropSharedSessionToSMSC(systemId);
  // Удаляем сокет с SMSC из пула сокетов для прослушивания запросов.
  SocketPool_Singleton::getInstance().remove_socket(socketToSmsc);
  IdleSocketsPool::getInstance().removeActiveSocket(socketToSmsc);
  PendingOutDataQueue::getInstance().cancelScheduledData(socketToSmsc);
  smsc_log_info(logger, "SessionHelper::terminateSessionToSmsc::: session to smsc for socket [%s] has been terminated", socketToSmsc.toString().c_str());
}

void
SessionHelper::dropActiveSession(smsc::core_ax::network::Socket& socket)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sesshlpr");
  smsc_log_info(logger, "SessionHelper::dropActiveSession::: try terminate session for socket [%s]", socket.toString().c_str());

  SessionCache::search_result_t searchRes = SessionCache::getInstance().getSession(socket);

  if ( searchRes.first ) {
    SMPPSession& smppSession = searchRes.second;
    //    std::string systemId = smppSession.getSystemId();

    // По значению сессии пытаемся найти сессию с SMSC. Если нашли,
    // то наша сессия - это сессия с SME, иначе это сессия с SMSC.
    std::pair<bool,SMPPSession> sessonToSMSC_searchResult =
      SMPPSessionSwitch::getInstance().getCrossedSession(smppSession);
    if ( sessonToSMSC_searchResult.first == true ) {
      terminateSessionToSme(socket);
      //      SMPPSessionSwitch::getInstance().removeSmeSession(smppSession);
      //      CacheOfSMPP_message_traces::getInstance().removeMessageTracesFromCache(socket);
    } else {
      terminateSessionToSmsc(sessonToSMSC_searchResult.second.getSocketToPeer(), sessonToSMSC_searchResult.second.getSystemId());
      //SMPPSessionSwitch::getInstance().dropSharedSessionToSMSC(systemId);
    }
    //SessionCache::getInstance().removeSession(socket);
  }
  //  SocketPool_Singleton::getInstance().remove_socket(socket);
  //  IdleSocketsPool::getInstance().removeActiveSocket(socket);
  //  PendingOutDataQueue::getInstance().cancelScheduledData(socket);
  smsc_log_info(logger, "SessionHelper::dropActiveSession::: session for socket [%s] has been terminated", socket.toString().c_str());
}

}
