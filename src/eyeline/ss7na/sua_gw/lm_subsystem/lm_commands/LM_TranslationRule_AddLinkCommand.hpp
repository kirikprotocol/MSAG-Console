#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDLINKCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDLINKCOMMAND_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_AddLinkCommand : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationRule_AddLinkCommand(const std::string& link_id,
                                    common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _linkId(link_id)
  {}

  virtual std::string executeCommand();

  void setTranslationRuleName(const std::string& ruleName);
private:
  std::string _linkId, _ruleName;
};

}}}}}

#endif
