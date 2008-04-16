#include <algorithm>
#include <util/Exception.hpp>
#include <sua/utilx/Exception.hpp>
#include <logger/Logger.h>

namespace corex {
namespace io {

template <class LOCK>
IOObjectsPool_tmpl<LOCK>::IOObjectsPool_tmpl(int maxPoolSize) {
  if ( !(_fds = new struct pollfd [maxPoolSize]) ||
       !(_snaphots_fds = new struct pollfd [maxPoolSize]) )
    throw smsc::util::SystemError("IOObjectsPool_tmpl::IOObjectsPool_tmpl::: can't allocate memory for _fds");
  if ( !(_used_fds = new int[maxPoolSize]) )
    throw smsc::util::SystemError("IOObjectsPool_tmpl::IOObjectsPool_tmpl::: can't allocate memory for _used_fds");

  memset(reinterpret_cast<uint8_t*>(_fds), 0, sizeof(struct pollfd) * maxPoolSize);
  memset(reinterpret_cast<uint8_t*>(_snaphots_fds), 0, sizeof(struct pollfd)*maxPoolSize);

  _maxPoolSize = maxPoolSize;
  _socketsCount = 0;
  for(int i=0; i<_maxPoolSize; ++i)
    _used_fds[i] = -1;
}

template <class LOCK>
int
IOObjectsPool_tmpl<LOCK>::listen()
{
  int retVal = OK_NO_EVENTS;

  _lock.Lock();

  if ( !_inputEventsReady.empty() )
    retVal = OK_READ_READY;

  if ( !_outputEventsReady.empty() )
    retVal |=  OK_WRITE_READY;

  if ( !_newConnectionEventsReady.empty() )
    retVal |= OK_ACCEPT_READY;

  if ( retVal ) {
    _lock.Unlock();
    return retVal;
  } else {
    memcpy(reinterpret_cast<uint8_t*>(_snaphots_fds), reinterpret_cast<uint8_t*>(_fds), _maxPoolSize * sizeof(struct pollfd));
    _lock.Unlock();
  }
  int fd, idx=0;

  // REMOVE DEBUG OUTPUT
  //smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("poll");
  //   smsc_log_info(logger, "IOObjectsPool::listen::: call to poll");
  //   errno = 0;
  //   {
  //     char strBuf[1024];
  //     int strBufOffset = 0;
  //     for(int i=0;i<_socketsCount;++i) {
  //       strBufOffset += sprintf(strBuf + strBufOffset, "%d:event=%d,", _snaphots_fds[i].fd, _snaphots_fds[i].events);
  //     }
  //     strBuf[strlen(strBuf)-1]=0;
  //     smsc_log_debug(logger, "IOObjectsPool::listen::: _socketsCount=%d, socketsFdList=[%s]", _socketsCount, strBuf);
  //   }
  // END DEBUG
  int st = ::poll(_snaphots_fds, _socketsCount, INFTIM);
  //smsc_log_info(logger, "IOObjectsPool::listen::: poll returned, st=%d, errno=%d; _socketsCount=%d", st, errno, _socketsCount);
  if ( st < 0 ) {
    if ( errno == EINTR )
      throw utilx::InterruptedException("IOObjectsPool_tmpl::listen::: poll() was interrupted");
    else
      throw smsc::util::SystemError("IOObjectsPool_tmpl::listen::: call to poll() failed");
  }

  smsc::core::synchronization::MutexGuard guard(_lock);
  for (int j=0; j<_socketsCount; ++j) {
    int readyFd;
    //    smsc_log_info(logger, "IOObjectsPool::listen::: check descriptor %d", j);
    if ( (readyFd = _snaphots_fds[j].fd) > -1 && _snaphots_fds[j].revents ) {
      // bypass each ready fd
      if ( _snaphots_fds[j].revents & POLLRDNORM ) {
        in_mask_t::iterator in_iter = _inMask.find(readyFd);
        if ( in_iter != _inMask.end() ) {
          _inputEventsReady.push_back(in_iter->second);
          //          smsc_log_info(logger, "IOObjectsPool::listen::: added input ready event");
        } else {
          accept_mask_t::iterator accept_iter = _acceptMask.find(readyFd);
          if ( accept_iter != _acceptMask.end() ) {
            _newConnectionEventsReady.push_back(accept_iter->second);
            //            smsc_log_info(logger, "IOObjectsPool::listen::: added accept ready event");
          }
        }
      }
      if ( _snaphots_fds[j].revents & POLLWRNORM ) {
        out_mask_t::iterator out_iter = _outMask.find(readyFd);
        if ( out_iter != _outMask.end() ) {
          _outputEventsReady.push_back(out_iter->second);
          //          smsc_log_info(logger, "IOObjectsPool::listen::: added output ready event");
        }
      }
      --st;
    }
    if ( st == 0 ) break;
  }

  if ( !_inputEventsReady.empty() )
    retVal = OK_READ_READY;

  if ( !_outputEventsReady.empty() )
    retVal |= OK_WRITE_READY;

  if ( !_newConnectionEventsReady.empty() )
    retVal |= OK_ACCEPT_READY;
  //  smsc_log_info(logger, "IOObjectsPool::listen::: we have bypassed all events, retVal = 0x%x", retVal);
  return retVal;
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::insert(InputStream* iStream)
{
  int fd = iStream->getOwner()->getDescriptor();
  int idx;
  smsc::core::synchronization::MutexGuard guard(_lock);

  if ( (idx=_used_fds[fd]) == -1 ) {
    if ( _socketsCount == _maxPoolSize )
      throw smsc::util::Exception("IOObjectsPool_tmpl::insert(iStream=%p)::: exceeded max pool size", iStream);
    _fds[_socketsCount].events = POLLRDNORM;
    _fds[_socketsCount].fd = fd;

//     smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("poll");
//     smsc_log_debug(logger, "IOObjectsPool::insert(InputStream)::: insert fd=%d,event=%d", _fds[_socketsCount].fd, _fds[_socketsCount].events);

    _used_fds[fd] = _socketsCount++;
    _inMask.insert(std::make_pair(fd, iStream));
  } else {
    _fds[idx].events |= POLLRDNORM;
    _inMask.insert(std::make_pair(fd, iStream));
  }
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::insert(OutputStream* oStream)
{
  int fd = oStream->getOwner()->getDescriptor();
  int idx;
  smsc::core::synchronization::MutexGuard guard(_lock);
  if ( (idx=_used_fds[fd]) == -1 ) {
    if ( _socketsCount == _maxPoolSize )
      throw smsc::util::Exception("IOObjectsPool_tmpl::insert(oStream=%p)::: exceeded max pool size", oStream);
    _fds[_socketsCount].events = POLLWRNORM;
    _used_fds[fd] = _socketsCount++;
    _outMask.insert(std::make_pair(fd, oStream));
  } else {
    _fds[idx].events |= POLLWRNORM;
    _outMask.insert(std::make_pair(fd, oStream));
  }
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::insert(corex::io::network::ServerSocket* socket)
{
  int fd = socket->getDescriptor();

  smsc::core::synchronization::MutexGuard guard(_lock);
  if ( _used_fds[fd] == -1 ) {
    if ( _socketsCount == _maxPoolSize )
      throw smsc::util::Exception("IOObjectsPool_tmpl::insert(serverSocket=%p)::: exceeded max pool size", socket);
    _fds[_socketsCount].fd = fd;
    _fds[_socketsCount].events = POLLRDNORM;
    _used_fds[fd] = _socketsCount++;
    _acceptMask.insert(std::make_pair(fd, socket));
  }
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::updatePollIndexes(int fd)
{
  int idx = _used_fds[fd];
  if ( idx == _socketsCount-1 ) {
    _fds[idx].fd = -1;
    _fds[idx].events = 0;
    --_socketsCount;
    _used_fds[idx] = -1;
  } else {
    _used_fds[fd] = -1;
    _fds[idx] = _fds[_socketsCount-1];
    _used_fds[_fds[idx].fd] = idx;
    _fds[_socketsCount-1].fd = -1;
    _fds[_socketsCount-1].events = 0;
    --_socketsCount;
  }
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::remove(InputStream* iStream)
{
  int fd = iStream->getOwner()->getDescriptor();

  smsc::core::synchronization::MutexGuard guard(_lock);
  _inMask.erase(fd);
  updatePollIndexes(fd);
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::remove(OutputStream* oStream)
{
  int fd = oStream->getOwner()->getDescriptor();

  smsc::core::synchronization::MutexGuard guard(_lock);
  _outMask.erase(fd);
  updatePollIndexes(fd);
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::remove(IOObject* streamsOwner)
{
  int fd = streamsOwner->getDescriptor();

  smsc::core::synchronization::MutexGuard guard(_lock);
  _inMask.erase(fd);
  _outMask.erase(fd);
  updatePollIndexes(fd);
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::remove(corex::io::network::ServerSocket* socket)
{
  int fd = socket->getDescriptor();

  smsc::core::synchronization::MutexGuard guard(_lock);
  _acceptMask.erase(fd);
  updatePollIndexes(fd);
}

template <class LOCK>
OutputStream*
IOObjectsPool_tmpl<LOCK>::getNextReadyOutputStream()
{
  smsc::core::synchronization::MutexGuard guard(_lock);
  if ( _outputEventsReady.empty() ) return NULL;
  else {
    OutputStream* oStream = _outputEventsReady.front();
    _outputEventsReady.pop_front();
    return oStream;
  }
}

template <class LOCK>
InputStream*
IOObjectsPool_tmpl<LOCK>::getNextReadyInputStream()
{
  smsc::core::synchronization::MutexGuard guard(_lock);
  if ( _inputEventsReady.empty() ) return NULL;
  else {
    InputStream* iStream = _inputEventsReady.front();
    _inputEventsReady.pop_front();
    return iStream;
  }
}

template <class LOCK>
corex::io::network::ServerSocket*
IOObjectsPool_tmpl<LOCK>::getNextReadyServerSocket()
{
  smsc::core::synchronization::MutexGuard guard(_lock);
  if ( _newConnectionEventsReady.empty() ) return NULL;
  else {
    corex::io::network::ServerSocket* socket = _newConnectionEventsReady.front();
    _newConnectionEventsReady.pop_front();
    return socket;
  }
}

}}
