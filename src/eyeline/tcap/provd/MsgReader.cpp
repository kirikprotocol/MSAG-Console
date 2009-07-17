#include <exception>
#include "MsgReader.hpp"
#include "TDlgIndDispatcher.hpp"
#include "eyeline/corex/io/IOExceptions.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

MsgReader::MsgReader(unsigned msgReaderNum, sua::libsua::SuaApi* libSuaApi)
  : _logger(smsc::logger::Logger::getInstance("tcap.provd")), _libSuaApi(libSuaApi)
{
  snprintf(_taskName, sizeof(_taskName), "MsgReader_%u", msgReaderNum);
}

int
MsgReader::Execute()
{
  while(!isStopping) {
    try {
      TDlgIndicationDispatcher tDsp;
      sua::libsua::SuaApi::ErrorCode_e errCode = _libSuaApi->msgRecv(tDsp.suaMsgBuf());
      if ( errCode == sua::libsua::SuaApi::OK ) {
        if (tDsp.processSuaMsgBuf()) {
          tDsp.dispatchTInd(&_tcapIndProcessor);
        } else
          smsc_log_error(_logger, "call to TDlgIndicationDispatcher::processSuaMsgBuf() failed");
      }
    } catch (corex::io::EOFException& ex) {
      // TODO: generate something indication
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "MsgReader::Execute:: caught exception [%s]", ex.what());
    }
  }
  return 0;
}

}}}
