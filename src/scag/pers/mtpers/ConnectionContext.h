#ifndef __SCAG_PERS_PVGW_PVGWCONTEXT_H__
#define __SCAG_PERS_PVGW_PVGWCONTEXT_H__

#include <string>
#include "logger/Logger.h"
#include "core/network/Socket.hpp"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pers/util/Types.h"
#include "PersCommand.h"
#include "Connection.h"


namespace scag { namespace mtpers {

using smsc::core::network::Socket;
using std::string;
using smsc::logger::Logger;
using scag::pers::util::PersServerResponseType;
using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

enum SocketState {
  READ_SOCKET,
  WRITE_SOCKET,
  READWRITE_SOCKET
};

enum Action {
  READ_REQUEST,
  PROCESS_REQUEST,
  SEND_RESPONSE
};

class ConnectionContext;

class SocketData {
public:
  static void setSocketState(Socket* s, uint8_t state) {
    s->setData(STATE, (void*)state);
  }
  static SocketState getSocketState(Socket* s) {
    return (SocketState)((uint64_t)s->getData(STATE));
  }
  static void setConnected(Socket* s, bool c) {
      s->setData(CONNECT_FLAG, (void*)c);
  }
  static bool getConnected(Socket* s) {
      return (bool)s->getData(CONNECT_FLAG);
  }
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

class IOTask;

struct ConnectionContext : public Connection {

public:
  ConnectionContext(Socket* sock);
  ~ConnectionContext();
  void clearBuffers();
  void createFakeResponse(PersServerResponseType response);
  bool parsePacket();
  void changeSocketState();
  void sendResponse();

public:
  Socket* socket;
  IOTask* iotask;
  SerialBuffer inbuf;
  SerialBuffer outbuf;
  Action action;
  PersPacket* packet;
  uint32_t packetLen;
  Logger* logger;

private:
  bool notSupport(PersCmd cmd);

};

}//mtpers
}//scag

#endif

