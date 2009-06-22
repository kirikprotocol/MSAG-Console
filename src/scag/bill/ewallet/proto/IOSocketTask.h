#ifndef SCAG_BILL_EWALLET_PROTO_IOSOCKETTASK_H
#define SCAG_BILL_EWALLET_PROTO_IOSOCKETTASK_H

#include "SocketTask.h"
#include "core/network/Multiplexer.hpp"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class IOSocketTask : public SocketTask
{
public:
    IOSocketTask( Core& core, const char* taskname ) : SocketTask(core,taskname) {}

protected:
    virtual bool setupSockets( util::msectime_type currentTime );

    virtual void processEvents();

    virtual bool setupSocket( Socket& socket, util::msectime_type currentTime ) = 0;
    virtual void process( Socket& socket ) = 0;
    virtual void postProcess();

protected:
    smsc::core::network::Multiplexer              mul_;
    smsc::core::network::Multiplexer::SockArray   ready_;
    smsc::core::network::Multiplexer::SockArray   error_;
    smsc::core::network::Multiplexer::SockArray   closed_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_IOSOCKETTASK_H */
