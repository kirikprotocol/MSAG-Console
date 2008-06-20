#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSTATEMACHINECOUNTCOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSTATEMACHINECOUNTCOMMAND_HPP__

# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_StateMachineCountCommand : public LM_Command {
public:
  explicit LM_StateMachineCountCommand(const std::string& stateMachineCountValue);
  virtual std::string executeCommand();
private:
  unsigned int _stateMachineCountValue;
};

}

#endif
