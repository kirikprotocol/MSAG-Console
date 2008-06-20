#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMCOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMCOMMAND_HPP__

# include <string>
# include <logger/Logger.h>

namespace lm_commands {

class LM_Command {
public:
  LM_Command();
  virtual ~LM_Command() {}
  virtual std::string executeCommand() = 0;
protected:
  smsc::logger::Logger* _logger;
};

}

#endif
