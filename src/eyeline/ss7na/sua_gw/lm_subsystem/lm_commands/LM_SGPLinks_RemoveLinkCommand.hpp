#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINKCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINKCOMMAND_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_SGPLinks_RemoveLinkCommand : public common::lm_subsystem::LM_Command {
public:
  explicit LM_SGPLinks_RemoveLinkCommand(const std::string& link_id)
  : _linkId(link_id)
  {}

  virtual std::string executeCommand();
private:
  std::string _linkId;
};

}}}}}

#endif
