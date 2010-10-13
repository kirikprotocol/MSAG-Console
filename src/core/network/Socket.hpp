#ifndef __SMSC_CORE_NETWORK_SOCKET_HPP__
#define __SMSC_CORE_NETWORK_SOCKET_HPP__

#ifdef _WIN32
#include <winsock2.h>
#include <stdio.h>
typedef int socklen_t;
#else
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#ifdef SOCKET
#undef SOCKET
#endif
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif
#include <string.h>

namespace smsc{
namespace core{
namespace network{

#define SOCKET_MAX_KEY 8

class Socket{
protected:
  sockaddr_in sockAddr;
  // fd_set fd;
  timeval tv;
  char buffer[256];
  int connected;
  int inBuffer;
  int bufPos;
  int timeOut;
  int connectTimeout;
  SOCKET sock;
  void *data[SOCKET_MAX_KEY];
public:

  Socket()
  {
    connected=0;
    inBuffer=0;
    bufPos=0;
    sock=(SOCKET)-1;
    connectTimeout=0;
    memset(data,0,sizeof(data));
  }
  Socket(SOCKET s,const sockaddr_in& saddrin )
  {
    connected=1;
    inBuffer=0;
    bufPos=0;
    timeOut=60;
    sock=s;
    sockAddr=saddrin;
    connectTimeout=0;
    memset(data,0,sizeof(data));
  }

  virtual ~Socket()
  {
    Close();
  }

    bool isConnected() const { return connected; }

  void setConnectTimeout(int to)
  {
    connectTimeout=to;
  }

  SOCKET getSocket(){return sock;}

  int Init(const char *host,int port,int timeout);

  int BindClient(const char* host)
  {
    sockaddr_in sAddr;
    hostent* lpHostEnt;
    in_addr_t ulINAddr;

    memset(&ulINAddr,0,sizeof(ulINAddr));
    sAddr.sin_family=AF_INET;
    ulINAddr=inet_addr(host);
  #ifndef INADDR_NONE
    if(ulINAddr!=-1)
  #else
    if(ulINAddr!=INADDR_NONE)
  #endif
    {
      memcpy(&sAddr.sin_addr,&ulINAddr,sizeof(ulINAddr));
    }else
    {
  #ifndef _REENTRANT
      lpHostEnt=gethostbyname(host);
  #else
      char buf[1024];
      int h_err;
      hostent he;
#ifdef __GNUC__
      gethostbyname_r( host, &he, buf, sizeof(buf), &lpHostEnt, &h_err );
#else
      lpHostEnt=gethostbyname_r(host, &he, buf, (int)sizeof(buf), &h_err);
#endif // __GNUC__
  #endif
      if(lpHostEnt==NULL)
      {
        return -1;
      }
      memcpy(&sAddr.sin_addr,lpHostEnt->h_addr,lpHostEnt->h_length);
    }
    sAddr.sin_port=0;
    if(bind(sock,(sockaddr*)&sAddr,(int)sizeof(sAddr)))return -1;
    return 0;
  }
  int Connect(bool nb = false);
  int ConnectEx(bool nb,const char* bindHost);
  void Close();
  void Abort();
  void ReuseAddr()
  {
    int one=1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, (int)sizeof(int));
  }
  int canRead(int to=0);
  int canWrite(int to=0);
  int Read(char *buf,int bufsize);
  int ReadAll(char* buf,int size,int to=0);
  int readChar();
  int Write(const char *buf,int bufsize);
  int WriteAll(const void *buf,int bufsize);
  int Gets(char *buf, int len);
  void Done();
  int Printf(const char* fmt,...);
  int Puts(const char* str);
  int InitServer(const char *host,int port,int timeout,int lng=1,bool force=false);
  int StartServer();
  Socket* Accept(int to=0);
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
  void GetPeer(char* buf) const
  {
    sockaddr_in addr;
    socklen_t len=sizeof(addr);
    if(getpeername(sock,(sockaddr*)&addr,&len)==SOCKET_ERROR)
    {
      buf[0]=0;
      return;
    }
    PrintAddrPort(addr,buf);
  }

  static void PrintAddrPort( const sockaddr_in& addr, char* buf )
  {
#ifndef __sun
    unsigned char *a=(unsigned char*)&addr.sin_addr.s_addr;
#else
    unsigned char *a=(unsigned char*)&addr.sin_addr.S_un.S_addr;
#endif
    unsigned char *p=(unsigned char*)&addr.sin_port;
    sprintf(buf,"%d.%d.%d.%d:%d",(int)a[0],(int)a[1],(int)a[2],(int)a[3],unsigned(p[0])*256 + p[1]);
  }

  void SetNoDelay(bool nd)
  {
    char b=nd?1:0;
    setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&b,1);
  }


};//Socket

}//network
}//core
}//smsc

#endif
