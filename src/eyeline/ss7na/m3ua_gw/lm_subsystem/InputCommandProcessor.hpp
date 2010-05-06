#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_INPUTCOMMANDPROCESSOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_INPUTCOMMANDPROCESSOR_HPP__

# include <deque>
# include "logger/Logger.h"
# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

class InputCommandProcessor : public common::lm_subsystem::InputCommandProcessor {
public:
  InputCommandProcessor() {
    _commandInterpreters.push_back(common::lm_subsystem::lm_commands_interpreter_refptr_t(new RootLayerCommandsInterpreter()));
  }

protected:
  virtual common::lm_subsystem::LM_CommandsInterpreter* createRootCmdInterpreter() const {
    return new RootLayerCommandsInterpreter();
  }
};

}}}}

#endif
