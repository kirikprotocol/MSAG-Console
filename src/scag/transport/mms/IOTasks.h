#ifndef SCAG_TRANSPORT_MMS_IO_TASKS
#define SCAG_TRANSPORT_MMS_IO_TASKS

#include "core/synchronization/EventMonitor.hpp"
#include "core/network/Socket.hpp"
#include "core/network/Multiplexer.hpp"
#include "logger/Logger.h"
#include "TaskList.h"

namespace scag { namespace transport { namespace mms {

using smsc::core::synchronization::EventMonitor;
using smsc::core::network::Socket;
using smsc::core::network::Multiplexer;
using smsc::logger::Logger;

class MmsContext;
class MmsManagerImpl;

class IOTask : public IOTaskParent {
public:
  IOTask(MmsManagerImpl &m, IOTaskManager &iom, int timeout);
  unsigned int getSocketCount() {
    return socket_count;
  }
  virtual void registerContext(MmsContext *cx) = 0;
  virtual void stop();
  virtual ~IOTask() {};

protected:
  class MmsMultiplexer : public Multiplexer {
  public:
    unsigned int Count() {
      return sockets.Count();
    }
    Socket* getSocket(unsigned int i) {
      return sockets[i];
    }
  };
  void checkConnectionTimeout(Multiplexer::SockArray& error);
  inline bool isTimedOut(Socket* s, time_t now);
  void killSocket(Socket *s);
  void removeSocket(Multiplexer::SockArray &error);
  inline void removeSocket(Socket *s);
  inline static void deleteSocket(Socket *s, char *buf, int how);
  inline void addSocket(Socket* s, bool connected);

  MmsMultiplexer multiplexer;
  EventMonitor sock_mon;
  MmsManagerImpl &manager;
  IOTaskManager &iomanager;
  Logger *logger;    
  int connection_timeout;
  Array<Socket*> waiting_add;
};

class MmsReaderTask : public IOTask {
public:
  MmsReaderTask(MmsManagerImpl& m, IOTaskManager& iom, int timeout);

  virtual int Execute();
  virtual const char* taskName();
  virtual void registerContext(MmsContext* cx);
private:
  bool createCommand(MmsContext *cx, Socket *s);

};

class MmsWriterTask : public IOTask {
public:
  MmsWriterTask(MmsManagerImpl& m, IOTaskManager& iom, int timeout);

  virtual int Execute();
  virtual const char* taskName();
  virtual void registerContext(MmsContext* cx);

protected:
  Array<Socket*> waiting_connect;
};

}//mms
}//transport
}//scag

#endif
