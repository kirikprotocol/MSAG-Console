#ifndef SCAG_BILL_EWALLET_PROTO_SOCKETWRITER_H
#define SCAG_BILL_EWALLET_PROTO_SOCKETWRITER_H

#include "IOSocketTask.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class SocketWriter : public IOSocketTask
{
public:
    SocketWriter(Core& core) : IOSocketTask(core,"ewall.writ"), writePending_(false) {}

    /*
    virtual void shutdown() {
        IOSocketTask::Task::shutdown();
        waitUntilReleased();
    }
     */

    void writePending();

protected:
    virtual bool setupSocket( Socket& socket, util::msectime_type currentTime );
    virtual void setupFailed( util::msectime_type currentTime );
    virtual bool hasEvents();
    virtual void process( Socket& socket );

    virtual void attachSocket( Socket& socket );
    virtual void detachSocket( Socket& socket );

private:
    bool   writePending_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_SOCKETWRITER_H */
