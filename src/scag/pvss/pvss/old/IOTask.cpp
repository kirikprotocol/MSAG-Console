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

      // smsc_log_debug(log_, "%p %s tick", this, taskName());
      {
          MutexGuard g(socketMonitor_);
          if ( isStopping ) break;
          if ( waiting_.empty() &&
               unwaiting_.empty() &&
               !hasExtraEvents() ) {
              socketMonitor_.wait(300);
              continue;
          }

          if ( !waiting_.empty() ) {
              for ( ConnArray::iterator i = waiting_.begin(), ie = waiting_.end();
                    i != ie; ++i ) {
                  working_.push_back(*i);
                  multiplexer_.add(i->get()->getSocket());
                  // showmul();
              }
              waiting_.clear();
          }

          if ( !unwaiting_.empty() ) {
              for ( ConnArray::iterator i = unwaiting_.begin(), ie = unwaiting_.end();
                    i != ie; ++i ) {
                  multiplexer_.remove(i->get()->getSocket());
                  removeWorking(i->get());
              }
              unwaiting_.clear();
          }

      }

      time_t now = checkConnectionTimeout();
      ready.Empty();
      error.Empty();
      processSockets(ready, error, now);
      while (error.Count()) {
          Socket* s;
          error.Pop(s);
          multiplexer_.remove(s);
          // showmul();
          ConnectionContext* cx = SocketData::getContext(s);
          removeWorking(cx);
      }
  }

  smsc_log_debug(log_, "stopping iotask %s %p ...", taskName(), this);

    multiplexer_.clear();
    // showmul();

    {
        MutexGuard mg(socketMonitor_);
        waiting_.clear();
        unwaiting_.clear();
    }

    for ( ConnArray::iterator i = working_.begin(), ie = working_.end();
          i != ie; ++i ) {
        preDisconnect(i->get());
        smsc_log_debug(log_, "%p: delete context %p socket %p", this, i->get(), i->get()->getSocket());
    }
    working_.clear();

  smsc_log_debug(log_, "%p %s quit", this, taskName());
  return 0;
}


uint32_t IOTask::getSocketsCount() {
    MutexGuard mg(socketMonitor_);
    return waiting_.size() + working_.size();
}


void IOTask::registerContext( ConnPtr& cx )
{
    Socket* s = cx->getSocket();
    {
        MutexGuard g(socketMonitor_);
        SocketData::updateTimestamp(s, time(NULL));
        waiting_.push_back(cx);
        wakepipe_.write("w",1);
        socketMonitor_.notify();
    }
    smsc_log_debug(log_,"context %p sock=%p registered",cx.get(),s);
}


void IOTask::unregisterContext( ConnPtr& cx )
{
    {
        MutexGuard g(socketMonitor_);
        unwaiting_.push_back(cx);
        wakepipe_.write("w",1);
        socketMonitor_.notify();
    }
    smsc_log_debug(log_,"context %p sock=%p unregistered",cx.get(),cx->getSocket());
}
/*
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
 */


time_t IOTask::checkConnectionTimeout()
{
    // error.Empty();
    time_t now = time(NULL);
    if (now - lastCheckTime_ < checkTimeoutPeriod_) {
        return now;
    }
    for ( size_t i = 0, ie = working_.size(); i < ie; ++i) {
        ConnPtr& cx =  working_[i];
        Socket* s = cx->getSocket();
        const time_t ts = SocketData::getTimestamp(s);
        if ( now - ts >= int(connectionTimeout_) ) {
            smsc_log_warn(log_, "%p: context %p socket %p timeout", this, cx.get(), s);
            multiplexer_.remove(s);
            // showmul();
            preDisconnect( cx.get() );
            working_.erase( working_.begin() + i );
            --i;
            --ie;
        }
    }
    lastCheckTime_ = now;
    return now;
}



void IOTask::removeWorking( ConnectionContext* cx )
{
    for ( ConnArray::iterator i = working_.begin(), ie = working_.end();
          i != ie; ++i ) {
        if ( i->get() == cx ) {
            preDisconnect(cx);
            smsc_log_debug(log_, "%p: context %p socket %p removed",
                           this, cx, cx->getSocket());
            working_.erase(i);
            break;
        }
    }
}


void IOTask::stop() {
    smsc_log_info(log_, "IOtask.stop %s %p", taskName(), this);
    MutexGuard g(socketMonitor_);
    isStopping = true;
    wakepipe_.write("w",1);
    socketMonitor_.notify();
}


void IOTask::preDisconnect( ConnectionContext* cx )
{
    if (cx) {
        smsc_log_debug(log_,"predisconnect %p sock=%p",cx,cx->getSocket());
        cx->unregisterFromCore();
    }
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


void MTPersReader::registerContext( ConnPtr& ptr )
{
    {
        MutexGuard mg(perfLock_);
        if ( std::find(perfSockets_.begin(),
                       perfSockets_.end(),
                       ptr) != perfSockets_.end() ) {
            perfSockets_.push_back(ptr);
        }
    }
    IOTask::registerContext(ptr);
}


void MTPersReader::preDisconnect( ConnectionContext* cx )
{
    {
        MutexGuard mg(perfLock_);
        performance_.inc( cx->getPerfCounter() );
        ConnArray::iterator i = std::find( perfSockets_.begin(),
                                           perfSockets_.end(),
                                           cx );
        if ( i != perfSockets_.end() ) {
            perfSockets_.erase(i);
        }
    }
    IOTask::preDisconnect(cx);
}


Performance MTPersReader::getPerformance() 
{
    Performance perf;
    MutexGuard mg(perfLock_);
    perf.inc(performance_);
    performance_.reset();
    if ( perfSockets_.empty() ) {
        return perf;
    }
    for ( ConnArray::iterator i = perfSockets_.begin(), ie = perfSockets_.end();
          i != ie; ++i ) {
        PerfCounter &pf = i->get()->getPerfCounter();
        const int accepted = pf.getAccepted();
        const int processed = pf.getProcessed();
        smsc_log_debug(log_, "context:%p socket:%p current performance %d:%d", i->get(), i->get()->getSocket(), accepted, processed);
        perf.inc(accepted, processed);
    }
    perf.connections += perfSockets_.size();
    return perf;
}


void MTPersWriter::processSockets(Multiplexer::SockArray &ready,
                                  Multiplexer::SockArray &error,
                                  time_t now) {
    // packetIsReady_ = false;
    const int mcr = multiplexer_.canWrite(ready, error, ioTimeout_);
    if (!mcr) {
        return;
    } else if (mcr<0) {
        char buf[20];
        wakepipe_.read(buf,sizeof(buf));
    }

    for (int i = 0, ie = ready.Count(); i < ie; ++i) {
        Socket* s = ready[i];
        ConnectionContext* cx = SocketData::getContext(s);
        if (!cx->processWriteSocket(now)) {
            // multiplexer_.remove(s);
            // showmul();
            error.Push(s);
        }
    }
}


void MTPersWriter::packetIsReady()
{
    MutexGuard mg(socketMonitor_);
    packetIsReady_ = true;
    socketMonitor_.notify();
}


// invoked with socketMonitor_ locked
bool MTPersWriter::hasExtraEvents()
{
    if (packetIsReady_) {
        packetIsReady_ = false;
        return true;
    }
    return false;
}


void MTPersWriter::registerContext( ConnPtr& cx )
{
    try {
        cx->setWriter( this );
        IOTask::registerContext(cx);
    } catch (std::exception& e) {
        smsc_log_error(log_,"cannot register channel %p sock=%p: %s",
                       cx.get(),cx->getSocket(), e.what());
        cx->setWriter(0);
    } catch (...) {
        smsc_log_error(log_,"cannot register channel %p sock=%p: unknown exc",
                       cx.get(),cx->getSocket());
        cx->setWriter(0);
    }
}


void MTPersWriter::preDisconnect( ConnectionContext* cx )
{
    cx->setWriter( 0 );
    IOTask::preDisconnect(cx);
}


}//pvss
}//scag2

