#ifndef _SCAG_PVSS_SERVER_CONNECTIONCONTEXT_H_
#define _SCAG_PVSS_SERVER_CONNECTIONCONTEXT_H_

#include <string>
#include <vector>
#include "logger/Logger.h"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pvss/base/Types.h"
#include "scag/pvss/base/PersServerResponse.h"
#include "scag/pvss/api/core/server/ServerCore.h"

#include "Connection.h"
#include "PerformanceCounter.h"

namespace scag2 {
namespace pvss  {

const int32_t READ_BUF_SIZE = 10240;
const uint32_t RESP_BUF_SIZE = 5;
const uint32_t PACKET_LENGTH_SIZE = 4;

enum SocketState {
  READ_SOCKET,
  WRITE_SOCKET,
  READWRITE_SOCKET
};

enum Action {
  READ_REQUEST,
  PROCESS_REQUEST,
  SEND_RESPONSE,
  DEL_SOCKET
};

class ConnectionContext;

class SocketData 
{
public:
    static ConnectionContext* getContext(smsc::core::network::Socket* s) {
      return (ConnectionContext *)s->getData(CONTEXT);
  }
    static void setContext(smsc::core::network::Socket* s, ConnectionContext* cx) {
      s->setData(CONTEXT, (void *)cx);
  }
    static void updateTimestamp(smsc::core::network::Socket* s, time_t t) {
      s->setData(TIMESTAMP, (void *)t);
  }
    static time_t getTimestamp(smsc::core::network::Socket* s) {
      return (time_t)s->getData(TIMESTAMP);
  }

private:
  enum DataKeys {
    CONTEXT,
    TIMESTAMP,
    CONNECT_FLAG,
    STATE
  };
};

class WriterTaskManager;
//class ReaderTaskManager;
class RequestPacket;

struct ConnectionContext : public Connection
{
    friend class eyeline::informer::EmbedRefPtr< ConnectionContext >;

public:
    ConnectionContext( smsc::core::network::Socket* sock,
                       core::server::ServerCore& server,
                       bool perfCounterOn = false);
  virtual ~ConnectionContext();
    virtual bool processReadSocket(const time_t& now) = 0;
    bool processWriteSocket(const time_t& now);

  PerfCounter& getPerfCounter() { return perfCounter_; }

    /// unregister from registry, push all pending packets
    void unregisterFromCore();

protected:
  bool sendResponseData(const char* data, uint32_t dataSize);
  bool readData(const time_t& now);

private:
  void writeData(const char* data, uint32_t size);

protected:
  Action action_;
  uint32_t packetLen_;
  bool async_;
  core::server::ServerCore& pvssServer_;
    smsc::core::synchronization::Mutex mutex_;
    util::storage::SerialBuffer inbuf_;

private:
  PerfCounter perfCounter_;
  uint32_t packetsCount_;
    util::storage::SerialBuffer outbuf_;
  char readBuf_[READ_BUF_SIZE];
};

typedef eyeline::informer::EmbedRefPtr< ConnectionContext >  ConnPtr;

}//pvss
}//scag2

#endif
