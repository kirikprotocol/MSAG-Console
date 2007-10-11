#ifndef __SCAG_TRANSPORT_MMS_CONTEXT_H__
#define __SCAG_TRANSPORT_MMS_CONTEXT_H__

#include "core/synchronization/Mutex.hpp"
#include "core/network/Socket.hpp"
#include "core/buffers/TmpBuf.hpp"
#include "MmsCommand.h"
#include "MmsParser.hpp"

#define DFLT_BUF_SIZE 32

namespace scag { namespace transport { namespace mms {

using smsc::core::network::Socket;
using smsc::core::buffers::TmpBuf;
using smsc::core::synchronization::Mutex;

static const unsigned int NO_MULT_REM      = 1;
static const unsigned int DEL_SERVICE_SOCK = 2;
static const unsigned int DEL_CLIENT_SOCK  = 4;
static const unsigned int DEL_CONTEXT      = 8;
static const unsigned int STAT_RESP        = 16;
static const unsigned int FAKE_RESP        = 32;

enum ActionID {
  NOP = -1,
  PROCESS_REQUEST = 0,
  PROCESS_RESPONSE,
  PROCESS_STATUS_RESPONSE,
  PROCESS_LCM,
  READ_REQUEST,
  SEND_REQUEST,
  READ_RESPONSE,
  SEND_RESPONSE
};

/* no virtual methods are allowed in HttpContext */
class MmsContext {
public:
  /* field 'next' must be always the first field */
  MmsContext *next;
  MmsContext(Socket* client_socket, bool _is_vasp, unsigned int tid) : action(READ_REQUEST), is_vasp(_is_vasp),
        client(client_socket), service(NULL), position(0), command(NULL) {
    setTransactionId(tid);
    setContext(client, this);
  }
  ~MmsContext();
  const char *getTaskName() const {
    return taskName[action];
  }

  static void setConnected(Socket *s, bool c) {
    s->setData(CONNECT_FLAG, (void *)c);
  }
  static bool getConnected(Socket *s) {
    return (bool)s->getData(CONNECT_FLAG);
  }
  static void setContext(Socket *s, MmsContext* cx) {
    s->setData(CONTEXT, (void *)cx);
  }
  static MmsContext* getContext(Socket *s) {
    return (MmsContext *)s->getData(CONTEXT);
  }
  static void updateTimestamp(Socket *s, time_t t) {
    s->setData(TIMESTAMP, (void *)t);
  }
  static time_t getTimestamp(Socket *s) {
    return (time_t)s->getData(TIMESTAMP);
  }
  void setDestiny(int status, unsigned int flag) {
    result = status;
    flags = flag;
  }
  void setTransactionId(unsigned int tid);
  const string& getTransactionId() const {
    return transaction_id;
  }

  bool createResponse(Socket *s);
  bool createRequest();

  void createFakeResponse();
  void serializeResponse();
  void serializeRequest();

  MmsResponse &getResponse() {
    return *(MmsResponse *)command;
  }

  MmsRequest &getRequest() {
    return *(MmsRequest *)command;
  }
  void replaceTid();

  Socket *client;
  Socket *service;
  MmsCommand *command;

  HttpPacket http_packet;
  ActionID action;

  unsigned int flags;
  unsigned int position;
  int result;

private:
  enum DataKeys {
    CONTEXT,
    TIMESTAMP,
    CONNECT_FLAG
  };
  static ActionID actionNext[8];
  static const char *taskName[8];
  string transaction_id;
  TransactionContext trc;
  bool is_vasp;
};

}//mms
}//transport
}//scag

#endif
