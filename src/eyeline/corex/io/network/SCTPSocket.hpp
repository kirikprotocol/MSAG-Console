#ifndef __EYELINE_COREX_IO_NETWORK_SCTPSOCKET_HPP__
# define __EYELINE_COREX_IO_NETWORK_SCTPSOCKET_HPP__

#ifndef __SunOS_5_9

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/sctp.h>
# include <arpa/inet.h>

# include <string>
# include <vector>

# include "eyeline/corex/io/network/Socket.hpp"
# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/corex/io/IOObject.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

class SCTPServerSocket;

class SctpInputStream : public InputStream {
public:
  SctpInputStream(IOObject* owner, int fd);
  virtual ssize_t read(uint8_t *buf, size_t bufSz);
  virtual ssize_t readv(const struct iovec *iov, int iovcnt);
  virtual IOObject* getOwner();
  uint16_t getStreamNo() const; //sinfo->sinfo_stream;
private:
  IOObject* _owner;
  int _fd;
  sctp_sndrcvinfo _sinfo;
};

class SctpOutputStream : public OutputStream {
public:
  SctpOutputStream(IOObject* owner, int fd);
  virtual ssize_t write(const uint8_t *buf, size_t bufSz, uint16_t streamNo=0, bool ordered=false) const;
  virtual ssize_t writev(const struct iovec *iov, int iovcnt) const;
  virtual IOObject* getOwner();
private:
  IOObject* _owner;
  int _fd;
};

class SCTPSocket : public Socket {
public:
  SCTPSocket(const std::string& dst_host, in_port_t port);

  virtual ~SCTPSocket();
  void connect();
  virtual void close();

  virtual SctpInputStream* getInputStream() const;
  virtual SctpOutputStream* getOutputStream() const;

  using IOObject::setNonBlocking;

  virtual void setNonBlocking(bool on);
  virtual void setNoDelay(bool on);
  virtual void setReuseAddr(bool on);
  virtual void setSoLinger(bool on, int timeout=0);

  virtual std::string toString() const;

  void bindx(const std::string local_addrs[], size_t addrs_num, in_port_t local_port);
protected:
  explicit SCTPSocket(int sockfd);
  explicit SCTPSocket(in_port_t port);

  friend class SCTPServerSocket;

  SCTPSocket(const SCTPSocket&);
  SCTPSocket& operator=(const SCTPSocket&);

  void fillAddressString(struct sockaddr *saddr, int addrs_num, char *addresses_info, size_t addresses_info_sz);
  void fillToStringInfo();

  virtual int _getDescriptor();

  int _sockfd;
  sockaddr_in _server_addr;

  std::string _dst_host;

  char _l_addresses_info[256], _r_addresses_info[256];
  mutable char _toString_buf[128];

  SctpInputStream* _inputStream;
  SctpOutputStream* _outputStream;
  bool _reverseSourceDestinationAddresses;
private:
  void initSocket(in_port_t port);
};

}}}}


#endif

#endif
