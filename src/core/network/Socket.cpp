
#include "Socket.hpp"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#define EINPROGRESS WSAEINPROGRESS
#else
#include <sys/ioctl.h>
#include <errno.h>
#ifndef linux
#include <sys/filio.h>
#endif
#endif

namespace smsc{
namespace core{
namespace network{


int Socket::Init(const char *host,int port,int timeout)
{
  hostent* lpHostEnt;
  in_addr_t ulINAddr;

  timeOut=timeout;

  memset(&ulINAddr,0,sizeof(ulINAddr));
  sockAddr.sin_family=AF_INET;
  ulINAddr=inet_addr(host);
#ifndef INADDR_NONE
  if(ulINAddr!=-1)
#else
  if(ulINAddr!=INADDR_NONE)
#endif
  {
    memcpy(&sockAddr.sin_addr,&ulINAddr,sizeof(ulINAddr));
  }else
  {
#ifndef _REENTRANT
    lpHostEnt=gethostbyname(host);
#else
    char buf[1024];
    int h_err;
    hostent he;
    lpHostEnt=gethostbyname_r(host, &he, buf, sizeof(buf), &h_err);
#endif
    if(lpHostEnt==NULL)
    {
      return -1;
    }
    memcpy(&sockAddr.sin_addr,lpHostEnt->h_addr,lpHostEnt->h_length);
  }
  sockAddr.sin_port=htons(port);
  return 0;
}

int Socket::Connect(bool nb)
{
  Close();
  sock=socket(AF_INET,SOCK_STREAM,0);

  if(sock==INVALID_SOCKET)
  {
    return -1;
  }

  if(nb) setNonBlocking(1);

  if(connect(sock,(sockaddr*)&sockAddr,sizeof(sockAddr)) && errno != EINPROGRESS)
  {
    closesocket(sock);
    sock=INVALID_SOCKET;
    return -1;
  }
  //linger l;
  //l.l_onoff=1;
  //l.l_linger=0;
  //setsockopt(sock,SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(l));

  connected=1;
  return 0;
}

int Socket::ConnectEx(bool nb,const char* bindHost)
{
  Close();
  sock=socket(AF_INET,SOCK_STREAM,0);

  if(sock==INVALID_SOCKET)
  {
    return -1;
  }

  if(nb) setNonBlocking(1);
  if(bindHost)
  {
    if(BindClient(bindHost)==-1)return -1;
  }

  if(connect(sock,(sockaddr*)&sockAddr,sizeof(sockAddr)) && errno != EINPROGRESS)
  {
    closesocket(sock);
    sock=INVALID_SOCKET;
    return -1;
  }
  //linger l;
  //l.l_onoff=1;
  //l.l_linger=0;
  //setsockopt(sock,SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(l));

  connected=1;
  return 0;
}

void Socket::Close()
{
  if(!connected)
  {
    if(sock!=INVALID_SOCKET)
    {
      closesocket(sock);
      sock=INVALID_SOCKET;
    }
    return;
  }
  shutdown(sock,2);
  closesocket(sock);
  sock=INVALID_SOCKET;
  connected=0;
}

void Socket::Abort()
{
  if(sock!=INVALID_SOCKET)
  {
    linger l;
    l.l_onoff=1;
    l.l_linger=0;
    setsockopt(sock,SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(l));
  }
  Close();
}

int Socket::canRead(int to)
{
  if(!connected || sock==INVALID_SOCKET)return -1;
  if(bufPos<inBuffer)return 1;
  if(to==0)to=timeOut;
  if(to==0)return 1;

  FD_ZERO(&fd);
  FD_SET(sock,&fd);
  tv.tv_sec=to;
  tv.tv_usec=0;
  int retval=select(FD_SETSIZE,&fd,NULL,NULL,&tv);
  return retval;
}

int Socket::canWrite(int to)
{
  if(!connected || sock==INVALID_SOCKET) return -1;
  if(to==0)to=timeOut;
  if(to==0)return 1;
  FD_ZERO(&fd);
  FD_SET(sock,&fd);
  tv.tv_sec=to;
  tv.tv_usec=0;
  return select(FD_SETSIZE,NULL,&fd,NULL,&tv);
}


int Socket::Read(char *buf,int bufsize)
{
  if(!connected)return -1;
  //int retval;
  if(bufPos<inBuffer)
  {
    int n=inBuffer-bufPos;
    if(n>bufsize)n=bufsize;
    memcpy(buf,buffer+bufPos,n);
    bufPos+=n;
    return n;
  }
  return recv(sock,buf,bufsize,0);
}

int Socket::readChar()
{
  if(inBuffer<=0 || bufPos==inBuffer)
  {
    if(canRead()<=0) return -1;
    inBuffer=Read(buffer,sizeof(buffer));
    bufPos=0;
  }
  if(inBuffer<=0)return -1;
  return (unsigned char)(buffer[bufPos++]);
}

int Socket::Write(const char *buf,int bufsize)
{
  if(!connected)return -1;
  return send(sock,buf,bufsize,0);
}


int Socket::Gets(char *buf, int len)
{
  int ch;
  int i = 0;

  while (--len)
  {
    ch=readChar();
    if(ch==-1)return SOCKET_ERROR;
    buf[i] = ch;
    i++;
    if(i>1 && (buf[i-2]==0x0d && buf[i-1]==0x0a) || (buf[i-1]==0x0a))break;
  }
  buf[i] = 0;

  return i;
}


int Socket::Printf(const char* fmt,...)
{
  char buf_init[4096];
  char *buf=buf_init;
  int bufsize=sizeof(buf_init);
  va_list args;
  va_start (args, fmt);
  int res;
  do{
#ifndef _WIN32
    res=vsnprintf (buf,bufsize,fmt, args);
#else
    res=_vsnprintf (buf,bufsize,fmt, args);
#endif

    if(res<0)
    {
      bufsize-=res;
      bufsize+=16;
      buf=new char[bufsize];
    }
  }while(res<0);
  int ret=Write(buf,strlen(buf));
  if(buf!=buf_init)delete [] buf;
  va_end (args);
  return ret;
}

int Socket::Puts(const char* str)
{
  return Write((char*)str,strlen(str));
}

int Socket::InitServer(const char *host,int port,int timeout,int lng,bool force)
{
  if(Init(host,port,timeout)==-1)return -1;
  sock=socket(AF_INET,SOCK_STREAM,0);
  if(sock==INVALID_SOCKET) return -1;
  if(force)
  {
    int val=1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&val,4);
  }
  if(bind(sock,(sockaddr*)&sockAddr,sizeof(sockAddr)))
  {
    closesocket(sock);
    return -1;
  }
  linger l;
  l.l_onoff=1;
  l.l_linger=lng;
  setsockopt(sock,SOL_SOCKET,SO_LINGER,(char*)&l,sizeof(l));
  return 0;
}

int Socket::StartServer()
{
  if(listen(sock,SOMAXCONN))
  {
    closesocket(sock);
    sock=0;
    return -1;
  }
  connected=1;
  return 0;
}

Socket* Socket::Accept(int to)
{
  if(canRead(to)<=0)return NULL;
  sockaddr_in addrin;
  int sz=sizeof(addrin);
#ifdef _WIN32
  SOCKET s=accept(sock,(sockaddr*)&addrin,&sz);
#else
  SOCKET s=accept(sock,(sockaddr*)&addrin,(socklen_t*)&sz);
#endif
  if(s==-1)return NULL;
  return new Socket(s,addrin);
}

void Socket::setNonBlocking(int mode)
{
#ifdef _WIN32
  ioctlsocket(sock, FIONBIO, (unsigned long*)&mode);
#else
  ioctl(sock, FIONBIO, &mode);
#endif
}

int Socket::ReadAll(char* buf,int size)
{
  int rd=0,res;
  while(rd<size)
  {
    res=Read(buf+rd,size-rd);
    if(res<=0)
    {
      return -1;
    }
    rd+=res;
  }
  return size;
}

int Socket::WriteAll(const void *buf,int bufsize)
{
  if(bufsize==0)return 0;
  int wr,count=0;
  const char *_buf=(const char*)buf;
  do{
    wr=Write(_buf+count,bufsize-count>1024?1024:bufsize-count);
    if(wr<=0)return -1;
    count+=wr;
  }while(count!=bufsize);
  return count;
}


#ifdef _WIN32

#define REQWINSOCK_VERSION 0x0101

class __SockInit{
public:
WSADATA wsaData;
__SockInit()
{
  WSAStartup(REQWINSOCK_VERSION, &wsaData);
}
~__SockInit()
{
  WSACleanup();
}
};
static __SockInit Dummy;
#endif


}//network
}//core
}//smsc
