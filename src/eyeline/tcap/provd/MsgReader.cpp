#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/MsgReader.hpp"
#include "eyeline/tcap/provd/TDlgIndDispatcher.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using ss7na::libsccp::SccpApi;

MsgReader::MsgReader(unsigned reader_id, SccpApi * sccp_api,
                     TDlgIndProcessorIface * use_tind_proc)
  : _logger(smsc::logger::Logger::getInstance("tcap.provd"))
  , _sccpApi(sccp_api), _tIndProc(use_tind_proc)
{
  snprintf(_taskName, sizeof(_taskName), "MsgRdr[%u]", reader_id);
}

int MsgReader::Execute()
{
  while (!isStopping) {
    try {
      TDlgIndicationDispatcher tDsp;
      SccpApi::ErrorCode_e     errCode = _sccpApi->msgRecv(tDsp.getSCSPMessage());
      if (errCode == SccpApi::OK) {
        if (tDsp.processSCSPMessage())
          tDsp.dispatchTInd(_tIndProc);
        else
          smsc_log_error(_logger, "%s: processSCSPMessage() failed", _taskName);
      } else
        smsc_log_error(_logger, "%s: msgRecv() failed: %u", _taskName, errCode);
    } catch (const std::exception & ex) {
      smsc_log_error(_logger, "%s: caught exception '%s'", _taskName, ex.what());
    } catch (...) {
      smsc_log_error(_logger, "%s: caught unknown exception", _taskName);
    }
  }
  return 0;
}

}}}
