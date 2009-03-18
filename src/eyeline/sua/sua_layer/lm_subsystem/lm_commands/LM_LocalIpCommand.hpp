#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMLOCALIPCOMMAND_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMLOCALIPCOMMAND_HPP__

# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

class LM_LocalIpCommand : public LM_Command {
public:
  explicit LM_LocalIpCommand(const std::string& localHostValue);
  virtual std::string executeCommand();
private:
  std::string _localHost;
};

}}}}}

#endif
