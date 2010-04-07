#include <unistd.h>
#include <assert.h>
#include <algorithm>
#include <utility>
#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "logger/Logger.h"

namespace eyeline {
namespace corex {
namespace io {

template <class LOCK>
IOObjectsPool_tmpl<LOCK>::IOObjectsPool_tmpl(int maxPoolSize)
: _pollingIsActive(false) {
  if ( !(_fds = new struct pollfd [maxPoolSize]) ||
       !(_snaphots_fds = new struct pollfd [maxPoolSize]) )
    throw smsc::util::SystemError("IOObjectsPool_tmpl::IOObjectsPool_tmpl::: can't allocate memory for _fds");
  if ( !(_used_fds = new int[maxPoolSize]) )
    throw smsc::util::SystemError("IOObjectsPool_tmpl::IOObjectsPool_tmpl::: can't allocate memory for _used_fds");

  memset(reinterpret_cast<uint8_t*>(_fds), 0, sizeof(struct pollfd) * maxPoolSize);
  memset(reinterpret_cast<uint8_t*>(_snaphots_fds), 0, sizeof(struct pollfd)*maxPoolSize);

  if ( ::pipe(_signallingPipe) < 0)
    throw smsc::util::SystemError("IOObjectsPool_tmpl::IOObjectsPool_tmpl::: call to pipe failed");

  _fds[0].events = POLLRDNORM;
  _fds[0].fd = _signallingPipe[0];

  _maxPoolSize = maxPoolSize;
  _socketsCount = 1;
  for(int i=0; i<_maxPoolSize; ++i)
    _used_fds[i] = -1;
}

template <class LOCK>
int
IOObjectsPool_tmpl<LOCK>::calcPolledFds(int* sockets_count)
{
  int retVal = OK_NO_EVENTS;

  _lock.Lock();

  if ( !_inputEventsReady.empty() )
    retVal = OK_READ_READY;

  if ( !_outputEventsReady.empty() )
    retVal |=  OK_WRITE_READY;

  if ( !_newConnectionEventsReady.empty() )
    retVal |= OK_ACCEPT_READY;

  if ( retVal == OK_NO_EVENTS ) {
    memcpy(reinterpret_cast<uint8_t*>(_snaphots_fds), reinterpret_cast<uint8_t*>(_fds), _maxPoolSize * sizeof(struct pollfd));
    *sockets_count = _socketsCount;
    _pollingIsActive = true;
  }
  _lock.Unlock();

  return retVal;
}

template <class LOCK>
int
IOObjectsPool_tmpl<LOCK>::listen(uint32_t timeout)
{
  int retVal = OK_NO_EVENTS, st=0;
  do {
    int socketsCount=0;
    retVal = calcPolledFds(&socketsCount);
    if ( retVal != OK_NO_EVENTS)
      return retVal;

    if ( timeout )
      st = ::poll(_snaphots_fds, socketsCount, timeout);
    else
      st = ::poll(_snaphots_fds, socketsCount, INFTIM);

    if ( st < 0 ) {
      if ( errno == EINTR )
        throw utilx::InterruptedException("IOObjectsPool_tmpl::listen::: poll() was interrupted");
      else
        throw smsc::util::SystemError("IOObjectsPool_tmpl::listen::: call to poll() failed");
    } else if ( !st )
      return TIMEOUT;

    if ( _snaphots_fds[0].revents & POLLRDNORM ) {
      uint8_t signallingByte;
      read(_signallingPipe[0], &signallingByte, sizeof(signallingByte));
    } else
      break;
  } while(true);

  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  _pollingIsActive = false;
  for (int j=0; j<_socketsCount; ++j) {
    int readyFd;
    if ( (readyFd = _snaphots_fds[j].fd) > -1 && _snaphots_fds[j].revents ) {
      if ( _snaphots_fds[j].revents & POLLNVAL ) {
        in_mask_t::iterator in_iter = _inMask.find(readyFd);
        if ( in_iter != _inMask.end() ) {
          updatePollIndexes(readyFd, POLLRDNORM);
          _inMask.erase(in_iter);
        }
        out_mask_t::iterator out_iter = _outMask.find(readyFd);
        if ( out_iter != _outMask.end() ) {
          updatePollIndexes(readyFd, POLLWRNORM);
          _outMask.erase(out_iter);
        }
        throw PollException("IOObjectsPool_tmpl::listen::: descriptor [=%d] has been closed", readyFd);
      }
      if ( _snaphots_fds[j].revents & POLLRDNORM ) {
        in_mask_t::iterator in_iter = _inMask.find(readyFd);
        if ( in_iter != _inMask.end() )
          _inputEventsReady.insert(std::make_pair(in_iter->first, in_iter->second));
        else {
          accept_mask_t::iterator accept_iter = _acceptMask.find(readyFd);
          if ( accept_iter != _acceptMask.end() )
            _newConnectionEventsReady.push_back(accept_iter->second);
        }
      }
      if ( _snaphots_fds[j].revents & POLLWRNORM ) {
        out_mask_t::iterator out_iter = _outMask.find(readyFd);
        if ( out_iter != _outMask.end() )
          _outputEventsReady.insert(std::make_pair(out_iter->first, out_iter->second));
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

  return retVal;
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::insert(InputStream* iStream)
{
  int fd = iStream->getOwner()->getDescriptor();
  int idx;
  {
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("io");
    smsc_log_debug(logger, "IOObjectsPool_tmpl::insert::: istream, fd=%d", fd);
  }

  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);

  if ( (idx=_used_fds[fd]) == -1 ) {
    if ( _socketsCount == _maxPoolSize )
      throw smsc::util::Exception("IOObjectsPool_tmpl::insert(iStream=%p)::: exceeded max pool size",
                                  iStream);
    _fds[_socketsCount].events = POLLRDNORM;
    _fds[_socketsCount].fd = fd;

    _used_fds[fd] = _socketsCount++;
    _inMask.insert(std::make_pair(fd, iStream));
  } else {
    _fds[idx].events |= POLLRDNORM;
    _inMask.insert(std::make_pair(fd, iStream));
  }
  if ( _pollingIsActive ) {
    uint8_t signallingByte = 0;
    write(_signallingPipe[1], &signallingByte, sizeof(signallingByte));
  }
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::insert(OutputStream* oStream)
{
  int fd = oStream->getOwner()->getDescriptor();
  int idx;
  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( (idx=_used_fds[fd]) == -1 ) {
    if ( _socketsCount == _maxPoolSize )
      throw smsc::util::Exception("IOObjectsPool_tmpl::insert(oStream=%p)::: exceeded max pool size",
                                  oStream);
    _fds[_socketsCount].events = POLLWRNORM;
    _fds[_socketsCount].fd = fd;

    _used_fds[fd] = _socketsCount++;
    _outMask.insert(std::make_pair(fd, oStream));

  } else {
    _fds[idx].events |= POLLWRNORM;
    _outMask.insert(std::make_pair(fd, oStream));
  }
  if ( _pollingIsActive ) {
    uint8_t signallingByte = 0;
    write(_signallingPipe[1], &signallingByte, sizeof(signallingByte));
  }
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::insert(corex::io::network::ServerSocket* socket)
{
  int fd = socket->getDescriptor();

  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( _used_fds[fd] == -1 ) {
    if ( _socketsCount == _maxPoolSize )
      throw smsc::util::Exception("IOObjectsPool_tmpl::insert(serverSocket=%p)::: exceeded max pool size", socket);
    _fds[_socketsCount].fd = fd;
    _fds[_socketsCount].events = POLLRDNORM;
    _used_fds[fd] = _socketsCount++;
    _acceptMask.insert(std::make_pair(fd, socket));
    if ( _pollingIsActive ) {
      uint8_t signallingByte = 0;
      write(_signallingPipe[1], &signallingByte, sizeof(signallingByte));
    }
  }
}

template <class LOCK>
bool
IOObjectsPool_tmpl<LOCK>::updatePollIndexes(int fd, short event)
{
  int idx = _used_fds[fd];

  assert( idx < _maxPoolSize);
  if ( idx < 0 ) return false;

  assert(_socketsCount>=1);

  if ( idx == _socketsCount-1 ) {
    if ( event )
      _fds[idx].events &= ~event;
    else
      _fds[idx].events = 0;
    if ( !_fds[idx].events ) {
      --_socketsCount;
      _used_fds[fd] = -1;
      _fds[idx].fd = -1;
    }
  } else {
    if ( event )
      _fds[idx].events &= ~event;
    else
      _fds[idx].events = 0;
    if ( !_fds[idx].events ) {
      _used_fds[fd] = -1;
      _fds[idx] = _fds[_socketsCount-1];
      _used_fds[_fds[idx].fd] = idx;
      _fds[_socketsCount-1].fd = -1;
      _fds[_socketsCount-1].events = 0;
      --_socketsCount;
    }
  }
  return true;
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::remove(InputStream* iStream)
{
  int fd = iStream->getOwner()->getDescriptor();

  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( !updatePollIndexes(fd, POLLRDNORM) )
    return;

  _inputEventsReady.erase(fd);
  _inMask.erase(fd);
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::remove(OutputStream* oStream)
{
  int fd = oStream->getOwner()->getDescriptor();

  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( !updatePollIndexes(fd, POLLWRNORM) )
    return;

  _outputEventsReady.erase(fd);
  _outMask.erase(fd);
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::remove(IOObject* streamsOwner)
{
  int fd = streamsOwner->getDescriptor();

  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( !updatePollIndexes(fd) )
    return;

  _inputEventsReady.erase(fd);
  _outputEventsReady.erase(fd);
  _inMask.erase(fd);
  _outMask.erase(fd);
}

template <class LOCK>
void
IOObjectsPool_tmpl<LOCK>::remove(corex::io::network::ServerSocket* socket)
{
  int fd = socket->getDescriptor();

  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( !updatePollIndexes(fd) )
    return;

  _acceptMask.erase(fd);
}

template <class LOCK>
OutputStream*
IOObjectsPool_tmpl<LOCK>::getNextReadyOutputStream()
{
  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( _outputEventsReady.empty() ) return NULL;
  else {
    out_events_t::iterator iter = _outputEventsReady.begin();
    OutputStream* oStream = iter->second;
    _outputEventsReady.erase(iter);
    return oStream;
  }
}

template <class LOCK>
InputStream*
IOObjectsPool_tmpl<LOCK>::getNextReadyInputStream()
{
  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( _inputEventsReady.empty() ) return NULL;
  else {
    in_events_t::iterator iter = _inputEventsReady.begin();
    InputStream* iStream = iter->second;
    _inputEventsReady.erase(iter);
    return iStream;
  }
}

template <class LOCK>
corex::io::network::ServerSocket*
IOObjectsPool_tmpl<LOCK>::getNextReadyServerSocket()
{
  smsc::core::synchronization::MutexGuardTmpl<LOCK> guard(_lock);
  if ( _newConnectionEventsReady.empty() ) return NULL;
  else {
    corex::io::network::ServerSocket* socket = _newConnectionEventsReady.front();
    _newConnectionEventsReady.pop_front();
    return socket;
  }
}

}}}
