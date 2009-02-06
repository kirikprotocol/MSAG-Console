#include <time.h>
#include <stdlib.h>

#include "IdleSocketsPool.hpp"

namespace smpp_dmplx {

IdleSocketsPool::IdleSocketsPool()
  : _log(smsc::logger::Logger::getInstance("idlsockpool")) {}

bool
IdleSocketsPool::getTimedOutSocketsList(IdleSocketList_t* timedOutSocketlist)
{
  if ( _idle_socket_list.empty() )
    return false;
  else {
    IdleSocketList_t::iterator iter = _idle_socket_list.begin();
    // список отсортирован по возрастанию значения таймаута
    while (iter != _idle_socket_list.end() && iter->second - ::time(NULL) <= 0)
      ++iter;

    timedOutSocketlist->splice(timedOutSocketlist->end(), _idle_socket_list, _idle_socket_list.begin(), iter);

    if ( !timedOutSocketlist->empty() )
      return true;
    else
      return false;
  }
}

void
IdleSocketsPool::insertWaitingSocket(const smsc::core_ax::network::Socket& socket, time_t socketTimeoutValue)
{
  IdleSocketList_t::iterator iter;
  for(iter = _idle_socket_list.begin(); iter != _idle_socket_list.end(); ++iter)
    if ( iter->first == socket ) {
      smsc_log_debug(_log,"IdleSocketsPool::insertWaitingSocket::: erase Socket_Timeout_pair_t from _idle_socket_list (socket=%s)", iter->first.toString().c_str());
      _idle_socket_list.erase(iter); break;
    }

  time_t realTimeOutValue = ::time(NULL) + socketTimeoutValue;
  for(iter=_idle_socket_list.begin(); 
      iter != _idle_socket_list.end() && iter->second < realTimeOutValue;
      ++iter);

  smsc_log_debug(_log,"IdleSocketsPool::insertWaitingSocket::: insert Socket_Timeout_pair_t(socket=%s,realTimeOutValue=%u)", socket.toString().c_str(), realTimeOutValue);
  _idle_socket_list.insert(iter, Socket_Timeout_pair_t(socket, realTimeOutValue));
}

void
IdleSocketsPool::removeActiveSocket(const smsc::core_ax::network::Socket& socket)
{
  for(IdleSocketList_t::iterator iter = _idle_socket_list.begin(); iter != _idle_socket_list.end(); ++iter) {
    if ( iter->first == socket ) {
      smsc_log_debug(_log,"IdleSocketsPool::removeActiveSocket::: remove Socket_Timeout_pair_t(socket=%s,realTimeOutValue=%u)", iter->first.toString().c_str(), iter->second);
      _idle_socket_list.erase(iter); break;
    }
  }
}

}
