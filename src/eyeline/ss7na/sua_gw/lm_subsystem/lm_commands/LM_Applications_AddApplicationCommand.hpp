#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSADDAPPLICATIONCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSADDAPPLICATIONCOMMAND_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_Applications_AddApplicationCommand : public common::lm_subsystem::LM_Command {
public:
  explicit LM_Applications_AddApplicationCommand(const std::string& app_id_value)
  : _appId(app_id_value)
  {}

  virtual std::string executeCommand();
private:
  std::string _appId;
};

}}}}}

#endif
