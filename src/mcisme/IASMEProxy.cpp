#include <poll.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "IASMEProxy.hpp"
#include "TaskProcessor.h"
#include "mcaia/BusyRequest.hpp"
#include "mcaia/ServerProtocol.hpp"

namespace smsc {
namespace mcisme {

IASMEProxy::IASMEProxy(const char* listening_iface,
                       in_port_t listening_port,
                       TaskProcessor* task_processor,
                       size_t max_queue_sz)
: _eventsQueue(max_queue_sz), _isRunning(true), _logger(logger::Logger::getInstance("iasme")),
  _taskProcessor(task_processor), _socketToPeer(NULL), _checkFdWriteable(false),
  _currentBuf(NULL), _totalWrittenBytes(0)
{
  _listeningSocket = new core::network::Socket();
  _listeningSocket->ReuseAddr();
  _listeningSocket->InitServer(listening_iface, listening_port, 0);
  _listeningSocket->StartServer();

  int fds[2];
  if ( pipe(fds) < 0 )
    throw util::SystemError("IASMEProxy::IASMEProxy::: cal to pipe failed");
  _signallingRdSide = fds[0]; _signallingWrSide = fds[1];
}

int
IASMEProxy::Execute()
{
  while ( _isRunning ) {
    try {
      struct pollfd fds[3];
      nfds_t numOfFds;

      fds[0].events = POLLRDNORM;
      fds[0].fd = _signallingRdSide;

      fds[1].events = POLLRDNORM;
      fds[1].fd = _listeningSocket->getSocket();

      if ( _socketToPeer ) {
        fds[2].events = POLLRDNORM;
        fds[2].fd = _listeningSocket->getSocket();
        if ( _checkFdWriteable )
          fds[2].events |= POLLWRNORM;
        numOfFds = 3;
      } else
        numOfFds = 2;

      int st = ::poll(fds, numOfFds, -1);

      if ( !_isRunning )
        break;

      if ( st < 0 ) {
        smsc_log_error(_logger, "IASMEProxy::Execute::: call to poll failed [%s]", strerror(errno));
      } else {
        if ( fds[0].revents & ( POLLRDNORM | POLLERR ) )
          processScheduledRequest();
        if ( fds[1].revents & POLLRDNORM )
          acceptConnection();
        if ( fds[2].revents & ( POLLRDNORM | POLLERR ) )
          processResponse();
        if ( fds[2].revents & ( POLLWRNORM | POLLERR ) )
          processNextRequest();
      }
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "IASMEProxy::Execute::: caught exception '%s'",
                     ex.what());
    }
  }
  return 0;
}

void
IASMEProxy::processScheduledRequest()
{
  uint8_t signallingByte=0;
  if ( read(_signallingRdSide, &signallingByte, sizeof(signallingByte)) !=
       sizeof(signallingByte) ) {
    smsc_log_error(_logger, "IASMEProxy::sendRequest::: call to read failed: '%s'", strerror(errno));
    return;
  }
  _checkFdWriteable = true;
  if ( !_currentBuf )
    prepareNextBufferForWrite();
}

void
IASMEProxy::prepareNextBufferForWrite()
{
  mcaia::BusyRequest busyReq;
  misscall::MissedCallEvent nextEvent;
  if ( !_eventsQueue.dequeue(nextEvent) ) {
    smsc_log_error(_logger, "IASMEProxy::prepareNextBufferForWrite::: can't get element from queue");
    return;
  }
  busyReq.setCaller(nextEvent.from);
  busyReq.setCalled(nextEvent.to);
  busyReq.setDate(nextEvent.time);
  busyReq.setCause(nextEvent.cause);
  busyReq.setFlags(nextEvent.flags);

  _buf.rewind();
  mcaia::ServerProtocol protocol;
  protocol.encodeMessage(busyReq, &_buf);

  _currentBuf = &_buf; _totalWrittenBytes = 0;
}

void
IASMEProxy::processNextRequest()
{
  if ( _currentBuf ) {
    const char* msgData = _currentBuf->getBuffer();
    size_t bufSz = _currentBuf->getDataWritten();
    int nbytes = _socketToPeer->Write(msgData + _totalWrittenBytes,
                                      static_cast<int>(bufSz - _totalWrittenBytes));
    if ( nbytes < 0 ) {
      smsc_log_error(_logger, "IASMEProxy::processNextRequest::: write to socket failed: %s",
                     strerror(errno));
      _totalWrittenBytes = 0;
      _checkFdWriteable = false;
      _currentBuf = NULL;
      core::synchronization::MutexGuard synchronize(_lock);
      delete _socketToPeer; _socketToPeer = NULL;
      return;
    }
    _totalWrittenBytes += nbytes;
    if ( _totalWrittenBytes == bufSz ) {
      _totalWrittenBytes = 0;
      if ( _eventsQueue.isEmpty() ) {
        _checkFdWriteable = false;
        _currentBuf = NULL;
      } else
        prepareNextBufferForWrite();
    }
  } else
    _checkFdWriteable = false;
}

void
IASMEProxy::acceptConnection()
{
  if ( _socketToPeer ) {
    core::network::Socket* acceptedSocket = _listeningSocket->Accept();
    // terminate accepted connection because of one active connection already exists
    delete acceptedSocket;
  } else {
    core::synchronization::MutexGuard synchronize(_lock);
    _socketToPeer = _listeningSocket->Accept();
  }
}

int
IASMEProxy::readData(char* buf, int bytes_to_read)
{
  int st = _socketToPeer->Read(buf, bytes_to_read);
  if ( st > 0 )
    return st;
  else {
    if ( st < 0 ) {
      smsc_log_error(_logger, "IASMEProxy::readData::: read from socket failed: '%s'",
                     strerror(errno));
    } else
      smsc_log_info(_logger, "IASMEProxy::readData::: connection to server closed by remote side");

    core::synchronization::MutexGuard synchronize(_lock);
    delete _socketToPeer;
    _socketToPeer = NULL;
  }
  return st;
}

void
IASMEProxy::processResponse()
{
  if ( !_messageBodyLen ) {
    int st = readData(_header.buf + _bytesHasBeenRead, sizeof(_header.buf) - _bytesHasBeenRead);
    if ( st <= 0 )
      return;
    _bytesHasBeenRead += st;
    if ( _bytesHasBeenRead == sizeof(_header.buf) ) {
      _messageBodyLen = ntohl(_header.value);
      _bytesHasBeenRead = 0;
    }
  } else {
    int st = readData(_bufferForBody + _bytesHasBeenRead, _messageBodyLen - _bytesHasBeenRead);
    if ( st <= 0 )
      return;
    _bytesHasBeenRead += st;
    if ( _bytesHasBeenRead == _messageBodyLen ) {
      _bytesHasBeenRead = 0;
      mcaia::ServerProtocol protocol;
      protocol.assignHandler(_taskProcessor);
      try {
        protocol.decodeAndHandleMessage(_bufferForBody, static_cast<size_t>(_messageBodyLen));
      } catch (...) {
        _messageBodyLen = 0;
        throw;
      }
      _messageBodyLen = 0;
    }
  }
}

bool
IASMEProxy::sendRequest(const misscall::MissedCallEvent& event)
{
  if ( _eventsQueue.enqueue(event) ) {
    uint8_t signallingByte=0;
    if ( write(_signallingWrSide, &signallingByte, sizeof(signallingByte)) !=
        sizeof(signallingByte) ) {
      smsc_log_error(_logger, "IASMEProxy::sendRequest::: call to write failed");
      return false;
    }
  } else {
    smsc_log_error(_logger, "IASMEProxy::sendRequest::: output events queue is full");
    return false;
  }
  return true;
}

}}
