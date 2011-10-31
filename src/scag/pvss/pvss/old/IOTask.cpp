#include "IOTask.h"
#include "IOTaskManager.h"

namespace scag2 {
namespace pvss  { 

using smsc::core::synchronization::MutexGuard;
using smsc::core::network::Multiplexer;
using smsc::core::network::Socket;
using smsc::logger::Logger;

IOTask::IOTask( // IOTaskManager& iomanager,
                uint32_t connectionTimeout,
                uint16_t ioTimeout,
                const char* logName) : 
// iomanager_(iomanager),
log_(Logger::getInstance(logName)),
connectionTimeout_(connectionTimeout),
ioTimeout_(ioTimeout),
lastCheckTime_(0),
checkTimeoutPeriod_(connectionTimeout_ / 10 > 1 ? connectionTimeout_ / 10 : 1)
{
    smsc_log_debug(log_, "%p %s task created", this, logName);
}

/*
bool IOTask::idle() const {
    // return !(itemsCount_ && (waiting_.Count() || multiplexer_.getSize())) && !isStopping;
    return false;
}
 */

int IOTask::Execute() 
{
  Multiplexer::SockArray ready;
  Multiplexer::SockArray error;

  smsc_log_debug(log_, "%p started", this);

  lastCheckTime_ = time(NULL);
  
  for (;;) {

      {
          MutexGuard g(socketMonitor_);
          if ( isStopping ) break;
          if ( ! ( waiting_.Count() || working_.Count() ) ) {

              smsc_log_debug(log_, "%p idle", this);
              socketMonitor_.wait();         
              smsc_log_debug(log_, "%p notified", this);
              continue;
          }

          while (waiting_.Count() ) {
              Socket *s;
              waiting_.Pop(s);
              multiplexer_.add(s);
              working_.Push(s);
          }
      }

      time_t now = checkConnectionTimeout(error);
      if (error.Count()) removeSockets(error);

      processSockets(ready, error, now);
      if (error.Count()) removeSockets(error);
  }

  smsc_log_debug(log_, "stopping iotask %p ...", this);
    // int socketsCount = 0;
    {
        MutexGuard g(socketMonitor_);
        multiplexer_.clear();

        Socket* s;
        while ( working_.Count() ) {
            working_.Pop(s);
            smsc_log_debug(log_, "%p: delete socket %p", this, s);
            ConnectionContext* cx = SocketData::getContext(s);
            if (cx && cx->canDelete()) {
                delete cx;
            }
        }

        while ( waiting_.Count() ) {
            waiting_.Pop(s);
            ConnectionContext* cx = SocketData::getContext(s);
            if (cx && cx->canDelete()) {
                delete cx;
            }
        }

    }
  smsc_log_debug(log_, "%p quit", this);

  return 0;
}


uint32_t IOTask::getSocketsCount() {
    MutexGuard mg(socketMonitor_);
    return waiting_.Count() + working_.Count();
}


void IOTask::registerContext( ConnectionContext* cx )
{
    Socket* s = cx->getSocket();
    MutexGuard g(socketMonitor_);
    SocketData::updateTimestamp(s, time(NULL));
    waiting_.Push(s);
    socketMonitor_.notify();
}


time_t IOTask::checkConnectionTimeout(Multiplexer::SockArray& error) 
{
    error.Empty();
    time_t now = time(NULL);
    if (now - lastCheckTime_ < checkTimeoutPeriod_) {
        return now;
    }
    for (int i = 0; i < working_.Count(); ++i) {
        Socket *s =  working_[i];
        const time_t ts = SocketData::getTimestamp(s);
        if ( now - ts >= int(connectionTimeout_) ) {
            smsc_log_warn(log_, "%p: socket %p timeout", this, s);
            multiplexer_.remove(s);
            error.Push(s);
        }
    }
    lastCheckTime_ = now;
    return now;
}


void IOTask::removeSockets(Multiplexer::SockArray &error) 
{
    MutexGuard g(socketMonitor_);
    while ( error.Count() ) {
        Socket* s;
        error.Pop(s);
        removeSocket(s);
    }
}


void IOTask::removeSocket(Socket *s) 
{
    smsc_log_debug(log_, "%p: socket %p failed", this, s);
    // remove from working
    const int wc = working_.Count();
    for ( int i = 0; i < wc; ++i ) {
        if ( working_[i] == s ) {
            working_.Delete(i);
            break;
        }
    }
    ConnectionContext* cx = SocketData::getContext(s);
    preDisconnect( cx );
    smsc_log_debug(log_, "%p: context %p socket %p removed  from multiplexer", this, cx, s);
    if ( cx->canFinalize() ) {
        smsc_log_debug(log_, "%p: context %p socket %p deleted", this, cx, s);
        delete cx;
    }
}


void IOTask::stop() {
    smsc_log_debug(log_, "stop iotask %p", this);
    MutexGuard g(socketMonitor_);
    isStopping = true;
    socketMonitor_.notify();
}


void MTPersReader::processSockets(Multiplexer::SockArray &ready,
                                  Multiplexer::SockArray &error,
                                  time_t now) {
    if (!multiplexer_.canRead(ready, error, ioTimeout_)) {
        return;
    }

    for (int i = 0; i < ready.Count(); ++i) {
        Socket* s = ready[i];
        ConnectionContext* cx = SocketData::getContext(s);
        if (!cx->processReadSocket(now)) {
            multiplexer_.remove(s);
            error.Push(s);
        }
    }
}


void MTPersReader::preDisconnect( ConnectionContext* cx )
{
    performance_.inc( cx->getPerfCounter() );
}


Performance MTPersReader::getPerformance() 
{
    MutexGuard mg(socketMonitor_);
    Performance perf;
    perf.inc(performance_);
    performance_.reset();
    const int mpcount = working_.Count();
    if (!mpcount) {
        return perf;
    }
    for (int i = 0; i < mpcount; ++i) {
        Socket *s = working_[i];
        ConnectionContext* cx = SocketData::getContext(s);
        PerfCounter &pf = cx->getPerfCounter();
        int accepted = pf.getAccepted();
        int processed = pf.getProcessed();
        smsc_log_debug(log_, "context:%p socket:%p current performance %d:%d", cx, s, accepted, processed);
        perf.inc(accepted, processed);
    }
    perf.connections += mpcount;
    return perf;
}


void MTPersWriter::processSockets(Multiplexer::SockArray &ready,
                                  Multiplexer::SockArray &error,
                                  time_t now) {
    if (!multiplexer_.canWrite(ready, error, ioTimeout_)) {
        return;
    }
    for (int i = 0; i < ready.Count(); ++i) {
        Socket* s = ready[i];
        ConnectionContext* cx = SocketData::getContext(s);
        if (!cx->processWriteSocket(now)) {
            multiplexer_.remove(s);
            error.Push(s);
        }
    }
}

}//pvss
}//scag2

