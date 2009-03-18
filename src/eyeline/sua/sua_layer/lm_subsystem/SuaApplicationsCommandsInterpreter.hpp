#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_SUAAPPLICATIONSCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_SUAAPPLICATIONSCOMMANDSINTERPRETER_HPP__

# include <eyeline/sua/sua_layer/lm_subsystem/LM_CommandsInterpreter.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {

class SuaApplicationsCommandsInterpreter : public LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
};

}}}}

#endif
