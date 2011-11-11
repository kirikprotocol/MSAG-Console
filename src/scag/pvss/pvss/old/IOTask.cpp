#include "IOTask.h"
#include "IOTaskManager.h"
#include "informer/io/TmpBuf.h"

namespace scag2 {
namespace pvss  { 

using smsc::core::synchronization::MutexGuard;
using smsc::core::network::Multiplexer;
using smsc::core::network::Socket;
using smsc::logger::Logger;

IOTask::IOTask( uint32_t connectionTimeout,
                uint16_t ioTimeout,
                const char* logName) : 
log_(Logger::getInstance(logName)),
multiplexer_(wakepipe_.getR()),
ioTimeout_(ioTimeout),
connectionTimeout_(connectionTimeout),
checkTimeoutPeriod_(connectionTimeout_ / 10 > 1 ? connectionTimeout_ / 10 : 1),
lastCheckTime_(0)
{
    smsc_log_debug(log_, "%p %s task created", this, logName);
}


int IOTask::Execute() 
{
  Multiplexer::SockArray ready;
  Multiplexer::SockArray error;

  smsc_log_debug(log_, "%p %s started", this, taskName());

  lastCheckTime_ = time(NULL);
  
  for (;;) {

      {
          MutexGuard g(socketMonitor_);
          if ( isStopping ) break;
          if ( ! ( waiting_.Count() || working_.Count() ) ) {

              // smsc_log_debug(log_, "%p idle", this);
              socketMonitor_.wait(500);
              // smsc_log_debug(log_, "%p notified", this);
              continue;
          }

          while (waiting_.Count() ) {
              ConnPtr cx;
              waiting_.Pop(cx);
              multiplexer_.add(cx->getSocket());
              // showmul();
              working_.Push(cx);
          }
      }

      time_t now = checkConnectionTimeout();
      ready.Empty();
      error.Empty();
      MutexGuard mg(socketMonitor_);
      processSockets(ready, error, now);
      while (error.Count()) {
          Socket* s;
          error.Pop(s);
          multiplexer_.remove(s);
          // showmul();
          ConnectionContext* cx = SocketData::getContext(s);
          for ( int i = 0, ie = working_.Count(); i != ie; ++i ) {
              if ( working_[i].get() == cx ) {
                  preDisconnect(cx);
                  working_.Delete(i);
                  smsc_log_debug(log_, "%p: context %p socket %p removed", this, cx, s);
                  break;
              }
          }
      }
  }

  smsc_log_debug(log_, "stopping iotask %s %p ...", taskName(), this);
    {
        MutexGuard g(socketMonitor_);
        multiplexer_.clear();
        // showmul();

        while ( working_.Count() ) {
            ConnPtr cx;
            working_.Pop(cx);
            preDisconnect(cx.get());
            smsc_log_debug(log_, "%p: delete context %p socket %p", this, cx.get(), cx->getSocket());
        }

        while ( waiting_.Count() ) {
            ConnPtr cx;
            waiting_.Pop(cx);
        }

    }
  smsc_log_debug(log_, "%p %s quit", this, taskName());
  return 0;
}


uint32_t IOTask::getSocketsCount() {
    MutexGuard mg(socketMonitor_);
    return waiting_.Count() + working_.Count();
}


void IOTask::registerContext( ConnPtr& cx )
{
    Socket* s = cx->getSocket();
    MutexGuard g(socketMonitor_);
    SocketData::updateTimestamp(s, time(NULL));
    waiting_.Push(cx);
    socketMonitor_.notify();
}


bool IOTask::unregisterContext( ConnPtr& cx )
{
    MutexGuard g(socketMonitor_);
    for ( int i = 0, ie = waiting_.Count(); i != ie; ++i ) {
        if ( waiting_[i] == cx ) {
            waiting_.Delete(i);
            return true;
        }
    }
    for ( int i = 0, ie = working_.Count(); i != ie; ++i ) {
        if ( working_[i] == cx ) {
            multiplexer_.remove( cx->getSocket() );
            // showmul();
            working_.Delete(i);
            preDisconnect( cx.get() );
            return true;
        }
    }
    return false;
}


time_t IOTask::checkConnectionTimeout()
{
    // error.Empty();
    time_t now = time(NULL);
    MutexGuard mg(socketMonitor_);
    if (now - lastCheckTime_ < checkTimeoutPeriod_) {
        return now;
    }
    for (int i = 0, ie = working_.Count(); i != ie; ++i) {
        ConnPtr& cx =  working_[i];
        Socket* s = cx->getSocket();
        const time_t ts = SocketData::getTimestamp(s);
        if ( now - ts >= int(connectionTimeout_) ) {
            smsc_log_warn(log_, "%p: context %p socket %p timeout", this, cx.get(), s);
            multiplexer_.remove(s);
            // showmul();
            preDisconnect( cx.get() );
            working_.Delete(i);
            --i;
            --ie;
        }
    }
    lastCheckTime_ = now;
    return now;
}


void IOTask::stop() {
    smsc_log_info(log_, "IOtask.stop %s %p", taskName(), this);
    // MutexGuard g(socketMonitor_);
    isStopping = true;
    wakepipe_.write("w",1);
    // socketMonitor_.notify();
}


void IOTask::preDisconnect( ConnectionContext* cx )
{
    if (cx) cx->unregisterFromCore();
}


/*
void IOTask::showmul()
{
    eyeline::informer::TmpBuf<char,512> buf;
    char tbuf[30];
    int nsock = 0;
    for ( ; ; ++nsock ) {
        Socket* s = multiplexer_[nsock];
        if (!s) break;
        sprintf(tbuf," %p",s);
        buf.append(tbuf,strlen(tbuf));
    }
    buf.append("",1);
    smsc_log_debug(log_,"IOTask %s %p mul(%u):%s", taskName(), this, nsock, buf.get());
}
 */


void MTPersReader::processSockets(Multiplexer::SockArray &ready,
                                  Multiplexer::SockArray &error,
                                  time_t now)
{
    const int mcr = multiplexer_.canRead(ready, error, ioTimeout_);
    if (!mcr) {
        return;
    } else if (mcr<0) {
        char buf[20];
        wakepipe_.read(buf,sizeof(buf));
    }


    for (int i = 0, ie = ready.Count(); i < ie; ++i) {
        Socket* s = ready[i];
        ConnectionContext* cx = SocketData::getContext(s);
        if (!cx->processReadSocket(now)) {
            // multiplexer_.remove(s);
            // showmul();
            error.Push(s);
        }
    }
}


void MTPersReader::preDisconnect( ConnectionContext* cx )
{
    performance_.inc( cx->getPerfCounter() );
    IOTask::preDisconnect(cx);
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
        ConnPtr& cx = working_[i];
        PerfCounter &pf = cx->getPerfCounter();
        int accepted = pf.getAccepted();
        int processed = pf.getProcessed();
        smsc_log_debug(log_, "context:%p socket:%p current performance %d:%d", cx.get(), cx->getSocket(), accepted, processed);
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
            // multiplexer_.remove(s);
            // showmul();
            error.Push(s);
        }
    }
}

}//pvss
}//scag2

