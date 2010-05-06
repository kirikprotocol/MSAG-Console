#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_INPUTCOMMANDPROCESSOR_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_INPUTCOMMANDPROCESSOR_HPP__

# include <deque>
# include "logger/Logger.h"
# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

class InputCommandProcessor {
public:
  InputCommandProcessor()
  : _logger(smsc::logger::Logger::getInstance("lm"))
  {}

  virtual ~InputCommandProcessor() {}

  std::string process(const std::string& input_command_line);
  std::string getUserPrompt();

protected:
  virtual LM_CommandsInterpreter* createRootCmdInterpreter() const = 0;

  smsc::logger::Logger* _logger;
  std::deque<lm_commands_interpreter_refptr_t> _commandInterpreters;
};

}}}}

#endif
