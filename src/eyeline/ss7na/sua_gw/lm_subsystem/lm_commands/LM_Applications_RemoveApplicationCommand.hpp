#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSREMOVEAPPLICATIONCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSREMOVEAPPLICATIONCOMMAND_HPP__

# include <netinet/in.h>
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_Applications_RemoveApplicationCommand : public common::lm_subsystem::LM_Command {
public:
  explicit LM_Applications_RemoveApplicationCommand(const std::string& app_id)
  : _appId(app_id)
  {}

  virtual std::string executeCommand();
private:
  std::string _appId;
};

}}}}}

#endif
