#ifndef __SUA_SUALAYER_LMSUBSYSTEM_TRANSLATIONRULECOMMANDSINTERPRETER_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_TRANSLATIONRULECOMMANDSINTERPRETER_HPP__

# include <sua/sua_layer/lm_subsystem/LM_CommandsInterpreter.hpp>
# include <sua/sua_layer/lm_subsystem/lm_commands/LM_TranslationRule_AddApplicationCommand.hpp>
# include <sua/sua_layer/lm_subsystem/lm_commands/LM_TranslationRule_AddLinkCommand.hpp>
# include <sua/sua_layer/lm_subsystem/lm_commands/LM_TranslationRule_AddTranslationEntryCommand.hpp>

namespace lm_subsystem {

class TranslationRuleCommandsInterpreter : public LM_CommandsInterpreter {
public:
  explicit TranslationRuleCommandsInterpreter(const std::string& ruleName);

  virtual std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t> interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
private:
  lm_commands::LM_TranslationRule_AddApplicationCommand* create_addApplicationCommand(utilx::StringTokenizer& stringTokenizer);
  lm_commands::LM_TranslationRule_AddLinkCommand* create_addLinkCommand(utilx::StringTokenizer& stringTokenizer);

  lm_commands::LM_TranslationRule_AddTranslationEntryCommand* create_addTranslationEntryCommand(utilx::StringTokenizer& stringTokenizer);

  std::string _ruleName;
};

}

#endif
