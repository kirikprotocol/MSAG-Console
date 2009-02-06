#include "CacheOfIncompleteReadMessages.hpp"

namespace smpp_dmplx {

CacheOfIncompleteReadMessages::CacheOfIncompleteReadMessages()
  : _log(smsc::logger::Logger::getInstance("incomplcache"))
{}

RawMessage&
CacheOfIncompleteReadMessages::getIncompleteMessageForSocket(const smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(_log, "CacheOfIncompleteReadMessages::getIncompleteMessageForSocket::: get incomplete message from cache for socket=[%s]", socket.toString().c_str());
  SocketMessageCache_t::iterator iter=_socketMessageCache.find(socket);
  if ( iter == _socketMessageCache.end() ) {
    smsc_log_debug(_log, "CacheOfIncompleteReadMessages::getIncompleteMessageForSocket::: message was not found, insert empty message");
    std::pair<SocketMessageCache_t::iterator, bool> ins_res = _socketMessageCache.insert(std::make_pair(socket, RawMessage()));
    iter = ins_res.first;
  } else
    smsc_log_debug(_log, "CacheOfIncompleteReadMessages::getIncompleteMessageForSocket::: message was found");

  return iter->second;
}

void
CacheOfIncompleteReadMessages::removeCompleteMessageForSocket(const smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(_log, "CacheOfIncompleteReadMessages::removeCompleteMessageForSocket::: remove complete message from cache for socket=[%s]", socket.toString().c_str());
  unsigned totalErased = _socketMessageCache.erase(socket);
  if ( totalErased )
    smsc_log_debug(_log, "CacheOfIncompleteReadMessages::removeCompleteMessageForSocket::: message was removed from cache for socket=[%s]", socket.toString().c_str());
}

}
