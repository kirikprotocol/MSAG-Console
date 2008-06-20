#include "LM_Command.hpp"

namespace lm_commands {

LM_Command::LM_Command()
  : _logger(smsc::logger::Logger::getInstance("lmsubsys"))
{}

}
