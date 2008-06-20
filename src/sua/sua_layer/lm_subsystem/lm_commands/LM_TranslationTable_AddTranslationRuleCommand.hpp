#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLEADDTRANSLATIONRULECOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLEADDTRANSLATIONRULECOMMAND_HPP__

# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_TranslationTable_AddTranslationRuleCommand : public LM_Command {
public:
  LM_TranslationTable_AddTranslationRuleCommand(const std::string& ruleName);

  virtual std::string executeCommand();
private:
  std::string _ruleName;
};

}

#endif
