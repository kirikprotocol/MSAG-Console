#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLEREMOVETRANSLATIONRULE_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONTABLEREMOVETRANSLATIONRULE_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationTable_RemoveTranslationRule : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationTable_RemoveTranslationRule(const std::string& rule_name,
                                            common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr), _ruleName(rule_name)
  {
    setId(composeCommandId(rule_name));
  }

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& rule_name);

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _ruleName;
};

}}}}}

#endif
