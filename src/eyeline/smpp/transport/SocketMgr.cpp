#include "SocketMgr.h"
#include "eyeline/smpp/Session.h"
#include "SmppReader.h"
#include "SmppWriter.h"

namespace eyeline {
namespace smpp {

using namespace smsc::core::synchronization;

SocketMgr::SocketMgr( const SocketMgrConfig& config,
                      TimerSubscriber& subscriber,
                      SocketCloser&    closer ) :
log_(smsc::logger::Logger::getInstance("smpp.skmgr")),
config_(config),
subscriber_(subscriber),
socketCloser_(closer),
stopping_(true) 
{
}

SocketMgr::~SocketMgr()
{
    stop();
}


void SocketMgr::start()
{
    if (!stopping_) return;
    MutexGuard mg(mon_);
    if (!stopping_) return;
    smsc_log_debug(log_,"starting...");
    stopping_ = false;
    Start();
}


void SocketMgr::stop()
{
    {
        if (stopping_) return;
        MutexGuard mg(mon_);
        if (stopping_) return;
        smsc_log_debug(log_,"stopping...");
        stopping_ = true;
        mon_.notifyAll();
    }
    tp_.shutdown();
    WaitFor();
}


void SocketMgr::addTimer( Socket& sock, TimerType type )
{
    unsigned delta;
    switch (type) {
    case TIMER_BIND : delta = config_.bindTimeout; break;
    case TIMER_ACTIVITY: delta = config_.inactivityTimeout; break;
    case TIMER_CONNECT : delta = config_.connectRetryInterval; break;
    default: delta = 10*msecPerSec;
    }
    smsc_log_debug(log_,"adding timer +%u %s on sock=%u",
                   delta, timerTypeToString(type), sock.getSocketId());
    const msectime_type now = currentTimeMillis();
    const msectime_type alarm = now + delta;
    MutexGuard mg(mon_);
    if (stopping_) return;
    SocketTimers::iterator i = socketTimers_.insert( std::make_pair(alarm,Timer()) );
    Timer& t = i->second;
    t.socket.reset(&sock);
    t.type   = type;
    // sock->setTimerTime( alarm );
    if (i == socketTimers_.begin()) {
        mon_.notify();
    }
}


void SocketMgr::removeTimers( Socket& socket )
{
    smsc_log_debug(log_,"sock=%u remove timers",socket.getSocketId());
    MutexGuard mg(mon_);
    for ( SocketTimers::iterator i = socketTimers_.begin(),
          ie = socketTimers_.end(); i != ie; ) {
        if ( i->second.socket == &socket ) {
            SocketTimers::iterator todel = i;
            ++i;
            socketTimers_.erase(todel);
        } else {
            ++i;
        }
    }
}


void SocketMgr::addSocket( Socket& socket )
{
    static const size_t sockPerReader = 3;
    static const size_t sockIncrement = 3;

    MutexGuard mg(mon_);
    if (stopping_) return;
    size_t ridx = 0;
    if ( !readers_.empty() ) {
        size_t nsock = readers_[ridx]->getSocketCount();
        for ( size_t i = 1; i < readers_.size(); ++i ) {
            const size_t n = readers_[i]->getSocketCount();
            if ( n < nsock ) {
                nsock = n;
                ridx = i;
            }
        }
    }

    const size_t maxsock = readers_.size() / sockIncrement + sockPerReader;

    if ( ridx >= readers_.size() ||
         readers_[ridx]->getSocketCount() >= maxsock ) {
        // need a new reader
        smsc_log_debug(log_,"new reader needed");
        readers_.push_back( SmppIOTaskPtr(new SmppReader(socketCloser_,
                                                         config_.lastActivityTimeout)));
        writers_.push_back( SmppIOTaskPtr(new SmppWriter(socketCloser_)) );
        tp_.startTask( readers_.back().get() );
        tp_.startTask( writers_.back().get() );
        ridx = readers_.size()-1;
    } else {
        smsc_log_debug(log_,"using old reader %u",ridx);
    }
    // socket.setWriter(*writers_[ridx]);
    readers_[ridx]->addSocket(socket);
    writers_[ridx]->addSocket(socket);
}


int SocketMgr::Execute()
{
    SocketTimers elapsed;
    while (true) {
        msectime_type now;
        {
            MutexGuard mg(mon_);
            now = currentTimeMillis();
            if (stopping_) {
                break;
            }
            SocketTimers::iterator i = socketTimers_.upper_bound(now);
            SocketTimers(socketTimers_.begin(),i).swap(elapsed);
            socketTimers_.erase(socketTimers_.begin(),i);
            if ( elapsed.empty() ) {
                int wait = msecPerSec;
                if (!socketTimers_.empty()) {
                    wait = int(socketTimers_.begin()->first - now);
                }
                if ( wait > int(msecPerSec) ) {
                    wait = msecPerSec;
                }
                mon_.wait(wait);
                continue;
            }
        }

        // process elapsed timers
        for ( SocketTimers::iterator i = elapsed.begin(); i != elapsed.end(); ++i ) {
            try {
                subscriber_.timerElapsed(now,*i->second.socket,i->second.type);
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"timerElapsed %s on sock=%u exc: %s",
                              timerTypeToString(i->second.type),
                              i->second.socket->getSocketId(), e.what());
            }
        }

    }

    elapsed.clear();
    {
        MutexGuard mg(mon_);
        elapsed.swap(socketTimers_);
    }
    return 0;
}

}
}
