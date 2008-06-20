#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSADDAPPLICATIONCOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSADDAPPLICATIONCOMMAND_HPP__

# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_Applications_AddApplicationCommand : public LM_Command {
public:
  explicit LM_Applications_AddApplicationCommand(const std::string& appIdValue);
  virtual std::string executeCommand();
private:
  std::string _appId;
};

}

#endif
