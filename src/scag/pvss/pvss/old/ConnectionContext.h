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

using smsc::core::network::Socket;
using std::string;
using std::vector;
using smsc::logger::Logger;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;
using smsc::core::synchronization::Mutex;

const int32_t READ_BUF_SIZE = 1024;
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

class SocketData {
public:
  static ConnectionContext* getContext(Socket* s) {
      return (ConnectionContext *)s->getData(CONTEXT);
  }
  static void setContext(Socket* s, ConnectionContext* cx) {
      s->setData(CONTEXT, (void *)cx);
  }
  static void updateTimestamp(Socket* s, time_t t) {
      s->setData(TIMESTAMP, (void *)t);
  }
  static time_t getTimestamp(Socket* s) {
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
class ReaderTaskManager;
class RequestPacket;

struct ConnectionContext : public Connection {

public:
  ConnectionContext(Socket* sock, WriterTaskManager& writerManager, core::server::ServerCore& server, bool perfCounterOn = false);
  virtual ~ConnectionContext();
  virtual bool processReadSocket(const time_t& now) = 0;

  bool processWriteSocket(const time_t& now);
  bool canFinalize();
  bool canDelete();
  Socket* getSocket() const;
  PerfCounter& getPerfCounter() { return perfCounter_; }
  //void flushLogs();

protected:
  bool sendResponseData(const char* data, uint32_t dataSize);
  bool readData(const time_t& now);

private:
  void writeData(const char* data, uint32_t size);
  void getPeerIp();

protected:
  Action action_;
  uint32_t packetLen_;
  bool async_;
  core::server::ServerCore& pvssServer_;
  Mutex mutex_;
  string peerIp_;
  SerialBuffer inbuf_;
  Socket* socket_;
  Logger* logger_;
  Logger* debuglogger_;

private:
  WriterTaskManager& writerManager_;
  PerfCounter perfCounter_;
  uint8_t tasksCount_;
  uint32_t packetsCount_;
  //vector<DbLog> dbLogs_;
  SerialBuffer outbuf_;
  char readBuf_[READ_BUF_SIZE];
};

}//pvss
}//scag2

#endif

