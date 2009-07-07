#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "util/Exception.hpp"
#include "TCPServerSocket.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

TCPServerSocket::TCPServerSocket(const std::string& host, in_port_t port)
  : TCPSocket(host, port) {
  setReuseAddr(true);
  bind();
  listen();
  fillToStringInfo(false);
}

TCPServerSocket::TCPServerSocket(in_port_t port)
  : TCPSocket(port) {
  setReuseAddr(true);
  bind();
  listen();
  fillToStringInfo(false);
}

void
TCPServerSocket::setReuseAddr(bool on)
{
  TCPSocket::setReuseAddr(on);
}

void
TCPServerSocket::setNonBlocking(bool on)
{
  TCPSocket::setNonBlocking(on);
}

void
TCPServerSocket::bind()
{
  if ( ::bind(_sockfd, (struct sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) < 0 )
    throw smsc::util::SystemError("TCPServerSocket::bind:::call to bind() failed");
}

void
TCPServerSocket::listen()
{
  if ( ::listen(_sockfd, 10) < 0 )
    throw smsc::util::SystemError("TCPServerSocket::listen:::call to listen() failed");
}

TCPSocket*
TCPServerSocket::accept()
{
  int acceptedSockFd=0;
  struct sockaddr_in addr;
  socklen_t addrLen = sizeof(addr);
  memset(&addr, 0, addrLen);
  if ( (acceptedSockFd = ::accept(_sockfd, (struct sockaddr*)&addr, &addrLen)) < 0 )
    throw smsc::util::SystemError("TCPServerSocket::accept:::call to accept() failed");

  return new TCPSocket(acceptedSockFd);
}

std::string
TCPServerSocket::toString() const
{
  if ( !_toString_buf_init ) {
    snprintf(_toString_buf, sizeof(_toString_buf), "%s.%d", _l_ip_address, _l_port);
    _toString_buf_init = true;
  }
  return std::string(_toString_buf);
}

void
TCPServerSocket::close()
{
  TCPSocket::close();
}

int
TCPServerSocket::_getDescriptor()
{
  return TCPSocket::_getDescriptor();
}

}}}}
