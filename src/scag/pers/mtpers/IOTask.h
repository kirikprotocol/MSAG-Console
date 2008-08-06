#ifndef __SCAG_PERS_PVGW_IOTASK_H__
#define __SCAG_PERS_PVGW_IOTASK_H__

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Array.hpp"
#include "scag/pers/RWMultiplexer.hpp"

//#include "PvgwManager.h"
#include "ConnectionContext.h"

namespace scag { namespace mtpers { 

const size_t READ_BUF_SIZE = 1024;

using smsc::core::threads::ThreadedTask;
using scag::pers::RWMultiplexer;
using smsc::core::synchronization::EventMonitor;
using smsc::logger::Logger;
using smsc::core::buffers::Array;
using smsc::core::network::Multiplexer;

using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;

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
  IOTask(IOTaskManager& iomanager, uint16_t connectionTimeout);
  virtual void stop();
  virtual int Execute();
  virtual const char * taskName();

  void registerContext(ConnectionContext* cx);
  uint32_t getSocketsCount() const;
  void addSocket(Socket* s);
  void changeSocketState(Socket *s);

private:
  void checkConnectionTimeout(Multiplexer::SockArray& error);
  inline bool isTimedOut(Socket* s, time_t now);
  void removeSocket(Multiplexer::SockArray &error);
  inline void removeSocket(Socket *s);
  void addSocketToMultiplexer(Socket* s);
  void removeSocketFromMultiplexer(Socket* s);
  bool processReadSocket(Socket* s);
  bool processWriteSocket(Socket* s);
  void disconnectSocket(Socket *s);
  bool idle();

private:
  RWMultiplexer multiplexer_;
  EventMonitor socketMonitor_;
  IOTaskManager& iomanager_;
  Logger* logger;
  Array<Socket*> waitingAdd_;
  uint16_t connectionTimeout_;
  char readBuf_[READ_BUF_SIZE];

};

}//mtpers
}//scag

#endif

