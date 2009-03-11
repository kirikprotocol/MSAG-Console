#ifndef _SCAG_PVSS_SERVER_IOTASK_H_
#define _SCAG_PVSS_SERVER_IOTASK_H_

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Array.hpp"
#include "core/network/Multiplexer.hpp"
#include "scag/util/multiplexer/RWMultiplexer.hpp"

#include "ConnectionContext.h"

namespace scag2 {
namespace pvss  { 

using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;
using smsc::core::buffers::Array;
using smsc::core::network::Multiplexer;
using scag2::util::multiplexer::RWMultiplexer;

class IOTaskManager;
class TasksSorter;

class SortedTask : public ThreadedTask {    
    friend class IOTaskManager;
    friend class TasksSorter;
public:
    SortedTask(uint32_t itemsCount = 0) : itemsCount_(itemsCount), index_(0) {}
    virtual ~SortedTask() {};
    virtual int Execute() { return 0; };
    virtual const char* taskName() { return ""; };
    void addItem() { ++itemsCount_; };

protected:
    uint32_t itemsCount_;
    uint16_t index_;
};

class IOTask: public SortedTask {

public:
  IOTask(IOTaskManager& iomanager, uint16_t connectionTimeout, const char* logName);
  virtual ~IOTask() {}
  virtual int Execute();
  virtual void stop();

  void registerContext(ConnectionContext* cx);
  uint32_t getSocketsCount() const;

protected:
  virtual void processSockets(Multiplexer::SockArray &ready, Multiplexer::SockArray &error, const time_t& now) = 0;
  virtual void addSocketToMultiplexer(Socket* s) = 0;
  virtual void disconnectSocket(Socket *s) = 0;
  virtual void addSocket(Socket* s);
  inline bool isTimedOut(Socket* s, time_t now);
  time_t checkConnectionTimeout(Multiplexer::SockArray& error);
  void removeSocket(Multiplexer::SockArray &error);
  void removeSocket(Socket *s);
  void removeSocketFromMultiplexer(Socket* s);
  bool idle() const;

protected:
  uint16_t connectionTimeout_;
  uint16_t checkTimeoutPeriod_;
  time_t lastCheckTime_;
  Performance performance_;
  RWMultiplexer multiplexer_;
  EventMonitor socketMonitor_;
  IOTaskManager& iomanager_;
  Logger* logger;
  Array<Socket*> waitingAdd_;
};


class MTPersReader: public IOTask {
public:
  MTPersReader(IOTaskManager& iomanager, uint16_t connectionTimeout):IOTask(iomanager, connectionTimeout, "reader") {};
  virtual const char * taskName();
  Performance getPerformance();
protected:
  void addSocketToMultiplexer(Socket* s);
  void processSockets(Multiplexer::SockArray &ready, Multiplexer::SockArray &error, const time_t& now);
  void disconnectSocket(Socket *s);
};

class MTPersWriter: public IOTask {
public:
  MTPersWriter(IOTaskManager& iomanager, uint16_t connectionTimeout):IOTask(iomanager, connectionTimeout, "writer") {};
  virtual const char * taskName();
protected:
  void addSocketToMultiplexer(Socket* s);
  void processSockets(Multiplexer::SockArray &ready, Multiplexer::SockArray &error, const time_t& now);
  void disconnectSocket(Socket *s);
  void addSocket(Socket* s);
};

}//pvss
}//scag2

#endif

