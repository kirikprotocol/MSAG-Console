#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_TRANSLATIONRULECOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_TRANSLATIONRULECOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/m3ua_gw/lm_subsystem/lm_commands/LM_TranslationRule_AddDpc.hpp"
# include "eyeline/ss7na/m3ua_gw/lm_subsystem/lm_commands/LM_TranslationRule_AddSccpUser.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

class TranslationRuleCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  TranslationRuleCommandsInterpreter(const std::string& rule_name, bool to_mtp3)
  : _ruleName(rule_name), _toMtp3(to_mtp3), _addSccpUserCommandSelected(false),
    _addDpcCmd(NULL), _addSccpUserCmd(NULL)
  {}

  virtual ~TranslationRuleCommandsInterpreter() {
    delete _addDpcCmd;
    delete  _addSccpUserCmd;
  }
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer);
  virtual std::string getPromptString() const { return "eyeline m3ua(config translation-table translation-rule)# "; }

private:
  common::lm_subsystem::LM_Command* parseAddCommand(utilx::StringTokenizer& string_tokenizer);
  common::lm_subsystem::LM_Command* parseSetLpcCommand(utilx::StringTokenizer& string_tokenizer);
  common::lm_subsystem::LM_Command* parseTrafficModeCommand(utilx::StringTokenizer& string_tokenizer);

  common::lm_subsystem::CmdsTransactionMgr _trnMgr;
  std::string _ruleName;
  bool _toMtp3, _addSccpUserCommandSelected;
  lm_commands::LM_TranslationRule_AddDpc* _addDpcCmd;
  lm_commands::LM_TranslationRule_AddSccpUser* _addSccpUserCmd;
};

}}}}

#endif
