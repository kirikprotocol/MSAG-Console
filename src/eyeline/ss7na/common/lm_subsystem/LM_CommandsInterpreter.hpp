#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_LMCOMMANDINTERPRETERS_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_LMCOMMANDINTERPRETERS_HPP__

# include <string>
# include <vector>
# include <utility>
# include <netinet/in.h>

# include "eyeline/utilx/StringTokenizer.hpp"
# include "eyeline/ss7na/common/lm_subsystem/types.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

class CmdsTransactionMgr;

class LM_CommandsInterpreter {
public:
  virtual ~LM_CommandsInterpreter() {}

  struct interpretation_result {
    interpretation_result(LM_Command* a_command,
                          lm_commands_interpreter_refptr_t& an_interpreter,
                          bool pop_up_flag)
      : command(a_command), interpreter(an_interpreter), popUpCurrentInterpreter(pop_up_flag)
    {}

    LM_Command* command;
    lm_commands_interpreter_refptr_t interpreter;
    bool popUpCurrentInterpreter;
  };

  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer) = 0;
  virtual std::string getPromptString() const = 0;

protected:
  template <class INTERPRETER>
  INTERPRETER*
  create_CommandsInterpreter(utilx::StringTokenizer& string_tokenizer)
  {
    if ( string_tokenizer.hasNextToken() )
      throw InvalidCommandLineException("LM_CommandsInterpreter::create_InputCommandParser::: wrong input string format");

    return new INTERPRETER();
  }

  template<class COMMAND>
  COMMAND*
  create_LM_Command(utilx::StringTokenizer& string_tokenizer,
                    CmdsTransactionMgr& trn_mgr)
  {
    std::string tokenValue;
    if ( string_tokenizer.hasNextToken() )
      tokenValue = string_tokenizer.nextToken();
    else
      throw InvalidCommandLineException("LM_CommandsInterpreter::create_LM_Command::: missed mandatory keyword");

    if ( string_tokenizer.hasNextToken() )
      throw InvalidCommandLineException("LM_CommandsInterpreter::create_LM_Command::: wrong input string format");

    return new COMMAND(tokenValue, trn_mgr);
  }
};

}}}}

#endif
