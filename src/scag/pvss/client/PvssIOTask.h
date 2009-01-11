#ifndef _SCAG_PVSS_CLIENT_PVSSIOTASK_H
#define _SCAG_PVSS_CLIENT_PVSSIOTASK_H

#include "core/network/Multiplexer.hpp"
#include "PvssConnTask.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssIOTask : public PvssConnTask
{
protected:
    PvssIOTask( PvssStreamClient& pers, const char* logname ) :
    PvssConnTask(pers, smsc::logger::Logger::getInstance(logname)) {}
    
protected:
    virtual bool setupSockets();
    // virtual bool hasEvents() = 0;
    virtual void processEvents();

    virtual bool setupSocket( PvssConnection& con ) = 0;
    virtual void process( PvssConnection& con ) = 0;

protected:
    smsc::core::network::Multiplexer              mul_;
    smsc::core::network::Multiplexer::SockArray   ready_;
    smsc::core::network::Multiplexer::SockArray   error_;
};

} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSIOTHREAD_H */
