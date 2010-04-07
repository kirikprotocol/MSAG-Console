#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_IOEVENTPROCESSOR_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_IOEVENTPROCESSOR_HPP__

# include "logger/Logger.h"
# include "core/threads/ThreadedTask.hpp"
# include "eyeline/ss7na/common/io_dispatcher/ConnectMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class IOEventProcessor : public smsc::core::threads::ThreadedTask
{
public:
  IOEventProcessor(ConnectMgr& c_mgr)
  : _cMgr(c_mgr), _logger(smsc::logger::Logger::getInstance("io_dsptch")) {}
  virtual int Execute();
  virtual const char* taskName();
private:
  ConnectMgr& _cMgr;
  smsc::logger::Logger *_logger;
};

}}}}

#endif
