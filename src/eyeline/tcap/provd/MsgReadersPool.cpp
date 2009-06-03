#include "MsgReadersPool.hpp"
#include "MsgReader.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

MsgReadersPool::MsgReadersPool(unsigned maxNumOfMsgReaders,
                               sua::libsua::SuaApi* libSuaApi)
  : _logger(smsc::logger::Logger::getInstance("tcap.provd")),
    _maxNumOfMsgReaders(maxNumOfMsgReaders), _libSuaApi(libSuaApi)
{}

void
MsgReadersPool::startup()
{
  setMaxThreads(_maxNumOfMsgReaders);
  for(unsigned i=0; i<_maxNumOfMsgReaders; ++i) {
    startTask(new MsgReader(i, _libSuaApi));
  }
}

}}}
