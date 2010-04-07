#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINKCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINKCOMMAND_HPP__

# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
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
