#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDLINKCOMMAND_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDLINKCOMMAND_HPP__

# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_AddLinkCommand : public LM_Command {
public:
  explicit LM_TranslationRule_AddLinkCommand(const std::string& linkIdValue);

  virtual std::string executeCommand();

  void setTranslationRuleName(const std::string& ruleName);
private:
  std::string _linkId, _ruleName;
};

}}}}}

#endif
