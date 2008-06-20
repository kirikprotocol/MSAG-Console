#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDINTERPRETERS_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDINTERPRETERS_HPP__

# include <string>
# include <vector>
# include <utility>
# include <netinet/in.h>
# include <core/buffers/RefPtr.hpp>
# include <core/synchronization/Mutex.hpp>
# include <sua/utilx/StringTokenizer.hpp>
# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>
# include <sua/sua_layer/lm_subsystem/Exception.hpp>

namespace lm_subsystem {

class LM_CommandsInterpreter;
typedef smsc::core::buffers::RefPtr<LM_CommandsInterpreter,smsc::core::synchronization::Mutex> lm_commands_interpreter_refptr_t;

class lm_commands::LM_Command;
typedef smsc::core::buffers::RefPtr<lm_commands::LM_Command,smsc::core::synchronization::Mutex> lm_commands_refptr_t;

class LM_CommandsInterpreter {
public:
  virtual ~LM_CommandsInterpreter() {}
  virtual std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t> interpretCommandLine(utilx::StringTokenizer& stringTokenizer) = 0;
  virtual std::string getPromptString() const = 0;

protected:
  template <class INTERPRETER>
  INTERPRETER*
  create_CommandsInterpreter(utilx::StringTokenizer& stringTokenizer)
  {
    if ( stringTokenizer.hasNextToken() )
      throw InvalidCommandLineException("SuaLayerCommandsInterpreter::create_InputCommandParser::: wrong input string format");

    return new INTERPRETER();
  }

  template<class COMMAND>
  COMMAND*
  create_LM_Command(utilx::StringTokenizer& stringTokenizer)
  {
    std::string tokenValue;
    if ( stringTokenizer.hasNextToken() )
      tokenValue = stringTokenizer.nextToken();

    if ( stringTokenizer.hasNextToken() )
      throw InvalidCommandLineException("SuaLayerCommandsInterpreter::create_LM_Command::: wrong input string format");

    return new COMMAND(tokenValue);
  }
};

}

#endif
