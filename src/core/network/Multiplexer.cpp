#include "Multiplexer.hpp"
#include <string.h>

namespace smsc{
namespace core{
namespace network{

using smsc::core::buffers::Array;

int Multiplexer::checkState(bool isReadMode,SockArray& ready,SockArray& error,int timeout)
{
  int mask = isReadMode ? POLLIN : POLLOUT;
  for(int i=1, ie=fds.Count();i<ie;i++)
  {
    fds[i].events=mask;
  }
  ready.Empty();
  error.Empty();
  const int err=POLLNVAL|POLLERR|POLLHUP;
  int wake = 0;
  if (fds[0].fd == -1) {
      // wake is not used
      const int retval = poll(&fds[1],fds.Count()-1,timeout);
      if (retval <= 0) return 0; // timeout
  } else {
      // wake is used
      const int retval = poll(&fds[0],fds.Count(),timeout);
      if (retval <= 0) return 0;
      // checking the wake fd
      if ( fds[0].revents & POLLIN ) {
          wake = -1;
      } else if ( fds[0].revents & err ) {
          // "detaching" the pipe as it has errors
          fds[0].fd = -1;
      }
  }
  for(int i=1, ie=fds.Count(); i<ie; i++)
  {
/*
 * (xom 28.07.11) due to multimasking of revent
 * it seems to better check I/O signals first instead of errors
 * to avoid of lost the tail of message when read mode
 */
	if ( isReadMode && (fds[i].revents&POLLIN) ) {
		ready.Push(sockets[i-1]);
		continue;
	}
	if(fds[i].revents&err) {
		error.Push(sockets[i-1]);
		continue;
	}
	if ( (!isReadMode) && (fds[i].revents&POLLOUT) ) {
		ready.Push(sockets[i-1]);
		continue;
	}
  }
  return wake ? wake : ready.Count()+error.Count();
}

}//network
}//core
}//smsc
