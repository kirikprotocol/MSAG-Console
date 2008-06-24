#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSCOMMIT_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSCOMMIT_HPP__

# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_Applications_Commit : public LM_Command {
public:
  virtual std::string executeCommand();
};

}

#endif
