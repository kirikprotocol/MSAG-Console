#include "PvssConnTask.h"
#include "PvssConnection.h"

using namespace smsc::core::synchronization;

namespace scag2 {
namespace pvss {
namespace client {

void PvssConnTask::addConnection( PvssConnection& con )
{
    MutexGuard mg(mon_);
    for ( int i = 0; i < sockets_.Count(); ++i ) {
        if ( sockets_[i] == &con ) return;
    }
    sockets_.Push( &con );
    // socketsChanged_ = true;
    mon_.notify();
}


int PvssConnTask::Execute()
{
    smsc_log_debug(log_,"Starting %s", taskName());

    while ( ! isStopping ) {
        // smsc_log_debug(log_,"%s rolling", taskName());
        try {
            if ( ! setupSockets() ) {
                setupFailed();
                continue;
            }

            if ( hasEvents() ) {
                processEvents();
            }
            postProcess();

        } catch ( std::exception& e ) {
            smsc_log_warn( log_, "exception in %s: %s", taskName(), e.what() );
        }
    }
    smsc_log_debug(log_, "%s finished", taskName() );
    return 0;
}

} // namespace client
} // namespace pvss
} // namespace scag2
