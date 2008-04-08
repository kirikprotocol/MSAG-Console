#include <core/threads/ThreadedTask.hpp>
#include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
#include <logger/Logger.h>

namespace io_dispatcher {

class IOEventProcessor : public smsc::core::threads::ThreadedTask
{
public:
  IOEventProcessor(ConnectMgr& cMgr) : _cMgr(cMgr), _logger(smsc::logger::Logger::getInstance("io_dsptch")) {}
  virtual int Execute();
  virtual const char* taskName();
private:
  ConnectMgr& _cMgr;
  smsc::logger::Logger *_logger;
};

}
