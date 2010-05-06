#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSETRECONNECTINTERVAL_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSETRECONNECTINTERVAL_HPP__

# include <sys/errno.h>

# include "eyeline/utilx/strtol.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_SetReconnectInterval : public common::lm_subsystem::LM_Command {
public:
  LM_SetReconnectInterval(const std::string& reconnect_interval,
                          common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr)
  {
    setId(composeCommandId());
    _reconnectInterval = static_cast<unsigned>(utilx::strtol(reconnect_interval.c_str(), (char **)NULL, 10));
    if ( _reconnectInterval == 0 && errno )
      throw common::lm_subsystem::InvalidCommandLineException("LM_SetReconnectInterval::LM_SetReconnectInterval::: invalid reconnect_interval value format [%s]",
                                                              reconnect_interval.c_str());
  }

  virtual std::string executeCommand() {
    _trnMgr.addOperation(this);
    return "OK";
  }

  virtual void updateConfiguration();

  virtual bool replaceable() const { return true; }

  static std::string composeCommandId();

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  unsigned _reconnectInterval;
};

}}}}}

#endif
