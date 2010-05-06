#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_TRANSLATIONTABLECOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_TRANSLATIONTABLECOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

class TranslationTableCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer);
  virtual std::string getPromptString() const { return "eyeline m3ua(config translation-table)# "; }
private:
  common::lm_subsystem::LM_Command* parseAddTranslationRuleCommand(utilx::StringTokenizer& string_tokenizer,
                                                                   std::string* rule_name,
                                                                   bool* to_mtp3);
  common::lm_subsystem::CmdsTransactionMgr _trnMgr;
};

}}}}

#endif
