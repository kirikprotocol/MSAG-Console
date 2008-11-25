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
  smsc_log_debug(logger, "can read/write");
  read.Empty();
  write.Empty();
  error.Empty();

  int fdsCount = fds.Count();
  if(!fdsCount || poll(&fds[0], fdsCount, timeout) <= 0) {
    return 0;
  }
  smsc_log_debug(logger, "%d sockets can read/write", fdsCount);
  for(int i = 0; i < fdsCount; ++i)
  {
    Socket *s = sockets[i];
    short revents = fds[i].revents;
    if(revents & (POLLNVAL | POLLERR | POLLHUP))
	{
		error.Push(s);
		continue;
	}
    if(revents & POLLIN) {
      read.Push(s);
      smsc_log_debug(logger, "socket: %p ready for Read", s);
    } else if(revents & POLLOUT) {
      write.Push(s);
      smsc_log_debug(logger, "socket: %p ready for Write", s);
    }
  }

  return read.Count() + write.Count() + error.Count();
}

}}
