#ifndef _SCAG_PVSS_CORE_SERVER_SERVERNEWCONTEXT_H
#define _SCAG_PVSS_CORE_SERVER_SERVERNEWCONTEXT_H

#include "ServerContext.h"
#include "scag/pvss/api/core/PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

class ServerNewContext : public ServerContext
{
public:
    ServerNewContext( Request* req, PvssSocket& channel ) :
    ServerContext(req), socket_(channel) {}

    virtual smsc::core::network::Socket* getSocket() const { return socket_.socket(); }

    virtual void sendResponse() /*throw (PvssException)*/ { socket_.send(getResponse().get(),false,true); }

private:
    PvssSocket& socket_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERNEWCONTEXT_H */
