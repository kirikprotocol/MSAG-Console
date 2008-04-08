#include "SCTPSocket.hpp"
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <logger/Logger.h>
#include <sua/utilx/Exception.hpp>

namespace corex {
namespace io {
namespace network {

void
SCTPSocket::initSocket(in_port_t port)
{
  _sockfd = socket(AF_INET, /*SOCK_SEQPACKET*/ SOCK_STREAM, IPPROTO_SCTP);
  if ( _sockfd < 0 ) throw smsc::util::SystemError("SCTPSocket::initSocket::: call to socket() failed");

  struct sctp_event_subscribe evnts;

  memset(&evnts, 0, sizeof(evnts));
  evnts.sctp_data_io_event = 1;
  //  evnts.sctp_address_event = 1;
  //  evnts.sctp_association_event = 1;
  //  evnts.sctp_send_failure_event = 1;
  if ( setsockopt(_sockfd,IPPROTO_SCTP, SCTP_EVENTS,
                  &evnts, sizeof(evnts)) < 0 )
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

SCTPSocket::SCTPSocket(int sockfd)
  : _sockfd(sockfd)
{
  if ( _sockfd < 0 )
    throw smsc::util::SystemError("SCTPSocket::SCTPSocket::: input socket fd value is less than 0");

  _inputStream = new SctpInputStream(this, sockfd);
  if ( !_inputStream )
    throw smsc::util::SystemError("SCTPSocket::SCTPSocket::: can't allocate memory for InputStream");

  _outputStream = new SctpOutputStream(this, sockfd);
  if ( !_outputStream )
    throw smsc::util::SystemError("SCTPSocket::SCTPSocket::: can't allocate memory for OutputStream");

  struct sctp_event_subscribe evnts;

  memset(&evnts, 0, sizeof(evnts));
  evnts.sctp_data_io_event = 1;
  //  evnts.sctp_address_event = 1;
  //  evnts.sctp_association_event = 1;
  //  evnts.sctp_send_failure_event = 1;
  if ( setsockopt(_sockfd,IPPROTO_SCTP, SCTP_EVENTS,
                  &evnts, sizeof(evnts)) < 0 )
    throw smsc::util::SystemError("SCTPSocket::initSocket::: call to setsockopt() failed");

  fillToStringInfo();
  _reverseSourceDestinationAddresses = true;
}

SCTPSocket::~SCTPSocket()
{
  close();
  delete _inputStream; delete _outputStream;
}

void
SCTPSocket::fillAddressString(struct sockaddr *saddr, int addrsNum, char *addresses_info, size_t addresses_info_sz)
{
  struct sockaddr_in* ipv4_addr;

  memset(addresses_info, 0, addresses_info_sz);

  ipv4_addr = reinterpret_cast<struct sockaddr_in*>(saddr);
  char lport[16];
  char ip_address[32];
  sprintf(lport, ",%d", ntohs(ipv4_addr->sin_port));
  for (int i=0; i<addrsNum; ++i) {
    if (ipv4_addr->sin_family == AF_INET) {
      inet_ntop(AF_INET, &ipv4_addr->sin_addr, ip_address, sizeof(ip_address));
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
SCTPSocket::getInputStream()
{
  if ( !_inputStream ) throw corex::io::NotConnected("SCTPSocket::getInputStream::: socket not connected");
  return _inputStream;
}

SctpOutputStream*
SCTPSocket::getOutputStream()
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
  if ( setsockopt(_sockfd, IPPROTO_SCTP, SCTP_NODELAY, &flag, sizeof(flag)) < 0 )
    throw smsc::util::SystemError("SCTPSocket::setNoDelay::: call to setsockopt(SO_REUSEADDR) failed");
}

void
SCTPSocket::setReuseAddr(bool on)
{
  int flag;
  if ( on ) flag = 1;
  else flag = 0;
  if ( setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0 )
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
  if ( setsockopt(_sockfd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0 )
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
    if ( !(hp = gethostbyname_r(_dst_host.c_str(), &h_result, addr_list_buffer, sizeof(addr_list_buffer), &h_errno_buf)) )
      throw smsc::util::SystemError("SCTPSocket::connect::: call to gethostbyname() failed", h_errno_buf);
#else
    if ( !(hp = gethostbyname(_dst_host.c_str())) )
      throw smsc::util::SystemError("SCTPSocket::connect::: call to gethostbyname() failed", h_errno);
#endif

    if ( hp->h_addrtype != AF_INET )
      throw smsc::util::Exception("SCTPSocket::connect::: gethostbyname() returned unsupported addrtype [%d]", hp->h_addrtype);

    for(char** pptr = hp->h_addr_list; *pptr; ++pptr) {
      memcpy(&_server_addr.sin_addr, *pptr, sizeof(_server_addr.sin_addr));
      if ( ::connect(_sockfd, (sockaddr*)&_server_addr, sizeof(_server_addr)) == 0 ) {
        fillToStringInfo();
        _inputStream = new SctpInputStream(this, _sockfd); _outputStream = new SctpOutputStream(this, _sockfd);
        return;
      }
    }

    throw smsc::util::Exception("SCTPSocket::connect::: can't establish connect");
  } else {
    if ( ::connect(_sockfd, (sockaddr*)&_server_addr, sizeof(_server_addr)) < 0 )
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
  return _sockfd;
}

SctpOutputStream::SctpOutputStream(IOObject* owner, int fd)
  : _owner(owner), _fd(fd)/*, _streamNo(0), _sendFlags(0)*/ {}

// void
// SctpOutputStream::setStreamNo(uint16_t streamNo)
// {
//   _streamNo = streamNo;
// }

// void
// SctpOutputStream::setStreamOrdering(bool on)
// {
//   if ( on ) _sendFlags |= MSG_UNORDERED;
//   else _sendFlags &= ~MSG_UNORDERED;
// }

ssize_t
SctpOutputStream::write(const uint8_t *buf, size_t bufSz, uint16_t streamNo, bool ordered) const
{
  uint32_t sendFlags=0;
  if ( ordered ) sendFlags |= MSG_UNORDERED;
  else sendFlags &= ~MSG_UNORDERED;

  ssize_t result = sctp_sendmsg(_fd, buf,  bufSz,
                                NULL, 0, 0,
                                sendFlags,  streamNo,  0, -1);


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
{ return _owner; }

SctpInputStream::SctpInputStream(IOObject* owner, int fd)
  : _owner(owner), _fd(fd)
{
  memset((uint8_t*)&_sinfo, 0, sizeof(_sinfo));
}

ssize_t
SctpInputStream::read(uint8_t *buf, size_t bufSz)
{
  sockaddr_in from;
  socklen_t fromlen = sizeof(from);
  int msg_flags;
  ssize_t result;
  do {
    msg_flags=0;
    result = sctp_recvmsg(_fd, buf, bufSz,  (sockaddr*)&from, &fromlen, &_sinfo, &msg_flags);
    // REMOVE DEBUG OUTPUT
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("sctp");
    smsc_log_info(logger, "SctpInputStream::read::: sctp_recvmsg returned %d, errno=%d, msg_flags & MSG_NOTIFICATION=%d", result, errno, (msg_flags & MSG_NOTIFICATION));

    if ( msg_flags & MSG_NOTIFICATION ) {
      sctp_notification* snp = (sctp_notification*)buf;
      
      if ( snp->sn_header.sn_type == SCTP_PEER_ADDR_CHANGE ) {
        const sctp_paddr_change& sac = snp->sn_paddr_change;
        char ip_address[32]={0};
        
        if ( sac.spc_aaddr.ss_family == AF_INET ) {
          struct sockaddr_in* sin_addr = (struct sockaddr_in* )&sac.spc_aaddr;
          
          inet_ntop(AF_INET, &sin_addr->sin_addr, ip_address, sizeof(ip_address));
        }
        //        printf("sac->spc_state =%d for address=[%s]", sac.spc_state, ip_address);
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

IOObject*
SctpInputStream::getOwner()
{ return _owner; }

uint16_t
SctpInputStream::getStreamNo() const
{
  return _sinfo.sinfo_stream;
}

}}}
