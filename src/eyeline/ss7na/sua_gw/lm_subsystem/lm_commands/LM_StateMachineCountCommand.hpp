#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSTATEMACHINECOUNTCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSTATEMACHINECOUNTCOMMAND_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_StateMachineCountCommand : public common::lm_subsystem::LM_Command {
public:
  LM_StateMachineCountCommand(const std::string& state_machine_count,
                              common::lm_subsystem::CmdsTransactionMgr& trn_mgr);

  virtual std::string executeCommand();

private:
  unsigned int _stateMachineCountValue;
};

}}}}}

#endif
