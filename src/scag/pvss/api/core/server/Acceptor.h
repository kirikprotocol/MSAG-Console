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
    virtual int setupSockets( util::msectime_type currentTime );
    virtual bool hasEvents() { return !finSocks_.empty(); }
    virtual void processEvents();

private:
    struct FinSock 
    {
        smsc::core::network::Socket* socket;
        util::msectime_type          connTime;
        FinSock() : socket(0) {}
        ~FinSock() { if (socket) {delete socket;} }

        bool operator == ( const smsc::core::network::Socket* val ) const {
            return ( socket == val );
        }
    };

    typedef std::vector< FinSock > FinSockArray;

private:

    smsc::core::network::Socket  socket_;   // server socket
    FinSockArray                 finSocks_; // owned
    ServerCore*                  serverCore_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_ACCEPTOR_H */
