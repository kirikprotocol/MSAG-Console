#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSREMOVEAPPLICATIONCOMMAND_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSREMOVEAPPLICATIONCOMMAND_HPP__

# include <netinet/in.h>
# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

class LM_Applications_RemoveApplicationCommand : public LM_Command {
public:
  explicit LM_Applications_RemoveApplicationCommand(const std::string& appIdValue);
  virtual std::string executeCommand();
private:
  std::string _appId;
};

}}}}}

#endif
