#ifndef __CORE_NETWORK_MULTIPLEXER_HPP__
#define __CORE_NETWORK_MULTIPLEXER_HPP__

#include "core/network/Socket.hpp"
#include "core/buffers/Array.hpp"

namespace smsc{
namespace core{
namespace network{

using smsc::core::buffers::Array;

class Multiplexer{
public:
  typedef Array<Socket*> SockArray;
  Multiplexer(){}
  virtual ~Multiplexer(){}

  int add(Socket* sock)
  {
    if(exists(sock))return 0;
    sockets.Push(sock);
    return 1;
  }
  int remove(Socket* sock)
  {
    for(int i=0;i<sockets.Count();i++)
    {
      if(sockets[i]==sock)
      {
        sockets.Delete(i);
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
    sockets.Clean();
  }
  int canRead(SockArray& ready,int timeout=-1)
  {
    return checkState(0,ready,timeout);
  }
  int canWrite(SockArray& ready,int timeout=-1)
  {
    return checkState(1,ready,timeout);
  }

protected:
  SockArray sockets;
  Array<int> selected;

  /**
   * mode==0 - check for reading
   * mode==1 - check for writing
   * mode==2 - check for error
   */
  int checkState(int mode,SockArray& ready,int timeout);
};

};//network
};//core
};//smsc


#endif
