#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "util/Exception.hpp"
#include "eyeline/corex/io/network/TCPSocket.hpp"
#include "eyeline/corex/io/IOExceptions.hpp"
#include "logger/Logger.h"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

void
TCPSocket::openSocket()
{
  _sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if ( _sockfd < 0 ) throw smsc::util::SystemError("TCPSocket::openSocket::: call to socket() failed");
}

TCPSocket::TCPSocket(in_port_t port)
  : _toString_buf_init(false), _inputStream(NULL), _outputStream(NULL),
    _reverseSourceDestinationAddresses(false)
{
  memset(_l_ip_address, 0, sizeof(_l_ip_address));
  memset(_r_ip_address, 0, sizeof(_r_ip_address));
  _l_port = port; _r_port = 0;

  openSocket();

  memset((uint8_t*)&_server_addr, 0, sizeof(_server_addr));
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
  _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  inet_ntop(AF_INET, &_server_addr.sin_addr, _l_ip_address, static_cast<int>(sizeof(_l_ip_address)));
}

TCPSocket::TCPSocket(const std::string& host, in_port_t port)
  : _toString_buf_init(false), _inputStream(NULL), _outputStream(NULL),
    _reverseSourceDestinationAddresses(false)
{
  memset(_l_ip_address, 0, sizeof(_l_ip_address));
  memset(_r_ip_address, 0, sizeof(_r_ip_address));
  _l_port = 0; _r_port = 0;

  openSocket();

  _host = host;
  memset((uint8_t*)&_server_addr, 0, sizeof(_server_addr));
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
}

void
TCPSocket::fillToStringInfo(bool needPeerAddr)
{
  sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);
  if ( getsockname(_sockfd, (sockaddr*)&addr, &addrlen) < 0 )
    throw smsc::util::SystemError("TCPSocket::getsockname::: call to getsockname() failed");
  inet_ntop(AF_INET, &addr.sin_addr, _l_ip_address, static_cast<int>(sizeof(_l_ip_address)));
  _l_port = ntohs(addr.sin_port);

  if ( needPeerAddr ) {
    if ( getpeername(_sockfd, (sockaddr*)&addr, &addrlen) < 0 )
      throw smsc::util::SystemError("TCPSocket::getpeername::: call to getpeername() failed");
    inet_ntop(AF_INET, &addr.sin_addr, _r_ip_address, static_cast<int>(sizeof(_r_ip_address)));
    _r_port = ntohs(addr.sin_port);
  }
}

TCPSocket::TCPSocket(int sockfd)
  : _sockfd(sockfd), _toString_buf_init(false)
{
  if ( sockfd < 0 )
    throw smsc::util::SystemError("TCPSocket::TCPSocket::: input socket fd value is less than 0");

  _inputStream = new GenericInputStream(this, sockfd);
  _outputStream = new GenericOutputStream(this, sockfd);

  _reverseSourceDestinationAddresses = true;
  fillToStringInfo();
}

TCPSocket::~TCPSocket() {
  delete _inputStream; delete _outputStream;
  close();
}

void TCPSocket::connect(unsigned connect_timeout)
{
  if ( inet_pton(AF_INET, _host.c_str(), &_server_addr.sin_addr) < 1 ) {
    struct hostent *hp;
    struct hostent h_result;
    char addr_list_buffer[1024];
    int h_errno_buf;

#ifdef _REENTRANT
    if ( !(hp = gethostbyname_r(_host.c_str(), &h_result, addr_list_buffer, static_cast<int>(sizeof(addr_list_buffer)), &h_errno_buf)) )
      throw smsc::util::SystemError("TCPSocket::connect::: call to gethostbyname() failed", h_errno_buf);
#else
    if ( !(hp = gethostbyname(_host.c_str())) )
      throw smsc::util::SystemError("TCPSocket::connect::: call to gethostbyname() failed", h_errno);
#endif

    if ( hp->h_addrtype != AF_INET )
      throw smsc::util::Exception("TCPSocket::connect::: gethostbyname() returned unsupported addrtype [%d]", hp->h_addrtype);

    bool connRefused = false;
    for(char** pptr = hp->h_addr_list; *pptr; ++pptr) {
      memcpy(&_server_addr.sin_addr, *pptr, sizeof(_server_addr.sin_addr));
      if (connect_timeout)  {
        if ( tryConnectWithTimeout(connect_timeout) == 0 ) {
          fillToStringInfo();
          _inputStream = new GenericInputStream(this, _sockfd); _outputStream = new GenericOutputStream(this, _sockfd);
          return;
        }
        if ( errno == ETIMEDOUT ) {
          close(); // interrupt connection establishing attempts
          openSocket();
        } else
          connRefused = true;
      } else {
        if ( ::connect(_sockfd, (sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) == 0 ) {
          fillToStringInfo();
          _inputStream = new GenericInputStream(this, _sockfd); _outputStream = new GenericOutputStream(this, _sockfd);
          return;
        }
        if ( errno == ECONNREFUSED )
          connRefused = true;
      }
    }
    if ( connRefused)
      throw ConnectionFailedException("TCPSocket::connect::: connection refused");
    else
      throw smsc::util::Exception("TCPSocket::connect::: can't establish connect");
  } else {
    if (connect_timeout) {
      if ( tryConnectWithTimeout(connect_timeout) < 0 ) {
        if ( errno == ETIMEDOUT ) {
          close();
          throw ConnectionTimedoutException("TCPSocket::connect::: connection attempt timed out");
        } else
          throw ConnectionFailedException("TCPSocket::connect::: connection refused: errno = %d", errno);
      }
    } else {
      if ( ::connect(_sockfd, (sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) < 0 ) {
        if ( errno == ECONNREFUSED )
          throw ConnectionFailedException("TCPSocket::connect::: connection refused");
        else
          throw smsc::util::SystemError("TCPSocket::connect::: call to connect() failed");
      }
    }
    fillToStringInfo();
    _inputStream = new GenericInputStream(this, _sockfd); _outputStream = new GenericOutputStream(this, _sockfd);
  }
}

int
TCPSocket::tryConnectWithTimeout(int connect_timeout)
{
  setNonBlocking(_sockfd, true);
  errno=0;
  if ( ::connect(_sockfd, (sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) < 0 ) {
    if ( errno == ECONNREFUSED )
      throw ConnectionFailedException("TCPSocket::connect::: connection (connect_timeout=%d) refused",
                                      connect_timeout);
    else if ( errno != EINPROGRESS ) {
      setNonBlocking(_sockfd, false);
      char errMsg[128];
      sprintf(errMsg, "TCPSocket::tryConnectWithTimeout::: call to connect(connect_timeout=%d) failed",
              connect_timeout);
      throw smsc::util::SystemError(errMsg);
    }
    fd_set rset, wset;
    FD_ZERO(&rset); FD_SET(_sockfd, &rset);
    wset = rset;
    struct timeval timeout;
    timeout.tv_sec = connect_timeout; timeout.tv_usec = 0;
    int st = ::select(_sockfd+1, &rset, &wset, NULL, &timeout);
    int error = errno;
    if ( st < 0 )
      throw smsc::util::SystemError("TCPSocket::tryConnectWithTimeout::: call to select failed", error);
    setNonBlocking(_sockfd, false);
    if ( !st ) {
      errno = ETIMEDOUT;
      return -1;
    }
    if ( FD_ISSET(_sockfd, &wset) ) {
      if ( FD_ISSET(_sockfd, &rset) ) {
        error = 0;
        int len = sizeof(error);
        if ( getsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 )
          throw smsc::util::SystemError("TCPSocket::tryConnectWithTimeout::: call to getsockopt() failed");
        errno = error;
        return -1;
      }
    }
  }
  setNonBlocking(_sockfd, false);

  return 0;
}

void
TCPSocket::close()
{
  if ( _sockfd > -1 ) {
    ::close(_sockfd);
    _sockfd = -1;
  }
}

InputStream*
TCPSocket::getInputStream() const
{
  if ( !_inputStream ) throw NotConnected("TCPSocket::getInputStream::: socket not connected");
  return _inputStream;
}

OutputStream*
TCPSocket::getOutputStream() const
{
  if ( !_outputStream ) throw NotConnected("TCPSocket::getOutputStream::: socket not connected");
  return _outputStream;
}

void
TCPSocket::setNonBlocking(bool on)
{
  setNonBlocking(_sockfd, on);
}

void
TCPSocket::setNoDelay(bool on)
{
  int flag;
  if ( on ) flag = 1;
  else flag = 0;
  if ( setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, static_cast<int>(sizeof(flag))) < 0 )
    throw smsc::util::SystemError("TCPSocket::setNoDelay::: call to setsockopt(SO_REUSEADDR) failed");
}

void
TCPSocket::setReuseAddr(bool on)
{
  int flag;
  if ( on ) flag = 1;
  else flag = 0;
  if ( setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, static_cast<int>(sizeof(flag))) < 0 )
    throw smsc::util::SystemError("TCPSocket::setReuseAddr::: call to setsockopt(SO_REUSEADDR) failed");
}

void
TCPSocket::setSoLinger(bool on, int timeout)
{
  struct linger l;

  if ( on ) {
    l.l_onoff = 1;
    l.l_linger = timeout;
  } else {
    l.l_onoff = 0;
  }
  if ( setsockopt(_sockfd, SOL_SOCKET, SO_LINGER, &l, static_cast<int>(sizeof(l))) < 0 )
    throw smsc::util::SystemError("TCPSocket::setSoLinger::: call to setsockopt(SO_LINGER) failed");
}

std::string
TCPSocket::toString() const
{
  if ( !_toString_buf_init ) {
    if ( _reverseSourceDestinationAddresses )
      snprintf(_toString_buf, sizeof(_toString_buf), "%s.%d==>%s.%d", _r_ip_address, _r_port, _l_ip_address, _l_port);
    else
      snprintf(_toString_buf, sizeof(_toString_buf), "%s.%d==>%s.%d", _l_ip_address, _l_port, _r_ip_address, _r_port);
    _toString_buf_init = true;
  }
  return std::string(_toString_buf);
}

int
TCPSocket::_getDescriptor()
{
  if ( _sockfd == -1 )
    throw smsc::util::Exception("TCPSocket::_getDescriptor::: socket closed");

  return _sockfd;
}

}}}}
