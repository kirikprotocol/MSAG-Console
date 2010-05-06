#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSTATEMACHINECOUNT_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSTATEMACHINECOUNT_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_StateMachineCount : public common::lm_subsystem::LM_Command {
public:
  LM_StateMachineCount(const std::string& state_machine_count,
                       common::lm_subsystem::CmdsTransactionMgr& trn_mgr);

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  virtual bool replaceable() const { return true; }

  static std::string composeCommandId() { return "stateMachineCount"; }

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  unsigned int _stateMachineCountValue;
};

}}}}}

#endif
