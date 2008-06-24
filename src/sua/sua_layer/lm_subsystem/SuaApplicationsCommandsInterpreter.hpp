#ifndef __SUA_SUALAYER_LMSUBSYSTEM_SUAAPPLICATIONSCOMMANDSINTERPRETER_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_SUAAPPLICATIONSCOMMANDSINTERPRETER_HPP__

# include <sua/sua_layer/lm_subsystem/LM_CommandsInterpreter.hpp>

namespace lm_subsystem {

class SuaApplicationsCommandsInterpreter : public LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
};

}

#endif
