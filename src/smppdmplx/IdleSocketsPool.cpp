#include <time.h>
#include <stdlib.h>

#include "IdleSocketsPool.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

bool
smpp_dmplx::IdleSocketsPool::getTimedOutSocketsList(IdleSocketList_t& timedOutSocketlist)
{
  if ( _idle_socket_list.empty() )
    return false;
  else {
    IdleSocketList_t::iterator iter = _idle_socket_list.begin();
    // список отсортирован по возрастанию значения таймаута
    while (iter != _idle_socket_list.end() && iter->second - ::time(NULL) <= 0)
      ++iter;

    timedOutSocketlist.splice(timedOutSocketlist.end(), _idle_socket_list, _idle_socket_list.begin(), iter);

    if ( !timedOutSocketlist.empty() )
      return true;
    else
      return false;
  }
}

void
smpp_dmplx::IdleSocketsPool::insertWaitingSocket(const smsc::core_ax::network::Socket& socket, time_t socketTimeoutValue)
{
  IdleSocketList_t::iterator iter;
  for(iter = _idle_socket_list.begin(); iter != _idle_socket_list.end(); ++iter)
    if ( iter->first == socket ) {
      smsc_log_debug(dmplxlog,"IdleSocketsPool::insertWaitingSocket::: erase Socket_Timeout_pair_t from _idle_socket_list (socket=%s)", iter->first.toString().c_str());
      _idle_socket_list.erase(iter); break;
    }

  time_t realTimeOutValue = ::time(NULL) + socketTimeoutValue;
  for(iter=_idle_socket_list.begin(); 
      iter != _idle_socket_list.end() && iter->second < realTimeOutValue;
      ++iter);

  smsc_log_debug(dmplxlog,"IdleSocketsPool::insertWaitingSocket::: insert Socket_Timeout_pair_t(socket=%s,realTimeOutValue=%u)", socket.toString().c_str(), realTimeOutValue);
  _idle_socket_list.insert(iter, Socket_Timeout_pair_t(socket, realTimeOutValue));

  // for test only
  int i = 0;
  for(iter = _idle_socket_list.begin(); iter != _idle_socket_list.end(); ++iter)
    if ( iter->first == socket ) ++i;
  if ( i != 1 )
    abort();
}

void
smpp_dmplx::IdleSocketsPool::removeActiveSocket(const smsc::core_ax::network::Socket& socket)
{
  for(IdleSocketList_t::iterator iter = _idle_socket_list.begin(); iter != _idle_socket_list.end(); ++iter) {
    if ( iter->first == socket ) {
      smsc_log_debug(dmplxlog,"IdleSocketsPool::removeActiveSocket::: remove Socket_Timeout_pair_t(socket=%s,realTimeOutValue=%u)", iter->first.toString().c_str(), iter->second);
      _idle_socket_list.erase(iter); break;
    }
  }

  // for test only
  int i = 0;
  for(IdleSocketList_t::iterator iter = _idle_socket_list.begin(); iter != _idle_socket_list.end(); ++iter)
    if ( iter->first == socket ) ++i;
  if ( i != 0 )
    abort();
}
