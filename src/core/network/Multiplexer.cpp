#include "Multiplexer.hpp"
#include <string.h>

namespace smsc{
namespace core{
namespace network{

using smsc::core::buffers::Array;

int Multiplexer::checkState(int mode,SockArray& ready,SockArray& error,int timeout)
{
  int i;
  int mask;
  if(mode==STATE_MODE_READ)
  {
    mask=POLLIN;
  }else
  if(mode==STATE_MODE_WRITE)
  {
    mask=POLLOUT;
  }
  for(i=0;i<sockets.Count();i++)
  {
    fds[i].events=mask;
  }
  ready.Clean();
  error.Clean();
  int retval=poll(&fds[0],fds.Count(),timeout);
  if(retval<=0)return 0;
  int err=POLLNVAL|POLLERR|POLLHUP;
  for(int i=0;i<fds.Count();i++)
  {
    if(fds[i].revents&err)
    {
      error.Push(sockets[i]);
      continue;
    }
    if(fds[i].revents&mask)
    {
      ready.Push(sockets[i]);
    }
  }
  return ready.Count()+error.Count();
}

};//network
};//core
};//smsc
