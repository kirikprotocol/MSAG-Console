#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLEADDTRANSLATIONRULECOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLEADDTRANSLATIONRULECOMMAND_HPP__

# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationTable_AddTranslationRuleCommand : public LM_Command {
public:
  LM_TranslationTable_AddTranslationRuleCommand(const std::string& ruleName);

  virtual std::string executeCommand();
private:
  std::string _ruleName;
};

}}}}}

#endif
