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
  _buf(1024), _currentBuf(NULL), _totalWrittenBytes(0), _bytesHasBeenRead(0), _messageBodyLen(0)
{
  _listeningSocket = new core::network::Socket();
  errno = 0;
  if ( _listeningSocket->InitServer(listening_iface, listening_port, 0, 1, 1) < 0 )
    throw util::Exception("IASMEProxy::IASMEProxy::: InitServer failed - %s",
                          strerror(errno));
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
      struct pollfd fds[3] = {0};
      nfds_t numOfFds;

      fds[0].events = POLLRDNORM;
      fds[0].fd = _signallingRdSide;

      fds[1].events = POLLRDNORM;
      fds[1].fd = _listeningSocket->getSocket();

      if ( _socketToPeer ) {
        fds[2].events = POLLRDNORM;
        fds[2].fd = _socketToPeer->getSocket();
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
        if ( !_isRunning )
          break;
        if ( fds[2].revents & ( POLLRDNORM | POLLERR ) )
          processResponse();
        if ( fds[2].revents & ( POLLWRNORM | POLLERR ) )
          processNextRequest();
        if ( fds[1].revents & POLLRDNORM )
          acceptConnection();
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

  smsc_log_debug(_logger, "IASMEProxy::prepareNextBufferForWrite::: got event='%s' from queue",
                 nextEvent.toString().c_str());
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
    smsc_log_debug(_logger, "IASMEProxy::processNextRequest::: %u bytes has been written to socket, rest bytes count=%u",
                   nbytes, bufSz - _totalWrittenBytes);

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
    errno = 0;
    core::synchronization::MutexGuard synchronize(_lock);
    _socketToPeer = _listeningSocket->Accept();
    if ( !_socketToPeer ) {
      smsc_log_error(_logger, "IASMEProxy::acceptConnection::: can't accept connection - '%s'",
                     strerror(errno));
      return;
    }
    char peerAddr[1024];
    _socketToPeer->GetPeer(peerAddr);
    smsc_log_info(_logger, "IASMEProxy::acceptConnection::: accepted connection from peer=%s",
                  peerAddr);
  }
}

int
IASMEProxy::readData(char* buf, int bytes_to_read)
{
  int st = _socketToPeer->Read(buf, bytes_to_read);
  if ( st <= 0 ) {
    if ( st < 0 )
      smsc_log_error(_logger, "IASMEProxy::readData::: read from socket failed: '%s'",
                     strerror(errno));
    else
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
  smsc_log_debug(_logger, "IASMEProxy::processResponse::: enter it");
  if ( !_messageBodyLen ) {
    int st = readData(_header.buf + _bytesHasBeenRead, sizeof(_header.buf) - _bytesHasBeenRead);
    smsc_log_debug(_logger, "IASMEProxy::processResponse::: readData returned, st = %d", st);
    if ( st <= 0 ) {
      _bytesHasBeenRead = 0; _messageBodyLen = 0;
      return;
    }
    _bytesHasBeenRead += st;
    if ( _bytesHasBeenRead == sizeof(_header.buf) ) {
      _messageBodyLen = ntohl(_header.value);
      smsc_log_debug(_logger, "IASMEProxy::processResponse::: length prefix=%u", _messageBodyLen);
      _bytesHasBeenRead = 0;
    }
  } else {
    int st = readData(_bufferForBody + _bytesHasBeenRead, _messageBodyLen - _bytesHasBeenRead);
    smsc_log_debug(_logger, "IASMEProxy::processResponse::: try read body, st = %d, expected messageBodyLen=%d", st, _messageBodyLen); 
    if ( st <= 0 ) {
      _bytesHasBeenRead = 0; _messageBodyLen = 0;
      return;
    }
    _bytesHasBeenRead += st;
    smsc_log_debug(_logger, "IASMEProxy::processResponse::: _bytesHasBeenRead=%d", _bytesHasBeenRead);
    if ( _bytesHasBeenRead == _messageBodyLen ) {
      uint32_t messageBodyLen  = _messageBodyLen;
      _bytesHasBeenRead = 0;
      _messageBodyLen = 0;
      smsc_log_debug(_logger, "IASMEProxy::processResponse::: read total response message");
      mcaia::ServerProtocol protocol;
      protocol.assignHandler(_taskProcessor);
      protocol.decodeAndHandleMessage(_bufferForBody, static_cast<size_t>(messageBodyLen));
    }
  }
}

bool
IASMEProxy::sendRequest(const misscall::MissedCallEvent& event)
{
  smsc_log_debug(_logger, "IASMEProxy::sendRequest::: schedule event='%s' for processing",
                 event.toString().c_str());
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
