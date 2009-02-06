#include "SessionCache.hpp"
#include <utility>

namespace smpp_dmplx {

SessionCache::SessionCache()
  : _log(smsc::logger::Logger::getInstance("scache")) {}

void
SessionCache::makeSocketSessionEntry(smsc::core_ax::network::Socket& socket,
                                     SMPPSession& session)
{
  smsc_log_debug(_log, "SessionCache::makeSocketSessionEntry::: add <socket,session> pair to cache for socket=[%s] and session=[%s]", socket.toString().c_str(), session.toString().c_str());
  _socketToSessionMap.insert(std::make_pair(socket, session));
}

SessionCache::search_result_t
SessionCache::getSession(smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(_log, "SessionCache::getSession::: Try find session in cache for socket=[%s]", socket.toString().c_str());
  socket_to_session_map_t::iterator iter = _socketToSessionMap.find(socket);
  if ( iter == _socketToSessionMap.end() ) {
    smsc_log_debug(_log, "SessionCache::getSession::: session was not found");
    return std::make_pair(false, SMPPSession());
  } else {
    smsc_log_debug(_log, "SessionCache::getSession::: session was found. Session=[%s]", iter->second.toString().c_str());
    return std::make_pair(true, iter->second);
  }
}

void
SessionCache::removeSession(smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(_log, "SessionCache::removeSession::: remove <socket,session> pair from cache for socket=[%s]", socket.toString().c_str());
  unsigned totalRemoved = _socketToSessionMap.erase(socket);
  if ( totalRemoved )
    smsc_log_debug(_log, "SessionCache::removeSession::: <socket,session> pair for socket=[%s] has been removed", socket.toString().c_str());
}

}
