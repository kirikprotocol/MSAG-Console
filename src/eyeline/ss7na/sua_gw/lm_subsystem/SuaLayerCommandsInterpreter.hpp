#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_SUALAYERCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_SUALAYERCOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class SuaLayerCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
private:
  common::lm_subsystem::CmdsTransactionMgr _trnMgr;
};

}}}}

#endif
