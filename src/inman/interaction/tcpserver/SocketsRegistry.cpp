#ifndef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/tcpserver/SocketsRegistry.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

/* ************************************************************************** *
 * class SocketsRegistry implementation:
 * ************************************************************************** */
SocketInfo * SocketsRegistry::insert(const SocketInfo & sock_inf)
{
  std::pair<SocketsMap::iterator, bool>
    res = _sockMap.insert(SocketsMap::value_type(sock_inf.getUId(), sock_inf));
  _isModified = true;
  return &(res.first->second);
}
//
bool SocketsRegistry::erase(iterator use_it)
{
  if (use_it != _sockMap.end()) {
    _sockMap.erase(use_it);
    return (_isModified = true);
  }
  return false;
}
//
bool SocketsRegistry::erase(unsigned conn_id)
{
  return erase(_sockMap.find(conn_id));
}

pollfd_arr SocketsRegistry::composeFds(void)
{
  if (_isModified) {
    _fds.clear();
    _fds.resize((FDsArray::size_type)_sockMap.size());
    int i = 0;
    for (SocketsMap::const_iterator
          it = _sockMap.begin(); it != _sockMap.end(); ++it, ++i) {
      _fds[i].fd = it->second.getFd();
      _fds[i].events = POLLIN;
      _fds[i].revents = 0;
    }
    _isModified = false;
  }
  return pollfd_arr(_fds.size(), &_fds[0]);
}

} // interaction
} // inman
} // smsc

