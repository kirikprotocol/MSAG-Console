
#include "Socket.hpp"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/filio.h>


#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

namespace smsc{
namespace core{
namespace network{


int Socket::Init(const char *host,int port,int timeout)
{
  hostent* lpHostEnt;
  unsigned long ulINAddr;

  timeOut=timeout;

  memset(&ulINAddr,0,sizeof(ulINAddr));
  sockAddr.sin_family=AF_INET;
  ulINAddr=inet_addr(host);
#ifndef INADDR_NONE
  if(ulINAddr!=(unsigned int)-1)
#else
  if(ulINAddr!=INADDR_NONE)
#endif
  {
    memcpy(&sockAddr.sin_addr,&ulINAddr,sizeof(ulINAddr));
  }else
  {
    lpHostEnt=gethostbyname(host);
    if(lpHostEnt==NULL)
    {
      return -1;
    }
    memcpy(&sockAddr.sin_addr,lpHostEnt->h_addr,lpHostEnt->h_length);
  }
  sockAddr.sin_port=htons(port);
  return 0;
}

int Socket::Connect()
{
  Close();
  sock=socket(AF_INET,SOCK_STREAM,0);

  if(sock==INVALID_SOCKET)
  {
    return -1;
  }
  if(connect(sock,(sockaddr*)&sockAddr,sizeof(sockAddr)))
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
  connected=0;
}

int Socket::canRead()
{
  if(!connected)return -1;
  if(bufPos<inBuffer)return 1;
  if(timeOut==0)return 1;
  FD_ZERO(&fd);
  FD_SET(sock,&fd);
  tv.tv_sec=timeOut;
  tv.tv_usec=0;
  int retval=select(FD_SETSIZE,&fd,NULL,NULL,&tv);
  return retval;
}

int Socket::canWrite()
{
  if(!connected)return -1;
  if(timeOut==0)return 1;
  FD_ZERO(&fd);
  FD_SET(sock,&fd);
  tv.tv_sec=timeOut;
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
    if(canRead()<=0)return -1;
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


int Socket::Printf(char* fmt,...)
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

int Socket::InitServer(const char *host,int port,int timeout)
{
  if(Init(host,port,timeout)==-1)return -1;
  sock=socket(AF_INET,SOCK_STREAM,0);
  if(sock==INVALID_SOCKET) return -1;
  if(bind(sock,(sockaddr*)&sockAddr,sizeof(sockAddr)))return -1;
  return 0;
}

int Socket::StartServer()
{
  if(listen(sock,SOMAXCONN))return -1;
  connected=1;
  return 0;
}

Socket* Socket::Accept()
{
  if(canRead()<=0)return NULL;
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
  ioctl(sock, FIONBIO, &mode);
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


};//network
};//core
};//smsc
