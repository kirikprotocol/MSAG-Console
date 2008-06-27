#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULECOMMIT_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULECOMMIT_HPP__

# include <string>
# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_TranslationRule_Commit : public LM_Command {
public:
  virtual std::string executeCommand();
  void setTranslationRuleName(const std::string& ruleName);
private:
  std::string _ruleName;
};

}

#endif
