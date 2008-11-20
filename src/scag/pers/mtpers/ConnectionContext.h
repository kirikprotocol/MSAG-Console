#ifndef __SCAG_PERS_PVGW_PVGWCONTEXT_H__
#define __SCAG_PERS_PVGW_PVGWCONTEXT_H__

#include <string>
#include "logger/Logger.h"
#include "core/network/Socket.hpp"
#include "core/synchronization/Mutex.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pers/util/Types.h"
#include "PersCommand.h"
#include "Connection.h"
#include "PerformanceCounter.h"


namespace scag { namespace mtpers {

using smsc::core::network::Socket;
using std::string;
using smsc::logger::Logger;
using scag::pers::util::PersServerResponseType;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;
using smsc::core::synchronization::Mutex;

const uint32_t READ_BUF_SIZE = 1024;
const uint32_t RESP_BUF_SIZE = 5;

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

struct ConnectionContext : public Connection {

public:
  ConnectionContext(Socket* sock, WriterTaskManager& writerManager, ReaderTaskManager& readerManager, bool perfCounterOn = false);
  ~ConnectionContext();
  void sendResponse(const char* data, uint32_t dataSize);
  bool processReadSocket(const time_t& now);
  bool processWriteSocket();
  bool canFinalize();
  bool canDelete();
  Socket* getSocket();
  PerfCounter& getPerfCounter() { return perfCounter_; }

private:
  bool notSupport(PersCmd cmd);
  void writeData(const char* data, uint32_t size);
  void createFakeResponse(PersServerResponseType response);
  PersPacket* parsePacket();
  void sendFakeResponse();

private:
  SerialBuffer inbuf_;
  SerialBuffer outbuf_;
  SerialBuffer fakeResp_;
  uint32_t packetLen_;
  WriterTaskManager& writerManager_;
  ReaderTaskManager& readerManager_;
  Logger* logger_;
  Logger* debuglogger_;
  Action action_;
  Mutex mutex_;
  Socket* socket_;
  uint8_t tasksCount_;
  uint32_t packetsCount_;
  bool async_;
  uint32_t sequenceNumber_;
  PerfCounter perfCounter_;
  char readBuf_[READ_BUF_SIZE];
};

}//mtpers
}//scag

#endif

