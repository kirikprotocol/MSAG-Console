#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDAPPLICATIONCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDAPPLICATIONCOMMAND_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_AddApplicationCommand : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationRule_AddApplicationCommand(const std::string& app_id,
                                           common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _appId(app_id)
  {}

  virtual std::string executeCommand();
  void setTranslationRuleName(const std::string& ruleName);

private:
  std::string _appId, _ruleName;
};

}}}}}

#endif
