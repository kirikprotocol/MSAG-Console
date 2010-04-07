#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_ROOTLAYERCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_ROOTLAYERCOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/sua_gw/lm_subsystem/LM_CommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class RootLayerCommandsInterpreter : public LM_CommandsInterpreter {
public:
  virtual ~RootLayerCommandsInterpreter() {}
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
};

}}}}

#endif
