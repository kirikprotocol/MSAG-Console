#ifndef _SCAG_PVSS_SERVER_PERSSERVERCONTEXT_H_
#define _SCAG_PVSS_SERVER_PERSSERVERCONTEXT_H_

#include "scag/pvss/api/core/server/ServerContext.h"
#include "ConnectionContext.h"

namespace scag2 {
namespace pvss {

class PersServerContext : public core::server::ServerContext {
public:

    PersServerContext(Request* req, ConnectionContext* connect) :
        ServerContext(req), conn_(connect) {};

  virtual smsc::core::network::Socket * getSocket() const {
      return conn_->getSocket();
  }

  virtual void sendResponse() /* throw(PvssException) */  {
      conn_->sendResponse(getResponse().get());
  }

private:
    ConnPtr conn_;
};

}
}

#endif

