#ifndef __SMSC_MCISME_IASMEPROXY_HPP__
# define __SMSC_MCISME_IASMEPROXY_HPP__

# include <netinet/in.h>
# include "logger/Logger.h"
# include "core/threads/Thread.hpp"
# include "core/synchronization/Mutex.hpp"
# include "mcisme/misscall/callproc.hpp"
# include "eyeline/utilx/BoundedQueue.hpp"
# include "eyeline/protogen/framework/SerializerBuffer.hpp"

namespace smsc {
namespace mcisme {

class TaskProcessor;

class IASMEProxy : public core::threads::Thread {
public:
  IASMEProxy(const char* listening_iface, in_port_t listening_port,
             TaskProcessor* task_processor, size_t max_queue_sz);
  virtual int Execute();
  bool isConnected() const {
    core::synchronization::MutexGuard synchronize(_lock);
    return _socketToPeer != NULL;
  }

  bool sendRequest(const misscall::MissedCallEvent& event);

  void stop() { _isRunning = false; }

private:
  void processScheduledRequest();
  void prepareNextBufferForWrite();
  void processNextRequest();
  void acceptConnection();
  int readData(char* buf, int bytes_to_read);
  void processResponse();

  mutable core::synchronization::Mutex _lock;
  eyeline::utilx::BoundedQueue<misscall::MissedCallEvent> _eventsQueue;
  bool _isRunning;
  logger::Logger* _logger;
  TaskProcessor* _taskProcessor;
  core::network::Socket *_listeningSocket, *_socketToPeer;
  int _signallingRdSide, _signallingWrSide;
  bool _checkFdWriteable;
  eyeline::protogen::framework::SerializerBuffer _buf, *_currentBuf;
  size_t _totalWrittenBytes;
  uint32_t _bytesHasBeenRead, _messageBodyLen;
  union {
    char buf[sizeof(uint32_t)];
    uint32_t value;
  } _header;
  char _bufferForBody[1024];
};

}}

#endif
