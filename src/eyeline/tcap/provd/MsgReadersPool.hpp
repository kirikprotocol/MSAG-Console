/* ************************************************************************** *
 * TC Provider: SCCP message readers thread-pool.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_MSGREADERSPOOL_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_MSGREADERSPOOL_HPP__

# include "logger/Logger.h"
# include "core/threads/ThreadPool.hpp"

# include "eyeline/ss7na/libsccp/SccpApi.hpp"
# include "eyeline/tcap/provd/TDlgIndProcessor.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class MsgReadersPool : protected smsc::core::threads::ThreadPool {
private:
  unsigned    _iniReaders;  //dflt = 2
  unsigned    _maxReaders;  //dflt = 0, no limit
  unsigned    _lastRdrId;   //

  TDlgIndProcessorIface *   _tIndProc;
  ss7na::libsccp::SccpApi * _sccpApi;
  smsc::logger::Logger *    _logger;

public:
  MsgReadersPool(TDlgIndProcessorIface & use_tind_proc)
    : _iniReaders(0), _maxReaders(0), _lastRdrId(0)
    , _tIndProc(&use_tind_proc), _sccpApi(0)
    , _logger(smsc::logger::Logger::getInstance("tcap.provd"))
  { }
  ~MsgReadersPool()
  { }

  //NOTE: 'max_num_readers' = 0 means no limit
  void init(ss7na::libsccp::SccpApi * sccp_api,
            unsigned ini_num_readers, unsigned max_num_readers = 0);
  void Start(void);
  void Stop(bool do_wait = false);

  bool addMsgReader(void);
};

}}}

#endif /* __EYELINE_TCAP_PROVD_MSGREADERSPOOL_HPP__ */

