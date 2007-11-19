/* $Id$ */

#include "RWMultiplexer.hpp"

namespace scag{ namespace pers{

using smsc::core::buffers::Array;

int RWMultiplexer::add(Socket* sock, int type)
{
	for(int i = 0; i < sockets.Count(); i++)
	{
		if(sockets[i] == sock)
		{
			fds[i].events = type;
			return 0;
		}
	}
    sockets.Push(sock);
    int i = sockets.Count() - 1;
    fds[i].fd = sock->getSocket();
    fds[i].events = type;
    return 1;
}

int RWMultiplexer::canReadWrite(SockArray& read, SockArray& write, SockArray& error, int timeout)
{
  read.Empty();
  write.Empty();
  error.Empty();

  if(poll(&fds[0], fds.Count(), timeout) <= 0)
	return 0;

  for(int i = 0; i < fds.Count(); i++)
  {
    if(fds[i].revents & (POLLNVAL | POLLERR | POLLHUP))
	{
		error.Push(sockets[i]);
		continue;
	}
    if(fds[i].revents & POLLIN) {
      read.Push(sockets[i]);
      smsc_log_debug(logger, "socket: %p ready for Read", sockets[i]);
    } else if(fds[i].revents & POLLOUT) {
      write.Push(sockets[i]);
      smsc_log_debug(logger, "socket: %p ready for Write", sockets[i]);
    }
  }

  return read.Count() + write.Count() + error.Count();
}

}}
