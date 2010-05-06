#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_ROOTLAYERCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_ROOTLAYERCOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class RootLayerCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  virtual ~RootLayerCommandsInterpreter() {}
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer);
  virtual std::string getPromptString() const;
};

}}}}

#endif
