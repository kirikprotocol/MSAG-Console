#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLEADDTRANSLATIONRULE_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLEADDTRANSLATIONRULE_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
# include "eyeline/utilx/strtol.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationTable_AddTranslationRule : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationTable_AddTranslationRule(const std::string& rule_name,
                                         const std::string& gt,
                                         common::lm_subsystem::CmdsTransactionMgr& trn_mgr,
                                         bool to_mtp3 = true)
  : _trnMgr(trn_mgr), _ruleName(rule_name), _gt(rule_name), _ssn(0),
    _isSsnSet(false), _toMtp3(to_mtp3)
  {
    setId(composeCommandId(rule_name));
  }

  LM_TranslationTable_AddTranslationRule(const std::string& rule_name,
                                         const std::string& gt,
                                         const std::string& ssn,
                                         common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr), _ruleName(rule_name), _gt(rule_name), _toMtp3(false)
  {
    _ssn = static_cast<uint8_t>(utilx::strtol(ssn.c_str(), (char **)NULL, 10));
    if ( _ssn == 0 && errno )
      throw common::lm_subsystem::InvalidCommandLineException("LM_TranslationTable_AddTranslationRule::LM_TranslationTable_AddTranslationRule::: invalid ssn format [%s]",
                                                              ssn.c_str());
    _isSsnSet = true;
    setId(composeCommandId(rule_name));
  }

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& rule_name);

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _ruleName, _gt;
  uint8_t _ssn;
  bool _isSsnSet, _toMtp3;
};

}}}}}

#endif
