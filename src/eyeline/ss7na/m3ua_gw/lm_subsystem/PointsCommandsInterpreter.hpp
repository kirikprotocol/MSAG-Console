#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_POINTSCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_POINTSCOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

class PointsCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer);
  virtual std::string getPromptString() const { return "eyeline m3ua(config points)# "; }

private:
  common::lm_subsystem::LM_Command* parseAddPointCommand(utilx::StringTokenizer& string_tokenizer);
  common::lm_subsystem::CmdsTransactionMgr _trnMgr;
};

}}}}

#endif
