#ifndef __SMSC_CORE_AX_NETWORK_SOCKET_HPP__
# define __SMSC_CORE_AX_NETWORK_SOCKET_HPP__

# include <string>
# include <sys/types.h>
# include <netinet/in.h>

# include <smppdmplx/core_ax/network/SocketPool.hpp>

namespace smsc {
namespace core_ax {
namespace network {

class ServerSocket;
class Socket_impl;

/*
** Class is responsible for grant api to client network endpoint.
** The Socket/ServerSocket classes are realized in accordance with
** envelope-letter idiom.
*/
class Socket {
public:
  Socket(const std::string host, in_port_t port);
  Socket();

  Socket(const Socket& rhs);
  Socket& operator=(const Socket& rhs);

  virtual ~Socket();

  virtual void connect();
  virtual void close();
  void setReuseAddr();
  virtual void setNonBlocking();
  virtual void setBlocking();

  int getSocketDescriptor() const;

  virtual bool isReadable() const;
  virtual bool isWriteable() const;

  virtual bool operator==(const ServerSocket& rhs) const;
  virtual bool operator==(const Socket& rhs) const;

  virtual bool operator<(const Socket&) const;

  const std::string toString() const;
protected:
  Socket(int sockFd, struct sockaddr_in peerAddr);
  Socket(ServerSocket&);
  //  virtual operator void*();

  virtual void markAsReadable();
  virtual void markAsWriteable();

  Socket_impl *_impl;

  //friend Socket SocketPool::listen();
  //friend void SocketPool::push_socket(Socket& sock);
  //friend void SocketPool::push_socket(ServerSocket& sock);
  friend class SocketPool;
  friend class ServerSocket;
  friend class Socket_impl;
};

/*
  Class is responsible for grant api to server network endpoint.
*/
class ServerSocket /*: private Socket*/ {
public:
  ServerSocket(in_port_t listeningPort);
  ServerSocket(Socket& readySocket);
  ServerSocket();

  ServerSocket(const Socket& rhs);
  ServerSocket& operator=(const Socket& rhs);

  virtual ~ServerSocket();

  virtual Socket accept() const;
  virtual void close();
  void setReuseAddr();
  virtual void setNonBlocking();
  virtual void setBlocking();

  int getSocketDescriptor() const;

  virtual bool operator==(const ServerSocket& rhs) const;
  virtual bool operator==(const Socket& rhs) const;

  std::string toString() const;
protected:
  //  virtual operator void*();
  Socket_impl *_impl;

  //friend void SocketPool::push_socket(ServerSocket& sock);
  friend bool Socket::operator==(const ServerSocket& rhs) const;
  friend Socket::Socket(ServerSocket& rhs);
};

}
}
}

#endif
