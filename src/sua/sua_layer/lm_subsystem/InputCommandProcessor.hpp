#ifndef __SUA_SUALAYER_LMSUBSYSTEM_INPUTCOMMANDPROCESSOR_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_INPUTCOMMANDPROCESSOR_HPP__

# include <deque>
# include <logger/Logger.h>
# include <sua/utilx/Singleton.hpp>
# include <sua/sua_layer/lm_subsystem/LM_CommandsInterpreter.hpp>

namespace lm_subsystem {

class InputCommandProcessor : public utilx::Singleton<InputCommandProcessor> {
public:
  InputCommandProcessor();
  std::string process(const std::string& inputCommandLine);
  std::string getUserPrompt();
private:
  smsc::logger::Logger* _logger;

  std::deque<lm_commands_interpreter_refptr_t> _commandInterpreters;
};

}

#endif
