#ifndef __SunOS_5_9
#include "SCTPServerSocket.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

SCTPServerSocket::SCTPServerSocket(in_port_t port)
  : SCTPSocket(port)
{
  setReuseAddr(true);
  bind();
  listen();
}

void
SCTPServerSocket::bind()
{
  if ( ::bind(_sockfd, (struct sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) < 0 )
    throw smsc::util::SystemError("SCTPServerSocket::bind:::call to bind() failed");
}

void
SCTPServerSocket::listen()
{
  if ( ::listen(_sockfd, 10) < 0 )
    throw smsc::util::SystemError("SCTPServerSocket::listen:::call to listen() failed");
}

SCTPSocket*
SCTPServerSocket::accept()
{
  int acceptedSockFd=0;
  struct sockaddr_in addr;
  socklen_t addrLen = sizeof(addr);
  memset(&addr, 0, addrLen);

  acceptedSockFd = ::accept(_sockfd, (struct sockaddr*)&addr, &addrLen);
  if ( acceptedSockFd < 0 )
    throw smsc::util::SystemError("SCTPServerSocket::accept::: call to accept() failed");

  return new SCTPSocket(acceptedSockFd);
}

void
SCTPServerSocket::setReuseAddr(bool on)
{
  SCTPSocket::setReuseAddr(on);
}

void
SCTPServerSocket::setNonBlocking(bool on)
{
  SCTPSocket::setNonBlocking(on);
}

std::string
SCTPServerSocket::toString() const
{
  return "";
}

void
SCTPServerSocket::close()
{
  SCTPSocket::close();
}

int
SCTPServerSocket::_getDescriptor()
{
  return SCTPSocket::_getDescriptor();
}

}}}}

#endif
