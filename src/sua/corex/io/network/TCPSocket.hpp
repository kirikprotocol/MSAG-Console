#ifndef __COREX_IO_NETWORK_TCPSOCKET_HPP__
# define __COREX_IO_NETWORK_TCPSOCKET_HPP__ 1

# include <string>
# include <sua/corex/io/network/Socket.hpp>
# include <sua/corex/io/IOStreams.hpp>
# include <sua/corex/io/IOObject.hpp>

namespace corex {
namespace io {
namespace network {

class TCPServerSocket;

class TCPSocket : public Socket {
public:
  TCPSocket(const std::string& host, in_port_t port);

  virtual ~TCPSocket();
  virtual void connect();
  virtual void close();

  virtual corex::io::InputStream* getInputStream();
  virtual corex::io::OutputStream* getOutputStream();

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

  int _sockfd;
  sockaddr_in _server_addr;

  in_port_t _l_port, _r_port;
  std::string _host;
  char _l_ip_address[46], _r_ip_address[46];

  mutable bool _toString_buf_init;
  mutable char _toString_buf[128];

  corex::io::GenericInputStream* _inputStream;
  corex::io::GenericOutputStream* _outputStream;

  bool _reverseSourceDestinationAddresses;
};

}}}

#endif
