#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_LMCOMMAND_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_LMCOMMAND_HPP__

# include <string>
# include "logger/Logger.h"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

class LM_Command {
public:
  LM_Command()
  : _logger(smsc::logger::Logger::getInstance("lm"))
  {}
  virtual ~LM_Command() {}

  virtual std::string executeCommand() = 0;
  virtual void updateConfiguration() {}
  const std::string& getId() const { return _cmdId; }
  virtual bool replaceable() const { return false; }

protected:
  void setId(const std::string& cmd_id) { _cmdId = cmd_id; }
  smsc::logger::Logger* _logger;

private:
  std::string _cmdId;
};

}}}}

#endif
