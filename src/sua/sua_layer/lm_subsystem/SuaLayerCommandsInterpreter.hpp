#ifndef __SUA_SUALAYER_LMSUBSYSTEM_SUALAYERCOMMANDSINTERPRETER_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_SUALAYERCOMMANDSINTERPRETER_HPP__

# include <sua/sua_layer/lm_subsystem/LM_CommandsInterpreter.hpp>

namespace lm_subsystem {

class SuaLayerCommandsInterpreter : public LM_CommandsInterpreter {
public:
  virtual std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t> interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
};

}

#endif
