#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSCOMMIT_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSCOMMIT_HPP__

# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_SGPLinks_Commit : public LM_Command {
public:
  virtual std::string executeCommand();
};

}

#endif
