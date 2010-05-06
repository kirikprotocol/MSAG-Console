#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_TRANSLATIONRULECOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_TRANSLATIONRULECOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_TranslationRule_AddApplicationCommand.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_TranslationRule_AddLinkCommand.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_TranslationRule_AddTranslationEntryCommand.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class TranslationRuleCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  explicit TranslationRuleCommandsInterpreter(const std::string& rule_name)
  : _ruleName(rule_name)
  {}

  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer);
  virtual std::string getPromptString() const;

private:
  lm_commands::LM_TranslationRule_AddApplicationCommand* create_addApplicationCommand(utilx::StringTokenizer& string_tokenizer);
  lm_commands::LM_TranslationRule_AddLinkCommand* create_addLinkCommand(utilx::StringTokenizer& string_tokenizer);

  lm_commands::LM_TranslationRule_AddTranslationEntryCommand* create_addTranslationEntryCommand(utilx::StringTokenizer& string_tokenizer);

  std::string _ruleName;
  common::lm_subsystem::CmdsTransactionMgr _trnMgr;
};

}}}}

#endif
