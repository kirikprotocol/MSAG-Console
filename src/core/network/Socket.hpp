#ifndef __SMSC_CORE_NETWORK_SOCKET_HPP__
#define __SMSC_CORE_NETWORK_SOCKET_HPP__

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif

namespace smsc{
namespace core{
namespace network{

class Socket{
private:
  sockaddr_in sockAddr;
  fd_set fd;
  timeval tv;
  int connected;
  char buffer[256];
  int inBuffer;
  int bufPos;
  SOCKET sock;
  int timeOut;
public:

  Socket()
  {
    connected=0;
    inBuffer=0;
    bufPos=0;
  }
  Socket(SOCKET s,const sockaddr_in& saddrin )
  {
    connected=1;
    inBuffer=0;
    bufPos=0;
    timeOut=60;
    sock=s;
    sockAddr=saddrin;
  }

  ~Socket()
  {
    Close();
  }

  SOCKET getSocket(){return sock;}

  int Init(char *host,int port,int timeout);
  int Connect();
  void Close();
  int canRead();
  int canWrite();
  int Read(char *buf,int bufsize);
  int readChar();
  int Write(const char *buf,int bufsize);
  int Gets(char *buf, int len);
  void Done();
  int Printf(char* fmt,...);
  int Puts(const char* str);
  int InitServer(char *host,int port,int timeout);
  int StartServer();
  Socket* Accept();
};//Socket

};//network
};//core
};//smsc

#endif
