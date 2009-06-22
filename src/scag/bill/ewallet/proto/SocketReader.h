#ifndef SCAG_BILL_EWALLET_PROTO_SOCKETREADER_H
#define SCAG_BILL_EWALLET_PROTO_SOCKETREADER_H

#include "IOSocketTask.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class SocketReader : public IOSocketTask
{
public:
    SocketReader(Core& core) : IOSocketTask(core,"ewall.read") {}

    virtual void shutdown() {
        IOSocketTask::shutdown();
        waitUntilReleased();
    }

protected:
    virtual bool setupSocket( Socket& socket, util::msectime_type currentTime );
    virtual bool hasEvents();
    virtual void process( Socket& socket );
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_SOCKETREADER_H */
