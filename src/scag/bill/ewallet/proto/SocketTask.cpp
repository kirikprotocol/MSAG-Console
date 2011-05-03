#include "SocketTask.h"

using smsc::core::synchronization::MutexGuard;

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

void SocketTask::shutdown()
{
    if ( isStopping ) return;
    smsc_log_info(log_,"shutting down...");
    stop();
    wakeup();
}


void SocketTask::registerSocket( Socket& socket )
{
    smsc_log_debug(log_,"registering socket %p",&socket);
    {
        MutexGuard mg(mon_);
        for ( int i = 0; i < sockets_.Count(); ++i ) {
            if ( sockets_[i] == &socket ) return;
        }
        sockets_.Push( &socket );
        attachSocket( socket );
        mon_.notify();
    }
}


void SocketTask::unregisterSocket( Socket& socket )
{
    bool found = false;
    {
        MutexGuard mg(mon_);
        for ( int i = 0; i < sockets_.Count(); ++i ) {
            if ( sockets_[i] == &socket ) {
                sockets_.Delete(i);
                found = true;
                break;
            }
        }
    }
    if ( found ) {
        detachSocket(socket);
        smsc_log_debug(log_,"socket %p unregistered",&socket);
    }
}


void SocketTask::wakeup()
{
    MutexGuard mg(mon_);
    mon_.notify();
}


size_t SocketTask::sockets() const {
    MutexGuard mg(mon_);
    return sockets_.Count();
}


int SocketTask::doExecute()
{
    smsc_log_info(log_,"Starting %s", taskName());

    while ( ! isStopping ) {
        // smsc_log_debug(log_,"cycling %s",taskName());
        try {
            util::msectime_type currentTime = util::currentTimeMillis();
            wakeupTime_ = currentTime + 200;
            bool ok = false;
            {
                MutexGuard mg(mon_);
                ok = setupSockets(currentTime);
                if ( ! ok ) setupFailed(currentTime);
            }
            if ( ok && hasEvents() ) {
                processEvents();
            }
            postProcess();

        } catch ( std::exception& e ) {
            smsc_log_warn( log_, "caught exception in %s: %s", taskName(), e.what() );
        }
    }
    smsc_log_info(log_, "%s left the main loop", taskName() );
    {
        MutexGuard mg(mon_);
        while ( sockets_.Count() > 0 ) {
            Socket& socket = *sockets_[0];
            sockets_.Delete(0);
            detachSocket(socket);
        }
    }
    smsc_log_info(log_, "%s::Execute finished", taskName() );
    return 0;
}


void SocketTask::setupFailed( util::msectime_type currentTime )
{
    if ( wakeupTime_ > currentTime ) {
        mon_.wait( wakeupTime_ - currentTime );
    }
}


void SocketTask::attachSocket( Socket& socket )
{
    socket.attach( taskName() );
}


void SocketTask::detachSocket( Socket& socket )
{
    socket.detach( taskName() );
}

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
