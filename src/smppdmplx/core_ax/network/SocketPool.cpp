#include <string.h>
#include <errno.h>

#include <smppdmplx/core_ax/network/SocketPool.hpp>
#include <util/Exception.hpp>

#include <algorithm>
#include <sstream>

#include <logger/Logger.h>

void
smsc::core_ax::network::SocketPool::push_socket(Socket& sock, wait_socket_type_t wait_type)
{
  if ( wait_type == WAIT_READABLE) {
    if ( std::find(_socketList.begin(),_socketList.end(),sock) == _socketList.end() ) {
      if ( _socketList.size() + _writeableSocketList.size() <
           _maxSocketsInPool ) {
        _socketList.push_back(sock);
        FD_SET(sock.getSocketDescriptor(), &_fd_read);
        _maxFd = std::max(_maxFd, (size_t)sock.getSocketDescriptor());
      } else
        throw smsc::util::Exception("SocketPool::push_socket::: Max. nums. of listened sockets are excedeed");
    }
  } else {
    if ( std::find(_writeableSocketList.begin(),_writeableSocketList.end(),sock) == _writeableSocketList.end() ) {
      if ( _writeableSocketList.size() + _socketList.size() <
           _maxSocketsInPool ) {
        _writeableSocketList.push_back(sock);
        FD_SET(sock.getSocketDescriptor(), &_fd_write);
        _maxFd = std::max(_maxFd, (size_t)sock.getSocketDescriptor());
      } else
        throw smsc::util::Exception("SocketPool::push_socket::: Max. nums. of listened sockets are excedeed");
    }
  }
}

void
smsc::core_ax::network::SocketPool::push_socket(ServerSocket& sock)
{
  push_socket((Socket&)sock);
}

void
smsc::core_ax::network::SocketPool::remove_socket(const Socket& sock, wait_socket_type_t wait_type)
{
  if ( wait_type != smsc::core_ax::network::SocketPool::WAIT_WRITEABLE ) {
    SocketList_t::iterator iter =
      std::find(_socketList.begin(),_socketList.end(),sock);
    if (  iter != _socketList.end() ) {
      FD_CLR(sock.getSocketDescriptor(), &_fd_read);
      _socketList.erase(iter);
    }
  }
  SocketList_t::iterator wrSockIter = std::find(_writeableSocketList.begin(), _writeableSocketList.end(), sock);
  if (  wrSockIter != _writeableSocketList.end() ) {
    FD_CLR(sock.getSocketDescriptor(), &_fd_write);
    _writeableSocketList.erase(wrSockIter);
  }
}

void
smsc::core_ax::network::SocketPool::remove_socket(const ServerSocket& sock)
{
  remove_socket(sock);
}

smsc::core_ax::network::Socket
smsc::core_ax::network::SocketPool::listen()
{
  fd_set tmp_fd_read;
  tmp_fd_read = _fd_read;

  struct timeval* timeoutPtr;
  struct timeval timeout = _timeout;
  if ( !timeout.tv_sec && !timeout.tv_usec )
    timeoutPtr = NULL;
  else
    timeoutPtr = &timeout;

  /*
  ** Ожидаем поступления новых данных 
  */
  int st = ::select(_maxFd+1, &tmp_fd_read, NULL/*&tmp_fd_write*/, NULL, timeoutPtr);

  if ( st < 0 )
    throw smsc::util::SystemError("SocketPool::listen::: ::listen() failed");

  if ( st > 0 ) {
    for (SocketList_t::iterator iter = _socketList.begin(); iter != _socketList.end(); ++iter) {
      if ( FD_ISSET ((*iter).getSocketDescriptor(), &tmp_fd_read) ) {
        (*iter).markAsReadable();
        FD_CLR((*iter).getSocketDescriptor(), &_fd_read);

        Socket readySocket(*iter);
        _socketList.erase(iter);
        return readySocket;
      }
    }
    throw smsc::util::Exception("SocketPool::listen::: None of the sockets was found");
  }
}

static void fdDumps(const char* where, int maxFd, fd_set* fd_mask)
{
  std::ostringstream strBuf;
  for (int i=0; i<maxFd; ++i) {
    if ( FD_ISSET(i, fd_mask) )
      strBuf << i << " ";
  }

  smsc::logger::Logger* log = smsc::logger::Logger::getInstance("sock");
  smsc_log_debug(log,"______DBG::: %s: set following descriptors: [%s]", where, strBuf.str().c_str());
}

smsc::core_ax::network::SocketPool::listen_status_t
smsc::core_ax::network::SocketPool::listen(SocketList_t& readyForRead, SocketList_t& readyForWrite)
{
  fd_set tmp_fd_read, tmp_fd_write;
  tmp_fd_read = _fd_read;
  tmp_fd_write = _fd_write;

  struct timeval* timeoutPtr;
  struct timeval timeout = _timeout;
  if ( !timeout.tv_sec && !timeout.tv_usec )
    timeoutPtr = NULL;
  else
    timeoutPtr = &timeout;

  /*
  ** Ожидаем поступления новых данных
  */
  int st = ::select(_maxFd+1, &tmp_fd_read, &tmp_fd_write, NULL, timeoutPtr);
  if ( st < 0 )
    throw smsc::util::SystemError("SocketPool::listen::: ::listen() failed");
  else if ( st == 0 )
    return TIMEOUT;
  else if ( st > 0 ) {
    // Если есть готовые сокеты, то сперва проходит список сокетов, на которых
    // ожидается прием данных
    for (SocketList_t::iterator iter = _socketList.begin(); iter != _socketList.end();) {
      if ( FD_ISSET ((*iter).getSocketDescriptor(), &tmp_fd_read) ) {
        (*iter).markAsReadable();
        FD_CLR((*iter).getSocketDescriptor(), &_fd_read);

        Socket readySocket(*iter);
        _socketList.erase(iter++);

        readyForRead.push_front(readySocket); --st;
      } else
        ++iter;
    }
  }

  if ( st > 0 ) {
    // Если после формирования списка сокетов готовых на чтение есть еще 
    // сокеты с активным событием, то формируем список сокетов 
    // готовых на запись
    for (SocketList_t::iterator iter = _writeableSocketList.begin(); iter != _writeableSocketList.end();) {
      if ( FD_ISSET ((*iter).getSocketDescriptor(), &tmp_fd_write) ) {
        (*iter).markAsWriteable();
        FD_CLR((*iter).getSocketDescriptor(), &_fd_write);

        Socket readySocket(*iter);
        _writeableSocketList.erase(iter++);

        readyForWrite.push_front(readySocket);
      } else
        ++iter;
    }
  }
  return HAVE_READY_SOCKET;
}

void
smsc::core_ax::network::SocketPool::setTimeOut(time_t timeOut)
{
  _timeout.tv_sec = timeOut;
}

smsc::core_ax::network::SocketPool::SocketPool(size_t maxSocketsInPool) : _maxSocketsInPool(maxSocketsInPool), _maxFd(0)
{
  FD_ZERO(&_fd_read);
  FD_ZERO(&_fd_write);
  _timeout.tv_sec = 0; _timeout.tv_usec = 0;
}

smsc::core_ax::network::SocketPool::~SocketPool()
{
  _socketList.clear();
}
