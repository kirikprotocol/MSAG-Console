#ifndef __CORE_NETWORK_MULTIPLEXER_HPP__
#define __CORE_NETWORK_MULTIPLEXER_HPP__

#include "core/network/Socket.hpp"
#include "core/buffers/Array.hpp"
#include <poll.h>

namespace smsc{
namespace core{
namespace network{

class Multiplexer{
protected:
  enum{STATE_MODE_READ=0,STATE_MODE_WRITE=1};
public:
  typedef smsc::core::buffers::Array<Socket*> SockArray;
  Multiplexer(){}
  virtual ~Multiplexer(){}

  int add(Socket* sock)
  {
    if(exists(sock))return 0;
    sockets.Push(sock);
    int i=sockets.Count()-1;
    fds[i].fd=sock->getSocket();
    return 1;
  }
  int remove(Socket* sock)
  {
    for(int i=0;i<sockets.Count();i++)
    {
      if(sockets[i]==sock)
      {
        sockets.Delete(i);
        fds.Delete(i);
        return 1;
      }
    }
    return 0;
  }
  int exists(Socket* sock)
  {
    for(int i=0;i<sockets.Count();i++)
    {
      if(sockets[i]==sock)return 1;
    }
    return 0;
  }
  void clear()
  {
    sockets.Empty();
    fds.Empty();
  }
  int canRead(SockArray& ready,SockArray& error,int timeout=-1)
  {
    return checkState(STATE_MODE_READ,ready,error,timeout);
  }
  int canWrite(SockArray& ready,SockArray& error,int timeout=-1)
  {
    return checkState(STATE_MODE_WRITE,ready,error,timeout);
  }

  int getSize()const
  {
    return sockets.Count();
  }

protected:
  SockArray sockets;
  smsc::core::buffers::Array<pollfd> fds;

  int checkState(int mode,SockArray& ready,SockArray& error,int timeout);
};

}//network
}//core
}//smsc


#endif
