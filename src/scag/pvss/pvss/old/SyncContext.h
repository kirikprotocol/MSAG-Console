#ifndef _SCAG2_PVSS_SERVER_SYNCCONTEXT_H_
#define _SCAG2_PVSS_SERVER_SYNCCONTEXT_H_

#include "ConnectionContext.h"
#include "PersProtocol.h"

namespace scag2 {
namespace pvss  {

class Response;

class SyncContext : public ConnectionContext {

public:
  SyncContext(Socket* sock, WriterTaskManager& writerManager, core::server::ServerCore& server, PersProtocol& protocol, bool perfCounterOn = false);
  ~SyncContext() { smsc_log_info(logger_, "connection closed: %s", peerName_.c_str()); };
  bool processReadSocket(const time_t& now);
  void sendResponse(const Response* resp);

private:
  void createErrorResponse(uint8_t status, const char* msg = 0);
  bool sendResponse(const Response& resp);

private:
  SerialBuffer fakeResp_;
  PersProtocol& protocol_;
  uint32_t seqNum_;
};

}//pvss
}//scag2

#endif

