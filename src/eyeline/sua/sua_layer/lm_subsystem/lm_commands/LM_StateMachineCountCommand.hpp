#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSTATEMACHINECOUNTCOMMAND_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSTATEMACHINECOUNTCOMMAND_HPP__

# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

class LM_StateMachineCountCommand : public LM_Command {
public:
  explicit LM_StateMachineCountCommand(const std::string& stateMachineCountValue);
  virtual std::string executeCommand();
private:
  unsigned int _stateMachineCountValue;
};

}}}}}

#endif
