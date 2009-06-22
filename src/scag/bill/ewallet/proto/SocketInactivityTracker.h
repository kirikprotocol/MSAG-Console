#ifndef SCAG_BILL_EWALLET_PROTO_SOCKETINACTIVITYTRACKER_H
#define SCAG_BILL_EWALLET_PROTO_SOCKETINACTIVITYTRACKER_H

#include <memory>
#include "SocketTask.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class SocketInactivityTracker : public SocketTask
{
public:
    SocketInactivityTracker( Core& core ) :
    SocketTask(core,"ewall.itrk") {}
    
    /*
    bool removeSocket( Socket& socket )
    {
        bool result;
        {
            MutexGuard mg(socketMon_);
            result = sockets_.Delete(&socket);
        }
        if ( result ) {
            socket.detach(taskName());
        }
        return result;
    }
    void registerSocket( Socket& socket )
    {
        MutexGuard mg(socketMon_);
        util::msectime_type* ptr = sockets_.GetPtr(&socket);
        if ( ptr ) *ptr = util::currentTimeMillis();
        else {
            socket.attach(taskName());
            sockets_.Insert( &socket, util::currentTimeMillis() );
        }
    }
     */

    /*
    void updateSocketActivity( Socket& socket )
    {
        MutexGuard mg(socketMon_);
        util::msectime_type* ptr = sockets_.GetPtr(&socket);
        if ( ptr ) *ptr = util::currentTimeMillis();
    }
     */

    /*
    void shutdown()
    {
        if ( !started ) return;
        {
            MutexGuard mg(activity)
        }
    }
     */

protected:
    virtual bool setupSockets( util::msectime_type currentTime );
    virtual bool hasEvents();
    virtual void processEvents();

private:
    // SocketInactivityListener&                 listener_;  // not owned
    // util::msectime_type                       timeout_;

    // smsc::core::synchronization::EventMonitor socketMon_;
    // TimeHash                                  sockets_;
    smsc::core::buffers::Array< Socket* > stalledSockets_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_SOCKETINACTIVITYTRACKER_H */
