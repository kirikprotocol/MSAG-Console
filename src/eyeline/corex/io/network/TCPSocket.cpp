#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <util/Exception.hpp>
#include <eyeline/corex/io/network/TCPSocket.hpp>
#include <eyeline/corex/io/IOExceptions.hpp>

namespace eyeline {
namespace corex {
namespace io {
namespace network {

TCPSocket::TCPSocket(in_port_t port)
  : _toString_buf_init(false), _inputStream(NULL), _outputStream(NULL), _reverseSourceDestinationAddresses(false)
{
  memset(_l_ip_address, 0, sizeof(_l_ip_address));
  memset(_r_ip_address, 0, sizeof(_r_ip_address));
  _l_port = port; _r_port = 0;

  _sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if ( _sockfd < 0 ) throw smsc::util::SystemError("TCPSocket::TCPSocket::: call to socket() failed");

  memset((uint8_t*)&_server_addr, 0, sizeof(_server_addr));
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
  _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  inet_ntop(AF_INET, &_server_addr.sin_addr, _l_ip_address, static_cast<int>(sizeof(_l_ip_address)));
}

TCPSocket::TCPSocket(const std::string& host, in_port_t port)
  : _toString_buf_init(false), _inputStream(NULL), _outputStream(NULL), _reverseSourceDestinationAddresses(false)
{
  memset(_l_ip_address, 0, sizeof(_l_ip_address));
  memset(_r_ip_address, 0, sizeof(_r_ip_address));
  _l_port = 0; _r_port = 0;

  _sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if ( _sockfd < 0 ) throw smsc::util::SystemError("TCPSocket::TCPSocket::: call to socket() failed");
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
  if ( !_inputStream )
    throw smsc::util::SystemError("TCPSocket::TCPSocket::: can't allocate memory for InputStream");

  _outputStream = new GenericOutputStream(this, sockfd);
  if ( !_outputStream )
    throw smsc::util::SystemError("TCPSocket::TCPSocket::: can't allocate memory for OutputStream");

  _reverseSourceDestinationAddresses = true;
  fillToStringInfo();
}

TCPSocket::~TCPSocket() {
  delete _inputStream; delete _outputStream;
  close();
}

void TCPSocket::connect()
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

    for(char** pptr = hp->h_addr_list; *pptr; ++pptr) {
      memcpy(&_server_addr.sin_addr, *pptr, sizeof(_server_addr.sin_addr));
      if ( ::connect(_sockfd, (sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) == 0 ) {
        fillToStringInfo();
        _inputStream = new GenericInputStream(this, _sockfd); _outputStream = new GenericOutputStream(this, _sockfd);
        return;
      }
    }

    throw smsc::util::Exception("TCPSocket::connect::: can't establish connect");
  } else {
    if ( ::connect(_sockfd, (sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) < 0 )
      throw smsc::util::SystemError("TCPSocket::connect::: can't establish connect");
    fillToStringInfo();
    _inputStream = new GenericInputStream(this, _sockfd); _outputStream = new GenericOutputStream(this, _sockfd);
  }
}

void
TCPSocket::close()
{
  if ( _sockfd > -1 ) {
    ::close(_sockfd);
    _sockfd = -1;
  }
}

corex::io::InputStream*
TCPSocket::getInputStream()
{
  if ( !_inputStream ) throw corex::io::NotConnected("TCPSocket::getInputStream::: socket not connected");
  return _inputStream;
}

corex::io::OutputStream*
TCPSocket::getOutputStream()
{
  if ( !_outputStream ) throw corex::io::NotConnected("TCPSocket::getOutputStream::: socket not connected");
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
  return _sockfd;
}

}}}}
