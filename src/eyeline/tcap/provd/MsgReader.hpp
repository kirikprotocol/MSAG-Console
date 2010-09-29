/* ************************************************************************** *
 * TC Provider: SCCP message reader.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_MSGREADER_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_MSGREADER_HPP__

# include "logger/Logger.h"
# include "core/threads/ThreadedTask.hpp"

# include "eyeline/ss7na/libsccp/SccpApi.hpp"
# include "eyeline/tcap/provd/TDlgIndProcessor.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class MsgReader : public smsc::core::threads::ThreadedTask {
private:
  smsc::logger::Logger *    _logger;
  ss7na::libsccp::SccpApi * _sccpApi;
  TDlgIndProcessorIface *   _tIndProc;
  char                      _taskName[sizeof("MsgRdr[%u]") + 4*sizeof(unsigned)]; 

public:
  MsgReader(unsigned reader_id, ss7na::libsccp::SccpApi* sccp_api,
            TDlgIndProcessorIface * use_tind_proc);
  ~MsgReader()
  { }

  virtual int Execute();
  virtual const char* taskName() { return _taskName; }
};

}}}

#endif /* __EYELINE_TCAP_PROVD_MSGREADER_HPP__ */

