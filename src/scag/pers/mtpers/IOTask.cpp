#include "IOTask.h"
#include "IOTaskManager.h"

namespace scag { namespace mtpers { 

using smsc::core::synchronization::MutexGuard;

IOTask::IOTask(IOTaskManager& iomanager, uint16_t connectionTimeout, const char* logName):iomanager_(iomanager),
               connectionTimeout_(connectionTimeout)
{
  logger = Logger::getInstance(logName);
  smsc_log_debug(logger, "%p %s task created", this, logName);
}

bool IOTask::idle() const {
  return !(itemsCount_ && (waitingAdd_.Count() || multiplexer_.count())) && !isStopping;
}

int IOTask::Execute() {  
  Multiplexer::SockArray ready;
  Multiplexer::SockArray error;

  smsc_log_debug(logger, "%p started", this);

  for (;;) {
    {
      MutexGuard g(socketMonitor_);

      while (idle()) {
        smsc_log_debug(logger, "%p idle", this);
        socketMonitor_.wait();         
        smsc_log_debug(logger, "%p notified", this);
      }
      if (isStopping) {
        break;
      }

      while(waitingAdd_.Count()) {
        Socket *s;
        waitingAdd_.Pop(s);
        addSocketToMultiplexer(s);
      }
    }

    checkConnectionTimeout(error);

    removeSocket(error);

    processSockets(ready, error);

    removeSocket(error);
  }
  {
    Socket* s;
    ConnectionContext* cx;
    MutexGuard g(socketMonitor_);

    while (multiplexer_.count()) {
      s = multiplexer_.get(0);
      smsc_log_debug(logger, "%p: delete socket %p", this, s);
      cx = SocketData::getContext(s);
      multiplexer_._remove(0);
      if (cx->canDelete()) {
        delete cx;
      }
    }
  }

  smsc_log_debug(logger, "%p quit", this);

  return 0;
}

uint32_t IOTask::getSocketsCount() const {
  return itemsCount_;
}

void IOTask::addSocket(Socket* s) {
  MutexGuard g(socketMonitor_);

  SocketData::updateTimestamp(s, time(NULL));
  waitingAdd_.Push(s);
   
  socketMonitor_.notify();
}

void IOTask::registerContext(ConnectionContext* cx) {
  ++itemsCount_;
  addSocket(cx->getSocket());
}

void IOTask::checkConnectionTimeout(Multiplexer::SockArray& error) {
  error.Empty();
  time_t now = time(NULL);
  for (int i = 0; i < multiplexer_.count(); i++) {
    Socket *s =  multiplexer_.get(i);
    if (isTimedOut(s, now)) {
      smsc_log_warn(logger, "%p: socket %p timeout", this, s);
      error.Push(s);
    }
  }
}

inline bool IOTask::isTimedOut(Socket* s, time_t now) {
  return now - SocketData::getTimestamp(s) >= connectionTimeout_;
}

void IOTask::removeSocket(Multiplexer::SockArray &error) {
  if (!error.Count()) {
    return;
  }
  Socket* s;
  uint16_t contextsCount = error.Count();
  {
    MutexGuard g(socketMonitor_);
    while (error.Count()) {
      error.Pop(s);
      smsc_log_debug(logger, "%p: socket %p failed", this, s);
      disconnectSocket(s);
    }
  }
  iomanager_.removeContext(this, contextsCount);
}

void IOTask::disconnectSocket(Socket *s) {
  ConnectionContext* cx = SocketData::getContext(s);
  multiplexer_.remove(s);
  smsc_log_debug(logger, "%p: context %p socket %p removed  from multiplexer", this, cx, s);
  if (cx->canFinalize()) {
    smsc_log_debug(logger, "%p: context %p socket %p deleted", this, cx, s);
    delete cx;
  }
}

void IOTask::removeSocket(Socket *s) {
  {
    MutexGuard g(socketMonitor_);
    disconnectSocket(s);
  }
  iomanager_.removeContext(this);
}

void IOTask::removeSocketFromMultiplexer(Socket* s) {
  MutexGuard g(socketMonitor_);
  multiplexer_.remove(s);
}

void IOTask::stop() {
  isStopping = true;

  MutexGuard g(socketMonitor_);
  socketMonitor_.notify();
}

const char* MTPersReader::taskName() {
  return "MTPersReader";
}

void MTPersReader::addSocketToMultiplexer(Socket* s) {
  multiplexer_.addR(s);
}

void MTPersReader::processSockets(Multiplexer::SockArray &ready, Multiplexer::SockArray &error) {
  if (!multiplexer_.canRead(ready, error, connectionTimeout_)) {
    return;
  }
  for (int i = 0; i < ready.Count(); ++i) {
    Socket* s = ready[i];
    ConnectionContext* cx = SocketData::getContext(s);
    if (!cx->processReadSocket()) {
      error.Push(s);
    }
  }
}

const char* MTPersWriter::taskName() {
  return "MTPersWriter";
}

void MTPersWriter::addSocketToMultiplexer(Socket* s) {
  multiplexer_.addW(s);
}

void MTPersWriter::processSockets(Multiplexer::SockArray &ready, Multiplexer::SockArray &error) {
  if (!multiplexer_.canWrite(ready, error, connectionTimeout_)) {
    return;
  }
  for (int i = 0; i < ready.Count(); ++i) {
    Socket* s = ready[i];
    ConnectionContext* cx = SocketData::getContext(s);
    if (!cx->processWriteSocket()) {
      error.Push(s);
    }
  }
}

}//mtpers
}//scag

