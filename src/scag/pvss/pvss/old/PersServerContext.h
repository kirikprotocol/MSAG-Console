#ifndef _SCAG_PVSS_SERVER_PERSSERVERCONTEXT_H_
#define _SCAG_PVSS_SERVER_PERSSERVERCONTEXT_H_

#include "scag/pvss/api/core/server/ServerContext.h"
#include "Connection.h"

namespace scag2 {
namespace pvss {

class PersServerContext : public core::server::ServerContext {
public:

  PersServerContext(Request* req, Connection& connect):ServerContext(req), connection_(connect) {};

  virtual smsc::core::network::Socket * getSocket() const {
    return connection_.getSocket();
  }

  virtual void sendResponse() /* throw(PvssException) */  {
    connection_.sendResponse(getResponse().get());
  }

private:
  Connection& connection_;
};

}
}

#endif

