#include "IOTask.h"
#include "IOTaskManager.h"

namespace scag2 {
namespace pvss  { 

using smsc::core::synchronization::MutexGuard;

IOTask::IOTask(IOTaskManager& iomanager, uint32_t connectionTimeout, uint16_t ioTimeout, const char* logName):iomanager_(iomanager),
               connectionTimeout_(connectionTimeout), ioTimeout_(ioTimeout), lastCheckTime_(0)
{
  logger = Logger::getInstance(logName);
  smsc_log_debug(logger, "%p %s task created", this, logName);
  checkTimeoutPeriod_ = connectionTimeout_ / 10 > 1 ? connectionTimeout_ / 10 : 1;
}

bool IOTask::idle() const {
  return !(itemsCount_ && (waitingAdd_.Count() || multiplexer_.count())) && !isStopping;
}

int IOTask::Execute() {  
  Multiplexer::SockArray ready;
  Multiplexer::SockArray error;

  smsc_log_debug(logger, "%p started", this);

  lastCheckTime_ = time(NULL);
  
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

    time_t now = checkConnectionTimeout(error);

    removeSocket(error);

    processSockets(ready, error, now);

    removeSocket(error);
  }
  smsc_log_debug(logger, "stopping iotask %p ...", this);
  int socketsCount = 0;
  {
    Socket* s;
    ConnectionContext* cx;
    MutexGuard g(socketMonitor_);
    socketsCount = multiplexer_.count();

    while (multiplexer_.count()) {
      s = multiplexer_.get(0);
      smsc_log_debug(logger, "%p: delete socket %p", this, s);
      cx = SocketData::getContext(s);
      multiplexer_._remove(0);
      if (cx->canDelete()) {
        delete cx;
      }
    }
    while(waitingAdd_.Count()) {
      waitingAdd_.Pop(s);
      cx = SocketData::getContext(s);
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

time_t IOTask::checkConnectionTimeout(Multiplexer::SockArray& error) {
  error.Empty();
  time_t now = time(NULL);
  if (now - lastCheckTime_ < checkTimeoutPeriod_) {
    return now;
  }
  for (int i = 0; i < multiplexer_.count(); i++) {
    Socket *s =  multiplexer_.get(i);
    if (isTimedOut(s, now)) {
      smsc_log_warn(logger, "%p: socket %p timeout", this, s);
      error.Push(s);
    }
  }
  lastCheckTime_ = now;
  return now;
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
  smsc_log_debug(logger, "stop iotask %p", this);

  MutexGuard g(socketMonitor_);
  isStopping = true;
  socketMonitor_.notify();
}

const char* MTPersReader::taskName() {
  return "MTPersReader";
}

void MTPersReader::addSocketToMultiplexer(Socket* s) {
  multiplexer_.addR(s);
}

void MTPersReader::processSockets(Multiplexer::SockArray &ready, Multiplexer::SockArray &error, const time_t& now) {
  if (!multiplexer_.canRead(ready, error, ioTimeout_)) {
    return;
  }
  for (int i = 0; i < ready.Count(); ++i) {
    Socket* s = ready[i];
    ConnectionContext* cx = SocketData::getContext(s);
    if (!cx->processReadSocket(now)) {
      error.Push(s);
    }
  }
}

void MTPersReader::disconnectSocket(Socket *s) {
  ConnectionContext* cx = SocketData::getContext(s);
  performance_.inc(cx->getPerfCounter());
  multiplexer_.remove(s);
  smsc_log_debug(logger, "%p: context %p socket %p removed  from multiplexer", this, cx, s);
  if (cx->canFinalize()) {
    smsc_log_debug(logger, "%p: context %p socket %p deleted", this, cx, s);
    delete cx;
  }
}

Performance MTPersReader::getPerformance() {
  MutexGuard mg(socketMonitor_);
  int mpcount = multiplexer_.count();
  Performance perf;
  perf.inc(performance_);
  performance_.reset();
  if (!mpcount) {
    return perf;
  }
  for (int i = 0; i < mpcount; ++i) {
    Socket *s = multiplexer_.get(i);
    ConnectionContext* cx = SocketData::getContext(s);
    PerfCounter &pf = cx->getPerfCounter();
    int accepted = pf.getAccepted();
    int processed = pf.getProcessed();
    smsc_log_info(logger, "context:%p socket:%p current performance %d:%d", cx, s, accepted, processed);
    perf.inc(accepted, processed);
  }
  return perf;
}


const char* MTPersWriter::taskName() {
  return "MTPersWriter";
}

void MTPersWriter::addSocketToMultiplexer(Socket* s) {
  multiplexer_.addW(s);
}

void MTPersWriter::processSockets(Multiplexer::SockArray &ready, Multiplexer::SockArray &error, const time_t& now) {
  if (!multiplexer_.canWrite(ready, error, ioTimeout_)) {
    return;
  }
  for (int i = 0; i < ready.Count(); ++i) {
    Socket* s = ready[i];
    ConnectionContext* cx = SocketData::getContext(s);
    if (!cx->processWriteSocket(now)) {
      error.Push(s);
    }
  }
}

void MTPersWriter::addSocket(Socket* s) {
  MutexGuard g(socketMonitor_);
  waitingAdd_.Push(s);
  socketMonitor_.notify();
}

void MTPersWriter::disconnectSocket(Socket *s) {
  ConnectionContext* cx = SocketData::getContext(s);
  multiplexer_.remove(s);
  //cx->flushLogs();
  smsc_log_debug(logger, "%p: context %p socket %p removed  from multiplexer", this, cx, s);
  if (cx->canFinalize()) {
    smsc_log_debug(logger, "%p: context %p socket %p deleted", this, cx, s);
    delete cx;
  }
}


}//pvss
}//scag2

