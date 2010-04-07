#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMCOMMAND_HPP__

# include <string>
# include "logger/Logger.h"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_Command {
public:
  LM_Command();
  virtual ~LM_Command() {}
  virtual std::string executeCommand() = 0;
protected:
  smsc::logger::Logger* _logger;
};

}}}}}

#endif
