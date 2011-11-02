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
  static const bool STATE_MODE_READ = true;
  static const bool STATE_MODE_WRITE = false;
public:
  typedef smsc::core::buffers::Array<Socket*> SockArray;
  Multiplexer( int wakefd = -1 ) {
      fds[0].fd = wakefd;
      fds[0].events = POLLIN;
  }
  virtual ~Multiplexer(){}

  int add(Socket* sock)
  {
    if(exists(sock))return 0;
    sockets.Push(sock);
    int i=sockets.Count();
    fds[i].fd=sock->getSocket();
    return 1;
  }
  int remove(Socket* sock)
  {
    for(int i=0, ie=sockets.Count();i<ie;i++)
    {
      if(sockets[i]==sock)
      {
        sockets.Delete(i);
        fds.Delete(i+1);
        return 1;
      }
    }
    return 0;
  }
  int exists(Socket* sock)
  {
    for(int i=0,ie=sockets.Count();i<ie;i++)
    {
      if(sockets[i]==sock)return 1;
    }
    return 0;
  }
  void clear()
  {
    sockets.Empty();
    fds.Delete(1,fds.Count()-1);
  }
  /// check if some sockets can be read w/o blocking
  /// @return 0 - on timeout
  ///        -1 - on wakeup (if wakeup fd is set)
  ///        >0 - number of ready sockets otherwise
  /// NOTE: if retval!=0 then ready/error arrays will contain nonblocked sockets
  /// NOTE: if retval==-1 then you have to read from wakeup fd.
  int canRead(SockArray& ready,SockArray& error,int timeout=-1)
  {
	  return checkState(STATE_MODE_READ,ready,error,timeout);
  }
  /// see comment to canRead.
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
    /// NOTE: the first fd is a fd of waking pipe
  smsc::core::buffers::Array<pollfd> fds;

  int checkState(bool isReadMode,SockArray& ready,SockArray& error,int timeout);
};

}//network
}//core
}//smsc


#endif
