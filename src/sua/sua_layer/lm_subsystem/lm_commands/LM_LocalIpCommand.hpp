#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMLOCALIPCOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMLOCALIPCOMMAND_HPP__

# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_LocalIpCommand : public LM_Command {
public:
  explicit LM_LocalIpCommand(const std::string& localHostValue);
  virtual std::string executeCommand();
private:
  std::string _localHost;
};

}

#endif
