#ifndef __EYELINE_TCAP_PROVD_MSGREADER_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_MSGREADER_HPP__

# include "logger/Logger.h"
# include "core/threads/ThreadedTask.hpp"
# include "eyeline/sua/libsua/SuaApi.hpp"
# include "eyeline/tcap/provd/TCAPIndicationsProcessor.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class MsgReader : public smsc::core::threads::ThreadedTask {
public:
  MsgReader(unsigned msgReaderNum,
            sua::libsua::SuaApi* libSuaApi);

  virtual int Execute();
  virtual const char* taskName() { return _taskName; }
private:
  smsc::logger::Logger* _logger;
  sua::libsua::SuaApi* _libSuaApi;
  TCAPIndicationsProcessor _tcapIndProcessor;
  char _taskName[32];
};

}}}

#endif
