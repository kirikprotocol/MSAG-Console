#ifndef _SCAG_PVSS_CORE_IOTASK_H
#define _SCAG_PVSS_CORE_IOTASK_H

#include "SockTask.h"
#include "core/network/Multiplexer.hpp"

namespace scag2 {
namespace pvss {
namespace core {

class IOTask : public SockTask
{
protected:
    IOTask( Config& theconfig,
            Core& thecore,
            const char* tname ) :
    SockTask(theconfig,thecore,tname) {}

    virtual ~IOTask() {}

protected:
    /// setup things prior the invocation of hasEvents()
    virtual bool setupSockets(util::msectime_type currentTime);

    /// process events
    virtual void processEvents();

    virtual bool setupSocket( PvssSocket& sock ) = 0;
    virtual void process( PvssSocket& sock ) = 0;

protected:
    smsc::core::network::Multiplexer              mul_;
    smsc::core::network::Multiplexer::SockArray   ready_;
    smsc::core::network::Multiplexer::SockArray   error_;
};

} // namespace core
} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {
namespace core {

using scag2::pvss::core::IOTask;

}
}
}

#endif /* !_SCAG_PVSS_CORE_IOTASK_H */
