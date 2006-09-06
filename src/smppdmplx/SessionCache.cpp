#include "SessionCache.hpp"
#include <utility>

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

void
smpp_dmplx::SessionCache::makeSocketSessionEntry(smsc::core_ax::network::Socket& socket,
                                                 SMPPSession& session)
{
  smsc_log_debug(dmplxlog, "SessionCache::makeSocketSessionEntry::: add <socket,session> pair to cache for socket=[%s] and session=[%s]", socket.toString().c_str(), session.toString().c_str());
  _socketToSessionMap.insert(std::make_pair(socket, session));
}

smpp_dmplx::SessionCache::search_result_t
smpp_dmplx::SessionCache::getSession(smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(dmplxlog, "SessionCache::getSession::: Try find session in cache for socket=[%s]", socket.toString().c_str());
  socket_to_session_map_t::iterator iter = _socketToSessionMap.find(socket);
  if ( iter == _socketToSessionMap.end() ) {
    smsc_log_debug(dmplxlog, "SessionCache::getSession::: session was not found");
    return std::make_pair(false, SMPPSession());
  } else {
    smsc_log_debug(dmplxlog, "SessionCache::getSession::: session was found. Session=[%s]", iter->second.toString().c_str());
    return std::make_pair(true, iter->second);
  }
}

void
smpp_dmplx::SessionCache::removeSession(smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(dmplxlog, "SessionCache::removeSession::: remove <socket,session> pair from cache for socket=[%s]", socket.toString().c_str());
  _socketToSessionMap.erase(socket);
}
