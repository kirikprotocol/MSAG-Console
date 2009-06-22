#ifndef SCAG_BILL_EWALLET_CLIENT_CONNECTOR_H
#define SCAG_BILL_EWALLET_CLIENT_CONNECTOR_H

#include "scag/bill/ewallet/proto/SocketTask.h"
#include "core/buffers/Array.hpp"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace client {

class ClientCore;

/// an implementation
class Connector : public proto::SocketTask
{
public:
    Connector( ClientCore& core );

    virtual ~Connector() {}

    virtual const char* taskName() { return "ewall.cntr"; }

    virtual void shutdown();
    void connectSocket( proto::Socket& socket );

protected:
    virtual bool setupSockets(util::msectime_type currentTime );
    virtual bool hasEvents() { return pendingSockets_.Count() > 0; }
    virtual void processEvents();

private:
    smsc::core::buffers::Array< proto::Socket* >   pendingSockets_;
};

} // namespace client
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_CLIENT_CONNECTOR_H */
