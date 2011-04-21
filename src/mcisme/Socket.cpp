#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <util/Exception.hpp>
#include "Socket.hpp"

namespace smsc {
namespace mcisme {

int
Socket::connect()
{
  Close();

  sock=socket(AF_INET,SOCK_STREAM,0);

  if(sock==INVALID_SOCKET)
  {
    return -1;
  }

  if(connectTimeout)
    setNonBlocking(1);

  if(::connect(sock,(sockaddr*)&sockAddr,(unsigned)sizeof(sockAddr)) && errno != EINPROGRESS)
  {
    closesocket(sock);
    sock=INVALID_SOCKET;
    return -1;
  }

  if(connectTimeout)
  {
    fd_set rd,wr;
    FD_ZERO(&rd); FD_ZERO(&wr);
    FD_SET(sock,&rd); FD_SET(sock,&wr);

    timeval tv;
    tv.tv_sec=connectTimeout;
    tv.tv_usec=0;
    if(select(sock+1,&rd,&wr,0,&tv)<=0)
    {
      closesocket(sock);
      sock=INVALID_SOCKET;
      return -1;
    }
    if(FD_ISSET(sock,&wr)) {
      if (FD_ISSET(sock,&rd))
      {
        closesocket(sock);
        sock=INVALID_SOCKET;
        return -1;
      }
    } else {
      closesocket(sock);
      sock=INVALID_SOCKET;
      return -1;
    }
  }
  connected=1;

  if(connectTimeout)
    setNonBlocking(0);

  return 0;
}

void
Socket::setNonBlocking(int mode /* 0 - blocking, 1 - non blocking*/)
{
  int flags;
  if ( (flags = fcntl(sock, F_GETFL,0)) < 0 )
    throw util::SystemError("Socket::setNonBlocking::: fcntl(F_GETFL) failed");
  if ( !mode )
    flags |= O_NONBLOCK;
  else
    flags &= ~O_NONBLOCK;
  if ( fcntl(sock, F_SETFL, flags) < 0 )
    throw util::SystemError("Socket::setNonBlocking::: fcntl(F_SETFL) failed");
}

}}
