#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULETRAFFICMODECOMMAND_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULETRAFFICMODECOMMAND_HPP__

# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_TrafficModeCommand : public LM_Command {
public:
  LM_TranslationRule_TrafficModeCommand(const std::string& trafficMode);

  virtual std::string executeCommand();

  void setTranslationRuleName(const std::string& ruleName);
private:
  std::string _trafficMode, _ruleName;
};

}}}}}

#endif
