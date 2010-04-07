#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_TRANSLATIONRULECOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_TRANSLATIONRULECOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/sua_gw/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_TranslationRule_AddApplicationCommand.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_TranslationRule_AddLinkCommand.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_TranslationRule_AddTranslationEntryCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class TranslationRuleCommandsInterpreter : public LM_CommandsInterpreter {
public:
  explicit TranslationRuleCommandsInterpreter(const std::string& ruleName);

  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
private:
  lm_commands::LM_TranslationRule_AddApplicationCommand* create_addApplicationCommand(utilx::StringTokenizer& stringTokenizer);
  lm_commands::LM_TranslationRule_AddLinkCommand* create_addLinkCommand(utilx::StringTokenizer& stringTokenizer);

  lm_commands::LM_TranslationRule_AddTranslationEntryCommand* create_addTranslationEntryCommand(utilx::StringTokenizer& stringTokenizer);

  std::string _ruleName;
};

}}}}

#endif
