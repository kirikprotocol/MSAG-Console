#ifndef __SMSC_CORE_NETWORK_SOCKET_HPP__
#define __SMSC_CORE_NETWORK_SOCKET_HPP__

#ifdef _WIN32
#include <winsock.h>
typedef int socklen_t;
#else
#include <stdio.h>
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

#define SOCKET_MAX_KEY 8

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
  void *data[SOCKET_MAX_KEY];
public:

  Socket()
  {
    connected=0;
    inBuffer=0;
    bufPos=0;
    sock=-1;
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

  virtual ~Socket()
  {
    Close();
  }

  SOCKET getSocket(){return sock;}

  int Init(const char *host,int port,int timeout);
  int Connect();
  void Close();
  void Abort();
  void ReuseAddr()
  {
    int one=1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof(int));
  }
  int canRead(int to=0);
  int canWrite(int to=0);
  int Read(char *buf,int bufsize);
  int ReadAll(char* buf,int size);
  int readChar();
  int Write(const char *buf,int bufsize);
  int WriteAll(const void *buf,int bufsize);
  int Gets(char *buf, int len);
  void Done();
  int Printf(char* fmt,...);
  int Puts(const char* str);
  int InitServer(const char *host,int port,int timeout,int lng=1);
  int StartServer();
  Socket* Accept();
  Socket* Clone()
  {
    return new Socket(sock,sockAddr);
  }
  void setNonBlocking(int mode);
  void setData(unsigned int key,void* newdata)
  {
    if(key>=SOCKET_MAX_KEY)return;
    data[key]=newdata;
  }
  void* getData(unsigned int key)
  {
    if(key>=SOCKET_MAX_KEY)return NULL;
    return data[key];
  }
  int setTimeOut(int newto)
  {
    int old=timeOut;
    timeOut=newto;
    return old;
  }
  void GetPeer(char* buf)
  {
    sockaddr_in addr;
    socklen_t len=sizeof(addr);
    if(getpeername(sock,(sockaddr*)&addr,&len)==SOCKET_ERROR)
    {
      buf[0]=0;
    }
    unsigned char *a=(unsigned char*)&addr.sin_addr.S_un.S_addr;
    sprintf(buf,"%d.%d.%d.%d:%d",(int)a[0],(int)a[1],(int)a[2],(int)a[3],addr.sin_port);
  }


};//Socket

}//network
}//core
}//smsc

#endif
