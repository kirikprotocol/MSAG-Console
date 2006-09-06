#include "CacheOfIncompleteReadMessages.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

smpp_dmplx::RawMessage&
smpp_dmplx::CacheOfIncompleteReadMessages::getIncompleteMessageForSocket(const smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(dmplxlog, "SessionCache::getIncompleteMessageForSocket::: get incomplete message from cache for socket=[%s]", socket.toString().c_str());
  SocketMessageCache_t::iterator iter=_socketMessageCache.find(socket);
  if ( iter == _socketMessageCache.end() ) {
    smsc_log_debug(dmplxlog, "SessionCache::getIncompleteMessageForSocket::: message was not found");
    _socketMessageCache[socket] = RawMessage();
    std::pair<SocketMessageCache_t::iterator, bool> ins_res = _socketMessageCache.insert(std::make_pair(socket, RawMessage()));
    iter = ins_res.first;
  } else
    smsc_log_debug(dmplxlog, "SessionCache::getIncompleteMessageForSocket::: message was found");

  return iter->second;
}

void
smpp_dmplx::CacheOfIncompleteReadMessages::removeCompleteMessageForSocket(const smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(dmplxlog, "SessionCache::removeCompleteMessageForSocket::: remove complete message from cache for socket=[%s]", socket.toString().c_str());
  SocketMessageCache_t::iterator iter=_socketMessageCache.find(socket);
  if ( iter != _socketMessageCache.end() )
    _socketMessageCache.erase(iter);
}
