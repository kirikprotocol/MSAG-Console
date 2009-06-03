#ifndef __EYELINE_TCAP_PROVD_MSGREADERSPOOL_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_MSGREADERSPOOL_HPP__

# include "logger/Logger.h"
# include "core/threads/ThreadPool.hpp"
# include "eyeline/sua/libsua/SuaUser.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class MsgReadersPool : public smsc::core::threads::ThreadPool {
public:
  MsgReadersPool(unsigned maxNumOfMsgReaders,
                 sua::libsua::SuaApi* libSuaApi);

  void startup();
private:
  smsc::logger::Logger* _logger;
  unsigned _maxNumOfMsgReaders;
  sua::libsua::SuaApi* _libSuaApi;
};

}}}

#endif
