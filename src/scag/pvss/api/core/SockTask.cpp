#include "SockTask.h"
#include "PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {

int SockTask::doExecute()
{
    smsc_log_info(log_,"Starting %s", taskName());

    while ( ! isStopping ) {
        // smsc_log_debug(log_,"cycling %s",taskName());
        try {

            util::msectime_type currentTime = util::currentTimeMillis();

            // collect pending sockets
            processPending();

            int tmo = setupSockets( currentTime );
            if ( tmo > 0 ) {
                // setup failed, need to wait
                smsc::core::synchronization::MutexGuard mg(mon_);

                // fix timeout if necessary
                tmo = setupFailed( tmo );
                if ( tmo > 0 ) {
                    if ( tmo > 1000 ) tmo = 1000;
                    mon_.wait(tmo);
                }
                continue;

            }

            if ( hasEvents() ) {
                processEvents();
            }
            postProcess();

        } catch ( std::exception& e ) {
            smsc_log_warn( log_, "caught exception in %s: %s", taskName(), e.what() );
        }
    }
    smsc_log_info(log_, "%s left the main loop", taskName() );
    processPending();
    for ( ConnArray::iterator i = workingSockets_.begin(), ie = workingSockets_.end(); i != ie; ++i ) {
        detachFromSocket(**i);
    }
    smsc_log_info(log_, "%s::Execute finished", taskName() );
    return 0;
}


void SockTask::registerChannel( PvssSocketPtr& con )
{
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        if ( isStopping ) return;
//        smsc_log_debug(log_,"registering channel %p",&con);
        pendingSockets_.push_back(con);
        mon_.notify();
    }
}


void SockTask::unregisterChannel( PvssSocket& sock )
{
//    smsc_log_debug(log_,"unregistering channel %p sock=%p",&sock,sock.getSocket());
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        if ( isStopping ) {
            smsc_log_warn(log_,"cannot unregister channel %p sock=%p at stop",&sock,sock.getSocket());
            return;
        }
        unpendingSockets_.push_back(PvssSocketPtr(&sock));
        mon_.notify();
    }
}


void SockTask::processPending()
{
    ConnArray work;
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        for ( ConnArray::iterator ps = pendingSockets_.begin(), pe = pendingSockets_.end();
              ps != pe; ++ps ) {
            if ( std::find(workingSockets_.begin(),
                           workingSockets_.end(),
                           *ps) == workingSockets_.end() ) {
                // not found
                work.push_back(*ps);
                mon_.notify();
            }
        }
        pendingSockets_.clear();
    }
    workingSockets_.reserve( workingSockets_.size() + work.size() );
    for ( ConnArray::iterator i = work.begin(), ie = work.end(); i != ie; ++i ) {
        workingSockets_.push_back(*i);
        attachToSocket(**i);
//        smsc_log_debug(log_,"channel %p sock=%p registered",i->get(),(*i)->getSocket());
    }
    work.clear();
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        for ( ConnArray::iterator i = unpendingSockets_.begin(), ie = unpendingSockets_.end();
              i != ie; ++i ) {
            ConnArray::iterator found = std::find( workingSockets_.begin(),
                                                   workingSockets_.end(),
                                                   *i );
            if ( found != workingSockets_.end() ) {
                work.push_back(*found);
                workingSockets_.erase(found);
            }
        }
        unpendingSockets_.clear();
    }
    for ( ConnArray::iterator i = work.begin(), ie = work.end(); i != ie; ++i ) {
        detachFromSocket(**i);
//        smsc_log_debug(log_,"channel %p sock=%p unregistered",i->get(),(*i)->getSocket());
    }
}

} // namespace core
} // namespace pvss
} // namespace scag2
