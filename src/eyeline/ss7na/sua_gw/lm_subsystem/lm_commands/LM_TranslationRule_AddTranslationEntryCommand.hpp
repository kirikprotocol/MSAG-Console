#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDTRANSLATIONENTRYCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDTRANSLATIONENTRYCOMMAND_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_AddTranslationEntryCommand : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationRule_AddTranslationEntryCommand(const std::string& entry_name,
                                                const std::string& gt,
                                                unsigned int ssn)
  : _entryName(entry_name), _gt(gt), _ssn(ssn)
  {}

  virtual std::string executeCommand();
  void setTranslationRuleName(const std::string& rule_name) {
    _ruleName = rule_name;
  }
private:
  std::string _entryName, _ruleName, _gt;
  unsigned int _ssn;
};

}}}}}

#endif
