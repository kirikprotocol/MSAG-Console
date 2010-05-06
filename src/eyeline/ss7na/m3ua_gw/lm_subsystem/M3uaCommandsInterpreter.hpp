#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_M3UACOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_M3UACOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

class M3uaCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer);
  virtual std::string getPromptString() const { return "eyeline m3ua(config)# "; }

private:
  common::lm_subsystem::LM_Command*
  create_LM_LocalInterface(utilx::StringTokenizer& string_tokenizer);

  common::lm_subsystem::CmdsTransactionMgr _trnMgr;
};

}}}}

#endif
