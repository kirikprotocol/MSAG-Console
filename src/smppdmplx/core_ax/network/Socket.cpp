#include "Socket.hpp"
#include <util/Exception.hpp>
#include <smppdmplx/NetworkException.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#include <logger/Logger.h>

namespace smsc {
namespace core_ax {
namespace network {

class Socket_impl {
public:
  Socket_impl(const std::string host, in_port_t port);
  Socket_impl(int sockFd, struct sockaddr_in peerAddr);
  Socket_impl(in_port_t listeningPort);
  Socket_impl();
  ~Socket_impl();

  Socket accept() const;
  void connect();
  void close();

  void setReuseAddr();
  void setNonBlocking();
  void setBlocking();

  int getSocketDescriptor() const;

  bool isReadable() const;
  bool isWriteable() const;
  void markAsReadable();
  void markAsWriteable();

  void decrementRefCount();
  void incrementRefCount();
  int getRefCount() const;
  bool isListeningSocket() const;
  std::string toString() const;
private:
  int _sockfd;
  bool  _connected;

  bool _isReadble, _isWriteable;
  int _refCount;
  struct sockaddr_in _peerAddr;
  struct sockaddr_in _listenAddr;
  bool _isListeningSocket;

  friend class Socket;
  friend class ServerSocket;
};

}
}
}

smsc::core_ax::network::Socket::Socket(int sockFd, struct sockaddr_in peerAddr)
{
  if ( !(_impl = new Socket_impl(sockFd, peerAddr)) )
    throw smsc::util::SystemError("core_ax::network::Socket::Socket::: memory allocation was failed");
}

smsc::core_ax::network::Socket::Socket(const std::string peerHost, in_port_t peerPort)
{
  if ( !(_impl = new Socket_impl(peerHost, peerPort)) )
    throw smsc::util::SystemError("core_ax::network::Socket::Socket::: memory allocation was failed");
}

smsc::core_ax::network::Socket::Socket()
{
  if ( !(_impl = new Socket_impl()) )
    throw smsc::util::SystemError("core_ax::network::Socket::Socket::: memory allocation was failed");
}

smsc::core_ax::network::Socket::Socket(const Socket& rhs)
{
  _impl = rhs._impl;
  _impl->incrementRefCount();
}

smsc::core_ax::network::Socket&
smsc::core_ax::network::Socket::operator=(const Socket& rhs)
{
  if ( this != &rhs ) {
    _impl->decrementRefCount();
    if ( _impl->getRefCount() == 0 )
      delete _impl;

    _impl = rhs._impl;
    _impl->incrementRefCount();
  }

  return *this;
}

smsc::core_ax::network::Socket::Socket(ServerSocket& rhs)
{
  _impl = rhs._impl;
  _impl->incrementRefCount();
}

smsc::core_ax::network::Socket::~Socket()
{
  _impl->decrementRefCount();
  if ( _impl->getRefCount() == 0 )
    delete _impl;
}

void
smsc::core_ax::network::Socket::connect()
{
  _impl->connect();
}

void
smsc::core_ax::network::Socket::close()
{
  _impl->close();
}

void
smsc::core_ax::network::Socket::setReuseAddr()
{
  _impl->setReuseAddr();
}

void
smsc::core_ax::network::Socket::setNonBlocking()
{
  _impl->setNonBlocking();
}

void
smsc::core_ax::network::Socket::setBlocking()
{
  _impl->setBlocking();
}

int
smsc::core_ax::network::Socket::getSocketDescriptor() const
{
  return _impl->getSocketDescriptor();
}

bool
smsc::core_ax::network::Socket::isReadable() const
{
  return _impl->isReadable();
}

bool
smsc::core_ax::network::Socket::isWriteable() const
{
  return _impl->isWriteable();
}

void
smsc::core_ax::network::Socket::markAsReadable()
{
  _impl->markAsReadable();
}

void
smsc::core_ax::network::Socket::markAsWriteable()
{
  _impl->markAsWriteable();
}

bool
smsc::core_ax::network::Socket::operator==(const ServerSocket& rhs) const
{
  return _impl == rhs._impl;
}

bool
smsc::core_ax::network::Socket::operator==(const Socket& rhs) const
{
  return _impl == rhs._impl;
}

const std::string
smsc::core_ax::network::Socket::toString() const
{
  return _impl->toString();
}

//----------------------ServerSocket implementation--------------------------

smsc::core_ax::network::ServerSocket::ServerSocket() {
  if ( !(_impl = new Socket_impl()) )
    throw smsc::util::SystemError("core_ax::network::ServerSocket::ServerSocket::: memory allocation was failed");
}

smsc::core_ax::network::ServerSocket::ServerSocket(in_port_t listeningPort)
{
  if ( !(_impl = new Socket_impl(listeningPort)) )
    throw smsc::util::SystemError("core_ax::network::ServerSocket::ServerSocket::: memory allocation was failed");
}

smsc::core_ax::network::ServerSocket::ServerSocket(Socket& readySocket)
{
  if ( readySocket._impl->isListeningSocket() ) {
    _impl = readySocket._impl;
    _impl->incrementRefCount();
  } else 
    throw smsc::util::SystemError("core_ax::network::ServerSocket::ServerSocket(Socket&)::: argument is not listening ServerSocket");
}

smsc::core_ax::network::ServerSocket::~ServerSocket()
{
  _impl->decrementRefCount();
  if ( _impl->getRefCount() == 0 )
    delete _impl;
}

smsc::core_ax::network::Socket
smsc::core_ax::network::ServerSocket::accept() const
{
  return _impl->accept();
}

void smsc::core_ax::network::ServerSocket::setNonBlocking()
{
  _impl->setNonBlocking();
}

void smsc::core_ax::network::ServerSocket::setBlocking()
{
  _impl->setBlocking();
}

int
smsc::core_ax::network::ServerSocket::getSocketDescriptor() const
{
  return _impl->getSocketDescriptor();
}

bool
smsc::core_ax::network::ServerSocket::operator==(const ServerSocket& rhs) const
{
  return _impl == rhs._impl;
}

bool
smsc::core_ax::network::ServerSocket::operator==(const Socket& rhs) const
{
  return _impl == rhs._impl;
}

void
smsc::core_ax::network::ServerSocket::close()
{
  _impl->close();
}

std::string
smsc::core_ax::network::ServerSocket::toString() const
{
  return _impl->toString();
}

//----------------------Socket_impl implementation---------------------------
smsc::core_ax::network::Socket_impl::Socket_impl(const std::string peerHost, in_port_t peerPort) : _sockfd(-1), _isReadble(false), _isWriteable(false), _connected(false), _refCount(1), _isListeningSocket(false)
{
  if ( (_sockfd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    throw smsc::util::SystemError("core_ax::network::Socket_impl::Socket_impl::: socket failed");

  memset(&_peerAddr, 0, sizeof(_peerAddr));
  _peerAddr.sin_family = AF_INET;
  _peerAddr.sin_port = htons(peerPort);
  int st = inet_pton(AF_INET, peerHost.c_str(), &_peerAddr.sin_addr);
  if ( st < 0 ) 
    throw smsc::util::SystemError("core_ax::network::Socket_impl::Socket_impl::: socket failed");

  if ( !st ) {
    struct hostent* hp;
    hp = gethostbyname(peerHost.c_str());
    if ( hp )
      memcpy(&_peerAddr.sin_addr, hp->h_addr, hp->h_length);
    else
      throw smsc::util::SystemError("core_ax::network::Socket_impl::Socket_impl::: can't find host");
  }
}

smsc::core_ax::network::Socket_impl::Socket_impl() : _sockfd(-1), _isReadble(false), _isWriteable(false), _connected(false), _refCount(1), _isListeningSocket(false) {}

smsc::core_ax::network::Socket_impl::Socket_impl(int sockFd, struct sockaddr_in peerAddr): _sockfd(sockFd), _peerAddr(peerAddr), _isReadble(false), _isWriteable(false), _connected(true), _refCount(1), _isListeningSocket(false) {}

smsc::core_ax::network::Socket_impl::~Socket_impl()
{
  close();
}

void
smsc::core_ax::network::Socket_impl::connect()
{
  if ( _sockfd > -1 ) {
    if ( _connected == false ) {
      if( ::connect(_sockfd,(sockaddr*)&_peerAddr,sizeof(_peerAddr)) < 0 )
      {
	throw smpp_dmplx::NetworkException("core_ax::network::Socket_impl::connect::: connect falied");
      }
      _connected = true;
    }
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::connect::: _sockfd < 0");
}

smsc::core_ax::network::Socket
smsc::core_ax::network::Socket_impl::accept() const
{
  if ( _sockfd > -1 ) {
    struct sockaddr_in  peerAddr;
    socklen_t addrLen = sizeof(peerAddr);
    int acceptedSockFd;

    if ( (acceptedSockFd=::accept(_sockfd, (sockaddr*)&peerAddr, &addrLen)) < 0 )
      throw smpp_dmplx::NetworkException("core_ax::network::Socket::accept::: accept falied");

    return Socket(acceptedSockFd, peerAddr);
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::accept::: _sockfd < 0");
}

smsc::core_ax::network::Socket_impl::Socket_impl(in_port_t listeningPort) : _sockfd(-1), _isReadble(false), _isWriteable(false), _connected(false), _refCount(1), _isListeningSocket(true)
{
  if ( (_sockfd = ::socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    throw smsc::util::SystemError("core_ax::network::Socket_impl::Socket_impl::: socket falied");

  setReuseAddr();

  memset(&_listenAddr, 0, sizeof(_listenAddr));
  _listenAddr.sin_family = AF_INET;
  _listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  _listenAddr.sin_port = htons(listeningPort);

  if (::bind(_sockfd, (const struct sockaddr *)&_listenAddr, sizeof(_listenAddr)) < 0 )
    throw smsc::util::SystemError("core_ax::network::Socket_impl::rSocket_impl: bind falied");

  if (::listen(_sockfd,SOMAXCONN) < 0)
    throw smsc::util::SystemError("core_ax::network::Socket_impl::Socket_impl::: listen falied");

  _connected = true;
}

void
smsc::core_ax::network::Socket_impl::close()
{
  if ( _sockfd > -1 ) {
    ::close(_sockfd);
    _sockfd = -1;
  }
}

void
smsc::core_ax::network::Socket_impl::setReuseAddr()
{
  if ( _sockfd > -1 ) {
    int on = 1;
    if ( setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0 )
      throw smsc::util::SystemError("core_ax::network::Socket_impl::setReuseAddr::: setsockopt falied");
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::setReuseAddr::: _sockfd < 0");
}

void
smsc::core_ax::network::Socket_impl::setNonBlocking()
{
  if ( _sockfd > -1 ) {
    int flags;
    if ( (flags = ::fcntl(_sockfd, F_GETFL, 0)) < 0 ||
	 ::fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK) < 0 )
      throw smsc::util::SystemError("core_ax::network::Socket_impl::setNonBlocking::: fcntl falied");
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::setNonBlocking::: _sockfd < 0");
}

void
smsc::core_ax::network::Socket_impl::setBlocking()
{
  if ( _sockfd > -1 ) {
    int flags;
    if ( (flags = ::fcntl(_sockfd, F_GETFL, 0)) ||
	 ::fcntl(_sockfd, F_SETFL, flags & ~O_NONBLOCK) < 0 )
      throw smsc::util::SystemError("core_ax::network::Socket_impl::setBlocking::: fcntl falied");
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::setBlocking::: _sockfd < 0");
}

int
smsc::core_ax::network::Socket_impl::getSocketDescriptor() const
{
  if ( _sockfd > -1 ) {
    if ( _connected == true )
      return _sockfd;
    else
      throw smsc::util::Exception("core_ax::network::Socket_impl::getSocketDescriptor::: socket is not connected");
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::getSocketDescriptor::: _sockfd < 0");
}

bool
smsc::core_ax::network::Socket_impl::isReadable() const
{
  if ( _sockfd > -1 ) {
    if ( _connected == true )
      return _isReadble;
    else
      throw smsc::util::Exception("core_ax::network::Socket_impl::isReadable::: socket is not connected");
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::isReadable::: _sockfd < 0");
}

bool
smsc::core_ax::network::Socket_impl::isWriteable() const
{
  if ( _sockfd > -1 ) {
    if ( _connected == true )
      return _isWriteable;
    else
      throw smsc::util::Exception("core_ax::network::Socket_impl::isWriteable::: socket is not connected");
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::isWriteable::: _sockfd < 0");
}

void smsc::core_ax::network::Socket_impl::markAsReadable()
{
  if ( _sockfd > -1 ) {
    if ( _connected == true )
      _isReadble = true;
    else
      throw smsc::util::Exception("core_ax::network::Socket_impl::markAsReadable::: socket is not connected");
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::markAsReadable::: _sockfd < 0");
}

void smsc::core_ax::network::Socket_impl::markAsWriteable()
{
  if ( _sockfd > -1 ) {
    if ( _connected == true )
      _isWriteable = true;
    else
      throw smsc::util::Exception("core_ax::network::Socket_impl::markAsWriteable::: socket is not connected");
  } else
    throw smsc::util::Exception("core_ax::network::Socket_impl::markAsWriteable::: _sockfd < 0");
}

void smsc::core_ax::network::Socket_impl::decrementRefCount()
{
  --_refCount;
}

void smsc::core_ax::network::Socket_impl::incrementRefCount()
{
  ++_refCount;
}

int smsc::core_ax::network::Socket_impl::getRefCount() const
{
  return _refCount;
}

bool smsc::core_ax::network::Socket_impl::isListeningSocket() const
{
  return _isListeningSocket;
}

bool
smsc::core_ax::network::Socket::operator<(const smsc::core_ax::network::Socket& rhs) const
{
  return _impl < rhs._impl;
}

std::string
smsc::core_ax::network::Socket_impl::toString() const
{
  union socket_inet_addr {
    struct in_addr junk;
    uint8_t inet_addr[4];
  } socket_inet_addr;

  in_port_t socketPort;
  if ( isListeningSocket() ) {
    socket_inet_addr.junk = _listenAddr.sin_addr;
    socketPort = ntohs(_listenAddr.sin_port);
  } else {
    socket_inet_addr.junk = _peerAddr.sin_addr;
    socketPort = ntohs(_peerAddr.sin_port);
  }
  
  char socket_dump[48];
  snprintf(socket_dump, sizeof(socket_dump), "%d.%d.%d.%d.%d, fd=%d", socket_inet_addr.inet_addr[0], socket_inet_addr.inet_addr[1], socket_inet_addr.inet_addr[2], socket_inet_addr.inet_addr[3], socketPort, _sockfd);

  return std::string(socket_dump);
}
