#ifndef _SCAG_PVSS_CORE_SERVER_ACCEPTOR_H
#define _SCAG_PVSS_CORE_SERVER_ACCEPTOR_H

#include "scag/pvss/api/core/SockTask.h"
#include "core/network/Socket.hpp"
#include "scag/pvss/common/PvssException.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ServerConfig;
class ServerCore;

/// interface
class Acceptor : public SockTask
{
public:
    Acceptor( ServerConfig& theconfig, ServerCore& thecore );

    virtual ~Acceptor();

    virtual const char* taskName() { return "pvss.accpt"; };

    virtual void shutdown();

    virtual void init() /* throw (PvssException) */ ;

protected:
    virtual bool setupSockets( util::msectime_type currentTime );
    virtual bool hasEvents() { return finishingSockets_.Count() > 0; }
    virtual void processEvents();

private:
    void removeFinishingSocket( smsc::core::network::Socket* socket );

private:
    struct FinSock {
        smsc::core::network::Socket* socket;
        util::msectime_type          connTime;
    };

    smsc::core::network::Socket           socket_;   // server socket
    smsc::core::buffers::Array< FinSock > finishingSockets_; // owned
    ServerCore*                           serverCore_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_ACCEPTOR_H */
