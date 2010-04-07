#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include "eyeline/corex/io/IOExceptions.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "SCTPSocket.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

void
SCTPSocket::initSocket(in_port_t port)
{
  _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
  if ( _sockfd < 0 ) throw smsc::util::SystemError("SCTPSocket::initSocket::: call to socket() failed");

  struct sctp_event_subscribe evnts;

  memset(&evnts, 0, sizeof(evnts));
  evnts.sctp_data_io_event = 1;

  if ( setsockopt(_sockfd,IPPROTO_SCTP, SCTP_EVENTS,
                  &evnts, static_cast<int>(sizeof(evnts))) < 0 )
    throw smsc::util::SystemError("SCTPSocket::initSocket::: call to setsockopt() failed");

  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
}

SCTPSocket::SCTPSocket(const std::string& dst_host, in_port_t port)
  : _inputStream(NULL), _outputStream(NULL), _reverseSourceDestinationAddresses(false)
{
  _dst_host = dst_host;
  initSocket(port);
}

SCTPSocket::SCTPSocket(in_port_t port)
  : _inputStream(NULL), _outputStream(NULL), _reverseSourceDestinationAddresses(false)
{
  initSocket(port);

  _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

SCTPSocket::SCTPSocket(int sock_fd)
  : _sockfd(sock_fd)
{
  if ( _sockfd < 0 )
    throw smsc::util::SystemError("SCTPSocket::SCTPSocket::: input socket fd value is less than 0");

  _inputStream = new SctpInputStream(this, _sockfd);
  _outputStream = new SctpOutputStream(this, _sockfd);

  struct sctp_event_subscribe evnts;

  memset(&evnts, 0, sizeof(evnts));
  evnts.sctp_data_io_event = 1;

  if ( setsockopt(_sockfd,IPPROTO_SCTP, SCTP_EVENTS,
                  &evnts, static_cast<int>(sizeof(evnts))) < 0 )
    throw smsc::util::SystemError("SCTPSocket::initSocket::: call to setsockopt() failed");

  fillToStringInfo();
  _reverseSourceDestinationAddresses = true;
}

SCTPSocket::~SCTPSocket()
{
  delete _inputStream; delete _outputStream;
  close();
}

void
SCTPSocket::fillAddressString(struct sockaddr *saddr, int addrs_num, char *addresses_info, size_t addresses_info_sz)
{
  struct sockaddr_in* ipv4_addr;

  memset(addresses_info, 0, addresses_info_sz);

  ipv4_addr = reinterpret_cast<struct sockaddr_in*>(saddr);
  char lport[16];
  char ip_address[32];
  sprintf(lport, ",%d", ntohs(ipv4_addr->sin_port));
  for (int i=0; i<addrs_num; ++i) {
    if (ipv4_addr->sin_family == AF_INET) {
      inet_ntop(AF_INET, &ipv4_addr->sin_addr, ip_address, static_cast<int>(sizeof(ip_address)));
      if ( i > 0 )
        strcat(addresses_info, ",");
      strlcat(addresses_info, ip_address, addresses_info_sz);
      ++ipv4_addr;
    } else
      throw smsc::util::Exception("fillAddressString::: got address with family = %d (!=AF_INET)", ipv4_addr->sin_family);
  }
  strcat(addresses_info, lport);
}

void
SCTPSocket::fillToStringInfo()
{
  sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  struct sockaddr *saddr;

  int addrsNum = sctp_getladdrs(_sockfd, 0, (void**)&saddr);
  fillAddressString(saddr, addrsNum, _l_addresses_info, sizeof(_l_addresses_info));
  sctp_freeladdrs(saddr);

  addrsNum = sctp_getpaddrs(_sockfd, 0, (void**)&saddr);
  fillAddressString(saddr, addrsNum, _r_addresses_info, sizeof(_r_addresses_info));
  sctp_freepaddrs(saddr);
}

SctpInputStream*
SCTPSocket::getInputStream() const
{
  if ( !_inputStream ) throw corex::io::NotConnected("SCTPSocket::getInputStream::: socket not connected");
  return _inputStream;
}

SctpOutputStream*
SCTPSocket::getOutputStream() const
{
  if ( !_outputStream ) throw corex::io::NotConnected("SCTPSocket::getOutputStream::: socket not connected");
  return _outputStream;
}

void
SCTPSocket::setNonBlocking(bool on)
{
  IOObject::setNonBlocking(_sockfd, on);
}

void
SCTPSocket::setNoDelay(bool on)
{
  int flag;
  if ( on ) flag = 1;
  else flag = 0;
  if ( setsockopt(_sockfd, IPPROTO_SCTP, SCTP_NODELAY, &flag, static_cast<int>(sizeof(flag))) < 0 )
    throw smsc::util::SystemError("SCTPSocket::setNoDelay::: call to setsockopt(SO_REUSEADDR) failed");
}

void
SCTPSocket::setReuseAddr(bool on)
{
  int flag;
  if ( on ) flag = 1;
  else flag = 0;
  if ( setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, static_cast<int>(sizeof(flag))) < 0 )
    throw smsc::util::SystemError("SCTPSocket::setReuseAddr::: call to setsockopt(SO_REUSEADDR) failed");
}

void
SCTPSocket::setSoLinger(bool on, int timeout)
{
  struct linger l;

  if ( on ) {
    l.l_onoff = 1;
    l.l_linger = timeout;
  } else {
    l.l_onoff = 0;
  }
  if ( setsockopt(_sockfd, SOL_SOCKET, SO_LINGER, &l, static_cast<int>(sizeof(l))) < 0 )
    throw smsc::util::SystemError("SCTPSocket::setSoLinger::: call to setsockopt(SO_LINGER) failed");
}

void
SCTPSocket::connect()
{
  if ( _dst_host == "" )
    throw smsc::util::Exception("SCTPSocket::connect::: destination host wasn't specified");

  if ( inet_pton(AF_INET, _dst_host.c_str(), &_server_addr.sin_addr) < 1 ) {
    struct hostent *hp;
    struct hostent h_result;
    char addr_list_buffer[1024];
    int h_errno_buf;

#ifdef _REENTRANT
    if ( !(hp = gethostbyname_r(_dst_host.c_str(), &h_result, addr_list_buffer, static_cast<int>(sizeof(addr_list_buffer)), &h_errno_buf)) )
      throw smsc::util::SystemError("SCTPSocket::connect::: call to gethostbyname() failed", h_errno_buf);
#else
    if ( !(hp = gethostbyname(_dst_host.c_str())) )
      throw smsc::util::SystemError("SCTPSocket::connect::: call to gethostbyname() failed", h_errno);
#endif

    if ( hp->h_addrtype != AF_INET )
      throw smsc::util::Exception("SCTPSocket::connect::: gethostbyname() returned unsupported addrtype [%d]", hp->h_addrtype);

    bool connRefused = false;
    for(char** pptr = hp->h_addr_list; *pptr; ++pptr) {
      memcpy(&_server_addr.sin_addr, *pptr, sizeof(_server_addr.sin_addr));
      if ( ::connect(_sockfd, (sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) == 0 ) {
        fillToStringInfo();
        _inputStream = new SctpInputStream(this, _sockfd); _outputStream = new SctpOutputStream(this, _sockfd);
        return;
      }
      if ( errno = ECONNREFUSED )
        connRefused = true;
    }
    if ( connRefused )
      throw ConnectionFailedException("SCTPSocket::connect::: connection refused");
    else
      throw smsc::util::Exception("SCTPSocket::connect::: can't establish connect");
  } else {
    if ( ::connect(_sockfd, (sockaddr*)&_server_addr, static_cast<int>(sizeof(_server_addr))) < 0 )
      throw smsc::util::SystemError("SCTPSocket::connect::: can't establish connect");

    fillToStringInfo();
    _inputStream = new SctpInputStream(this, _sockfd); _outputStream = new SctpOutputStream(this, _sockfd);
  }
}

void
SCTPSocket::close()
{
  if ( _sockfd > -1 ) {
    ::close(_sockfd);
    _sockfd = -1;
  }
}

std::string
SCTPSocket::toString() const
{
  if ( _reverseSourceDestinationAddresses )
    snprintf(_toString_buf, sizeof(_toString_buf), "%s==>%s", _r_addresses_info, _l_addresses_info);
  else
    snprintf(_toString_buf, sizeof(_toString_buf), "%s==>%s",_l_addresses_info, _r_addresses_info);
  return std::string(_toString_buf);
}

int
SCTPSocket::_getDescriptor()
{
  if ( _sockfd == -1 )
      throw smsc::util::Exception("SCTPSocket::_getDescriptor::: socket closed");
  return _sockfd;
}

void
SCTPSocket::bindx(const std::string local_addrs[], size_t addrs_count, in_port_t local_port)
{
  if ( addrs_count > 0 ) {
    sockaddr_in localIfaceAddress;

    localIfaceAddress.sin_family = AF_INET;
    localIfaceAddress.sin_port = htons(local_port);
    if ( inet_pton(AF_INET, local_addrs[0].c_str(), &localIfaceAddress.sin_addr) < 1 )
      throw smsc::util::Exception("SCTPSocket::bindx::: local interface address mustn't be host name");

    if ( ::bind(_sockfd, (struct sockaddr*)&localIfaceAddress, static_cast<int>(sizeof(localIfaceAddress))) )
      throw smsc::util::SystemError("SCTPSocket::bindx::: call to bind() failed");

    if ( addrs_count > 1 ) {
      struct sockaddr_in *bindxAddrList;
      bindxAddrList = new sockaddr_in[addrs_count-1];

      for (int i=0; i<addrs_count-1; i++) {
        if ( inet_pton(AF_INET, local_addrs[i+1].c_str(), &localIfaceAddress.sin_addr) < 1 )
          throw smsc::util::Exception("SCTPSocket::bindx::: local interface address mustn't be host name");
        bindxAddrList[i] = localIfaceAddress;
      }

      if ( addrs_count > 1 ) {
        if ( ::sctp_bindx(_sockfd, bindxAddrList, static_cast<int>(addrs_count-1), SCTP_BINDX_ADD_ADDR) ) {
          delete [] bindxAddrList;
          throw smsc::util::SystemError("SCTPSocket::bindx::: call to sctp_bindx() failed");
        }
      }
      delete [] bindxAddrList;
    }
  }
}

SctpOutputStream::SctpOutputStream(IOObject* owner, int fd)
  : _owner(owner), _fd(fd) {}

ssize_t
SctpOutputStream::write(const uint8_t *buf, size_t buf_sz, uint16_t stream_no, bool ordered) const
{
  uint32_t sendFlags=0;
  if ( ordered ) sendFlags |= MSG_UNORDERED;
  else sendFlags &= ~MSG_UNORDERED;

  ssize_t result = sctp_sendmsg(_fd, buf,  buf_sz,
                                NULL, 0, 0,
                                sendFlags,  stream_no,  0, -1);


  if ( result < 0 ) {
    if ( errno == EPIPE )
      throw corex::io::BrokenPipe("SctpOutputStream::write::: write() returned EPIPE");
    else
      throw smsc::util::SystemError("SctpOutputStream::write::: call to sctp_sendmsg() failed");
  }

  return result;
}

ssize_t
SctpOutputStream::writev(const struct iovec *iov, int iovcnt) const
{
  throw smsc::util::SystemError("SctpOutputStream::writev::: not implemented");
}

IOObject*
SctpOutputStream::getOwner()
{
  return _owner;
}

SctpInputStream::SctpInputStream(IOObject* owner, int fd)
  : _owner(owner), _fd(fd)
{
  memset((uint8_t*)&_sinfo, 0, sizeof(_sinfo));
}

ssize_t
SctpInputStream::read(uint8_t *buf, size_t buf_sz)
{
  sockaddr_in from;
  socklen_t fromlen = sizeof(from);
  int msg_flags;
  ssize_t result;
  do {
    msg_flags=0;
    result = sctp_recvmsg(_fd, buf, buf_sz, (sockaddr*)&from, &fromlen, &_sinfo, &msg_flags);

    if ( msg_flags & MSG_NOTIFICATION ) {
      sctp_notification* snp = (sctp_notification*)buf;

      if ( snp->sn_header.sn_type == SCTP_PEER_ADDR_CHANGE ) {
        const sctp_paddr_change& sac = snp->sn_paddr_change;
        char ip_address[32]={0};

        if ( sac.spc_aaddr.ss_family == AF_INET ) {
          struct sockaddr_in* sin_addr = (struct sockaddr_in* )&sac.spc_aaddr;

          inet_ntop(AF_INET, &sin_addr->sin_addr, ip_address, static_cast<int>(sizeof(ip_address)));
        }
      }
    }
  } while (msg_flags & MSG_NOTIFICATION);
  if ( result < 0 ) {
    if ( errno == EINTR )
      throw utilx::InterruptedException("SctpInputStream::read::: sctp_recvmsg() was interrupted");
    else
      throw smsc::util::SystemError("SctpInputStream::read::: call to sctp_recvmsg() failed");
  }

  if ( !result )
    throw corex::io::EOFException("SctpInputStream::read::: connection closed by remote side");

  return result;
}

ssize_t
SctpInputStream::readv(const struct iovec *iov, int iovcnt)
{
  throw smsc::util::SystemError("SctpInputStream::readv::: not implemented");
}

IOObject*
SctpInputStream::getOwner()
{
  return _owner;
}

uint16_t
SctpInputStream::getStreamNo() const
{
  return _sinfo.sinfo_stream;
}

}}}}
