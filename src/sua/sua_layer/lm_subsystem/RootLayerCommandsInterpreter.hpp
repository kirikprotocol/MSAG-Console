#ifndef __SUA_SUALAYER_LMSUBSYSTEM_ROOTLAYERCOMMANDSINTERPRETER_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_ROOTLAYERCOMMANDSINTERPRETER_HPP__

# include <sua/sua_layer/lm_subsystem/LM_CommandsInterpreter.hpp>

namespace lm_subsystem {

class RootLayerCommandsInterpreter : public LM_CommandsInterpreter {
public:
  virtual ~RootLayerCommandsInterpreter() {}
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
};

}

#endif
