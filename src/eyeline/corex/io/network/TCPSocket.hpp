#ifndef __EYELINE_COREX_IO_NETWORK_TCPSOCKET_HPP__
# define __EYELINE_COREX_IO_NETWORK_TCPSOCKET_HPP__

# include <string>

# include "eyeline/corex/io/network/Socket.hpp"
# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/corex/io/IOObject.hpp"

namespace eyeline {
namespace corex {
namespace io {
namespace network {

class TCPServerSocket;

class TCPSocket : public Socket {
public:
  TCPSocket(const std::string& host, in_port_t port);

  virtual ~TCPSocket();
  void connect(unsigned connect_timeout=0);
  virtual void close();

  virtual InputStream* getInputStream() const;
  virtual OutputStream* getOutputStream() const;

  using IOObject::setNonBlocking;

  virtual void setNonBlocking(bool on);
  virtual void setNoDelay(bool on);
  virtual void setReuseAddr(bool on);
  virtual void setSoLinger(bool on, int timeout=0);
  virtual std::string toString() const;
protected:
  explicit TCPSocket(int sockfd);
  explicit TCPSocket(in_port_t port);

  friend class TCPServerSocket;

  TCPSocket(const TCPSocket&);
  TCPSocket& operator=(const TCPSocket&);

  void fillToStringInfo(bool needPeerAddr=true);

  virtual int _getDescriptor();

  void openSocket();
  int tryConnectWithTimeout(int connect_timeout);

  int _sockfd;
  sockaddr_in _server_addr;

  in_port_t _l_port, _r_port;
  std::string _host;
  char _l_ip_address[46], _r_ip_address[46];

  mutable bool _toString_buf_init;
  mutable char _toString_buf[128];

  InputStream* _inputStream;
  OutputStream* _outputStream;

  bool _reverseSourceDestinationAddresses;
};

}}}}

#endif
