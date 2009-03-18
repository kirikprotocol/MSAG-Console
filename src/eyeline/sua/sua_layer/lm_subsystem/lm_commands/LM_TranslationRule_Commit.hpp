#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULECOMMIT_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULECOMMIT_HPP__

# include <string>
# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_Commit : public LM_Command {
public:
  virtual std::string executeCommand();
  void setTranslationRuleName(const std::string& ruleName);
private:
  std::string _ruleName;
};

}}}}}

#endif
