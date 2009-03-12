#include "SockTask.h"
#include "PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {

int SockTask::Execute()
{
    smsc_log_info(log_,"Starting %s", taskName());

    while ( ! isStopping ) {
        // smsc_log_debug(log_,"cycling %s",taskName());
        try {
            util::msectime_type currentTime = util::currentTimeMillis();
            wakeupTime_ = currentTime + 200;
            {
                MutexGuard mg(mon_);
                if ( ! setupSockets(currentTime) ) {
                    setupFailed(currentTime);
                    continue;
                }
            }

            if ( hasEvents() ) {
                processEvents();
            }
            postProcess();

        } catch ( std::exception& e ) {
            smsc_log_warn( log_, "caught exception in %s: %s", taskName(), e.what() );
        }
    }
    {
        MutexGuard mg(mon_);
        while ( sockets_.Count() > 0 ) {
            PvssSocket& channel = *sockets_[0];
            sockets_.Delete(0);
            detachFromSocket(channel);
        }
    }
    smsc_log_info(log_, "%s finished", taskName() );
    return 0;
}


void SockTask::registerChannel( PvssSocket& con )
{
    smsc_log_debug(log_,"registering channel %p",&con);
    {
        MutexGuard mg(mon_);
        for ( int i = 0; i < sockets_.Count(); ++i ) {
            if ( sockets_[i] == &con ) return;
        }
        sockets_.Push( &con );
        mon_.notify();
    }
    attachToSocket(con);
}


void SockTask::unregisterChannel( PvssSocket& sock )
{
    bool found = false;
    {
        MutexGuard mg(mon_);
        for ( int i = 0; i < sockets_.Count(); ++i ) {
            if ( sockets_[i] == &sock ) {
                sockets_.Delete(i);
                found = true;
                break;
            }
        }
    }
    if ( found ) {
        detachFromSocket(sock);
        smsc_log_debug(log_,"channel %p unregistered",&sock);
    }
}


void SockTask::setupFailed(util::msectime_type currentTime)
{
    if ( wakeupTime_ > currentTime ) {
        waiting_ = true;
        mon_.wait( wakeupTime_ - currentTime );
        waiting_ = false;
    }
}


} // namespace core
} // namespace pvss
} // namespace scag2
