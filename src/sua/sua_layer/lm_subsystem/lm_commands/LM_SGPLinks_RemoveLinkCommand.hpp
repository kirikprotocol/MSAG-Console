#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINKCOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINKCOMMAND_HPP__

# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_SGPLinks_RemoveLinkCommand : public LM_Command {
public:
  LM_SGPLinks_RemoveLinkCommand(const std::string& linkId);

  virtual std::string executeCommand();
private:
  std::string _linkId;
};

}

#endif
