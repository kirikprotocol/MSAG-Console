#ifndef __SMSC_CORE_AX_NETWORK_SOCKETPOOL_HPP__
# define __SMSC_CORE_AX_NETWORK_SOCKETPOOL_HPP__

# include <sys/types.h>
# include <sys/time.h>
# include <list>
# include <utility>

# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smsc {
namespace core_ax {
namespace network {

class Socket;
class ServerSocket;

class SocketPool
{
public:
  SocketPool(size_t maxSocketsInPool=32);
  ~SocketPool();

  typedef enum { WAIT_READABLE=0, WAIT_WRITEABLE=1 } wait_socket_type_t;
  typedef enum { TIMEOUT=0, HAVE_READY_SOCKET=1 } listen_status_t;
  virtual void push_socket(Socket& sock, wait_socket_type_t wait_type = WAIT_READABLE);
  virtual void push_socket(ServerSocket& sock);
  virtual void remove_socket(const Socket& sock, wait_socket_type_t wait_type=smsc::core_ax::network::SocketPool::WAIT_READABLE);
  virtual void remove_socket(const ServerSocket& sock);

  typedef std::list<Socket> SocketList_t;

  virtual Socket listen();
  virtual listen_status_t listen(SocketList_t& readyForRead, SocketList_t& readyForWrite);
  void setTimeOut(time_t timeOut);
protected:
  const size_t _maxSocketsInPool;
  size_t _maxFd;

  fd_set _fd_read, _fd_write;
  struct timeval _timeout;

  // список сокетов ожидающих готовности на чтение
  SocketList_t _socketList;
  // список сокетов ожидающих готовности на запись
  SocketList_t _writeableSocketList;
private:
  // disable copy
  SocketPool(const SocketPool& rhs);
  SocketPool& operator=(const SocketPool& rhs);
};

}
}
}

#endif
