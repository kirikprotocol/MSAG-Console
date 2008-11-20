#ifndef __SCAG_MTPERS_IOTASK_H__
#define __SCAG_MTPERS_IOTASK_H__

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Array.hpp"
#include "core/network/Multiplexer.hpp"
#include "scag/pers/util/RWMultiplexer.hpp"

#include "ConnectionContext.h"

namespace scag { namespace mtpers { 

using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;
using smsc::core::buffers::Array;
using smsc::core::network::Multiplexer;
using scag::pers::util::RWMultiplexer;

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
  virtual time_t checkConnectionTimeout(Multiplexer::SockArray& error);
  virtual void addSocket(Socket* s);
  inline bool isTimedOut(Socket* s, time_t now);
  void removeSocket(Multiplexer::SockArray &error);
  void removeSocket(Socket *s);
  void removeSocketFromMultiplexer(Socket* s);
  virtual void disconnectSocket(Socket *s);
  bool idle() const;

protected:
  RWMultiplexer multiplexer_;
  EventMonitor socketMonitor_;
  IOTaskManager& iomanager_;
  Logger* logger;
  Array<Socket*> waitingAdd_;
  uint16_t connectionTimeout_;
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
protected:
  Performance performance_;
};

class MTPersWriter: public IOTask {
public:
  MTPersWriter(IOTaskManager& iomanager, uint16_t connectionTimeout):IOTask(iomanager, connectionTimeout, "writer") {};
  virtual const char * taskName();
protected:
  void addSocketToMultiplexer(Socket* s);
  void processSockets(Multiplexer::SockArray &ready, Multiplexer::SockArray &error, const time_t& now);
  time_t checkConnectionTimeout(Multiplexer::SockArray& error); 
  void addSocket(Socket* s);
};

}//mtpers
}//scag

#endif

