#include "LM_Applications_ShowCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_Applications_ShowCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_Applications_ShowCommand::executeCommand:::");
  return "OK";
}

}}}}}
