#include "Multiplexer.hpp"
#include <string.h>

namespace smsc{
namespace core{
namespace network{

using smsc::core::buffers::Array;

int Multiplexer::checkState(bool isReadMode,SockArray& ready,SockArray& error,int timeout)
{
  int mask = isReadMode ? POLLIN : POLLOUT;
  for(int i=0;i<sockets.Count();i++)
  {
    fds[i].events=mask;
  }
  ready.Empty();
  error.Empty();
  int retval=poll(&fds[0],fds.Count(),timeout);
  if(retval<=0)return 0;
  int err=POLLNVAL|POLLERR|POLLHUP;
  for(int i=0;i<fds.Count();i++)
  {
/*
 * (xom 28.07.11) due to multimasking of revent
 * it seems to better check I/O signals first instead of errors
 * to avoid of lost the tail of message when read mode
 */
	if ( isReadMode && (fds[i].revents&POLLIN) ) {
		ready.Push(sockets[i]);
		continue;
	}
	if(fds[i].revents&err) {
		error.Push(sockets[i]);
		continue;
	}
	if ( (!isReadMode) && (fds[i].revents&POLLOUT) ) {
		ready.Push(sockets[i]);
		continue;
	}
  }
  return ready.Count()+error.Count();
}

}//network
}//core
}//smsc
