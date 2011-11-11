#ifndef _SCAG_PVSS_CORE_CLIENT_CONNECTOR_H
#define _SCAG_PVSS_CORE_CLIENT_CONNECTOR_H

#include "scag/util/Time.h"
#include "scag/pvss/api/core/SockTask.h"
#include "scag/pvss/api/core/client/ClientConfig.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {

class ClientCore;

class Connector : public SockTask
{
public:
    Connector( ClientConfig& theconfig, ClientCore& thecore );
    
    virtual void shutdown();

    /// register channel for connection
    void connectChannel( PvssSocket& socket, util::msectime_type startConnectTime );

    virtual const ClientConfig& getConfig() const { return * static_cast<const ClientConfig*>(config_); }

protected:
    virtual int setupSockets(util::msectime_type currentTime);

    virtual bool hasEvents() {
        return !pendingSockets_.empty() || !finishingSockets_.empty();
    }

    virtual void processEvents();

private:
    smsc::core::synchronization::Mutex finishingMutex_;
    ConnArray        pendingSockets_;
    ConnArray        finishingSockets_;
};

} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CLIENT_CONNECTOR_H */
