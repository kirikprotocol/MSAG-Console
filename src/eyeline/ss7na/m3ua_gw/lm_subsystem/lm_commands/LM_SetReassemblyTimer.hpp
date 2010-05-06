#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSETREASSEMBLYTIMER_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSETREASSEMBLYTIMER_HPP__

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

class LM_SetReassemblyTimer : public common::lm_subsystem::LM_Command {
public:
  LM_SetReassemblyTimer(const std::string& timer_value,
                        common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr)
  {
    setId(composeCommandId());
    _timerValue = static_cast<unsigned>(utilx::strtol(timer_value.c_str(), (char **)NULL, 10));
    if ( _timerValue == 0 && errno )
      throw common::lm_subsystem::InvalidCommandLineException("LM_SetReassemblyTimer::LM_SetReassemblyTimer::: invalid timerValue format [%s]",
                                                              timer_value.c_str());
  }

  virtual std::string executeCommand() {
    _trnMgr.addOperation(this);
    return "OK";
  }

  virtual bool replaceable() const { return true; }

  virtual void updateConfiguration();

  static std::string composeCommandId();

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  unsigned _timerValue;
};

}}}}}

#endif
