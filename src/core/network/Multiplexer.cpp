#include "Multiplexer.hpp"
#include <string.h>

namespace smsc{
namespace core{
namespace network{

using smsc::core::buffers::Array;

int Multiplexer::checkState(int mode,SockArray& ready,int timeout=-1)
{
  fd_set fd;
  FD_ZERO(&fd);
  int i;
  for(i=0;i<sockets.Count();i++)
  {
    FD_SET(sockets[i]->getSocket(),&fd);
  }
  timeval tv;
  tv.tv_sec=timeout/1000;
  tv.tv_usec=(timeout%1000)*1000;
  int retval=select(FD_SETSIZE,
                    mode==0?&fd:NULL,
                    mode==1?&fd:NULL,
                    mode==2?&fd:NULL,
                    timeout==-1?NULL:&tv);
  if(retval<=0)return retval;
  ready.Clean();
  for(i=0;i<sockets.Count();i++)
  {
    if(FD_ISSET(sockets[i]->getSocket(),&fd))
    {
      ready.Push(sockets[i]);
      retval--;
      if(retval==0)return ready.Count();
    }
  }
  return ready.Count();
}

};//network
};//core
};//smsc

