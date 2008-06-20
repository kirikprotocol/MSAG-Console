#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDTRANSLATIONENTRYCOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDTRANSLATIONENTRYCOMMAND_HPP__

# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_TranslationRule_AddTranslationEntryCommand : public LM_Command {
public:
  explicit LM_TranslationRule_AddTranslationEntryCommand(const std::string& entryName,
                                                         const std::string& gt,
                                                         unsigned int ssn);
  virtual std::string executeCommand();
  void setTranslationRuleName(const std::string& ruleName);
private:
  std::string _entryName, _ruleName, _gt;
  unsigned int _ssn;
};

}

#endif
