#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINKCOMMAND_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINKCOMMAND_HPP__

# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

class LM_SGPLinks_RemoveLinkCommand : public LM_Command {
public:
  LM_SGPLinks_RemoveLinkCommand(const std::string& linkId);

  virtual std::string executeCommand();
private:
  std::string _linkId;
};

}}}}}

#endif
